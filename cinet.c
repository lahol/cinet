#include "cinet.h"
#include <string.h>
#include <json-glib/json-glib.h>
#include <json-glib/json-gobject.h>
#include <stdio.h>
#include <stdarg.h>

struct CINetMsgClass {
    guint32 msgtype;
    gsize size;
    JsonNode *(*msg_build)(CINetMsg *);
    CINetMsg *(*msg_read)(JsonNode *);
    void (*msg_free)(CINetMsg *);
    void (*msg_set_value)(CINetMsg *, const gchar *, const gpointer);
};

JsonNode *cinet_msg_version_build(CINetMsg *msg);
CINetMsg *cinet_msg_version_read(JsonNode *root);
void cinet_msg_version_set_value(CINetMsg *msg, const gchar *key, const gpointer value);
void cinet_msg_version_free(CINetMsg *msg);

static struct CINetMsgClass msgclasses[] = {
    { CI_NET_MSG_VERSION, sizeof(CINetMsgVersion), cinet_msg_version_build,
        cinet_msg_version_read, cinet_msg_version_free, cinet_msg_version_set_value},
    { CI_NET_MSG_EVENT_RING, sizeof(CINetMsg), NULL, NULL, NULL, NULL },
    { CI_NET_MSG_EVENT_CALL, sizeof(CINetMsg), NULL, NULL, NULL, NULL },
    { CI_NET_MSG_EVENT_CONNECT, sizeof(CINetMsg), NULL, NULL, NULL, NULL },
    { CI_NET_MSG_EVENT_DISCONNECT, sizeof(CINetMsg), NULL, NULL, NULL, NULL }
};

static struct CINetMsgClass *cinet_msg_get_class(CINetMsg *msg)
{
    if (!msg || msg->msgtype >= CI_NET_MSG_COUNT)
        return NULL;
    return &msgclasses[msg->msgtype];
}

static struct CINetMsgClass *cinet_msg_type_get_class(guint32 msgtype)
{
    if (msgtype >= CI_NET_MSG_COUNT)
        return NULL;
    return &msgclasses[msgtype];
}

JsonNode *cinet_msg_build(CINetMsg *msg)
{
    struct CINetMsgClass *cls = cinet_msg_get_class(msg);
    if (!cls || !cls->msg_build)
        return NULL;
    return cls->msg_build(msg);
}

CINetMsg *cinet_msg_read(guint32 msgtype, JsonNode *root)
{
    struct CINetMsgClass *cls = cinet_msg_type_get_class(msgtype);
    if (!cls || !cls->msg_read)
        return NULL;
    return cls->msg_read(root);
}

gpointer cinet_msg_alloc(guint32 msgtype)
{
    struct CINetMsgClass *cls = cinet_msg_type_get_class(msgtype);
    if (!cls || cls->size == 0)
        return NULL;
    CINetMsg *msg = g_malloc0(cls->size);
    msg->msgtype = msgtype;
}

void cinet_msg_free(CINetMsg *msg)
{
    struct CINetMsgClass *cls = cinet_msg_get_class(msg);
    if (cls && cls->msg_free)
        cls->msg_free(msg);
    if (msg)
        g_free(msg);
}

static inline void cinet_set_ulong(gpointer dst, gint off, guint32 val)
{
    ((guchar*)dst)[off  ] = val & 0xff;
    ((guchar*)dst)[off+1] = (val >>  8) & 0xff;
    ((guchar*)dst)[off+2] = (val >> 16) & 0xff;
    ((guchar*)dst)[off+3] = (val >> 24) & 0xff;
}

#define cinet_get_ulong(buf, off) ((((guchar*)(buf))[(off)] & 0xff) |\
                                   ((((guchar*)(buf))[(off)+1] & 0xff) << 8) |\
                                   ((((guchar*)(buf))[(off)+2] & 0xff) << 16) |\
                                   ((((guchar*)(buf))[(off)+3] & 0xff) << 24))

#define CINET_HEADER_SET_LEN(h, len)   cinet_set_ulong((h), 6, (len))
#define CINET_HEADER_SET_TYPE(h, type) cinet_set_ulong((h), 10, (type))

#define CINET_HEADER_GET_LEN(h)        cinet_get_ulong((h), 6)
#define CINET_HEADER_GET_TYPE(h)       cinet_get_ulong((h), 10)

#define CINET_MAGIC_STRING "ci-msg"
#define CINET_CHECK_MAGIC_STRING(data) (!strncmp((gchar*)(data), CINET_MAGIC_STRING, 6))

gsize cinet_msg_write_header(gchar *data, gsize len, CINetMsgHeader *header)
{
    guint32 value;
    if (!data || len < CINET_HEADER_LENGTH || !header)
        return -1;
    /* magic string */
    strcpy(data, "ci-msg");
    /* msg length */
    CINET_HEADER_SET_LEN(data, header->msglen);
    CINET_HEADER_SET_TYPE(data, header->msgtype);

    return CINET_HEADER_LENGTH;
}

gsize cinet_msg_read_header(CINetMsgHeader *header, gchar *data, gsize len)
{
    if (len < CINET_HEADER_LENGTH || !header || !data)
        return -1;

    if (!CINET_CHECK_MAGIC_STRING(data))
        return -1;
    header->msglen = CINET_HEADER_GET_LEN(data);
    header->msgtype = CINET_HEADER_GET_TYPE(data);

    return CINET_HEADER_LENGTH;
}

gint cinet_msg_write_msg(gchar **buffer, gsize *len, CINetMsg *msg)
{
    if (!msg || !buffer || !len)
       return -1;
    JsonGenerator *gen = NULL;
    JsonNode *node = NULL;
    CINetMsgHeader header;

    gchar *payload;
    gsize sz, off;

    node = cinet_msg_build(msg);
    if (!node)
        return -1;

    gen = json_generator_new();
    json_generator_set_root(gen, node);
    payload = json_generator_to_data(gen, &sz);
    json_node_free(node);
    g_object_unref(gen);

    *len = sz+CINET_HEADER_LENGTH;
    *buffer = g_malloc(*len);
    header.msgtype = msg->msgtype;
    header.msglen  = sz;

    off = cinet_msg_write_header(*buffer, *len, &header);
    if (off < CINET_HEADER_LENGTH) {
        g_free(payload);
        g_free(*buffer);
        *buffer = NULL;
        *len = 0;
        return -1;
    }

    memcpy(&(*buffer)[off], payload, sz);
    g_free(payload);

    return 0;
}

gint cinet_msg_read_msg(CINetMsg **msg, gchar *buffer, gsize len)
{
    if (!msg || !buffer)
        return -1;

    CINetMsgHeader header;
    gsize off;

    JsonParser *parser;
    JsonNode *root;

    if ((off = cinet_msg_read_header(&header, buffer, len)) < CINET_HEADER_LENGTH)
        return -1;

    parser = json_parser_new();
    if (!json_parser_load_from_data(parser, &buffer[off], len-off, NULL)) {
        g_object_unref(parser);
        return -1;
    }

    root = json_parser_get_root(parser);

    *msg = cinet_msg_read(header.msgtype, root);

    g_object_unref(parser);

    if (*msg)
        return 0;
    return -1;
}

CINetMsg *cinet_message_new(guint32 msgtype, ...)
{
    CINetMsg *msg = cinet_msg_alloc(msgtype);
    struct CINetMsgClass *cls = cinet_msg_type_get_class(msgtype);
    va_list ap;
    gchar *key;
    gpointer val;

    if (!msg)
        return NULL;

    if (cls && cls->msg_set_value) {
        va_start(ap, msgtype);
        do {
            key = va_arg(ap, gchar*);
            val = va_arg(ap, gpointer);
            if (key) {
                cls->msg_set_value(msg, key, val);
            }
        } while (key);
        va_end(ap);
    }

    return msg;
}

void cinet_message_set_value(CINetMsg *msg, const gchar *key, const gpointer value)
{
    struct CINetMsgClass *cls = cinet_msg_get_class(msg);
    if (!cls || !cls->msg_set_value || !key)
        return;
    cls->msg_set_value(msg, key, value);
}

JsonNode *cinet_msg_version_build(CINetMsg *msg)
{
    CINetMsgVersion *cmsg = (CINetMsgVersion*)msg;
    JsonBuilder *builder = json_builder_new();
    JsonNode *root;

    json_builder_begin_object(builder);

    json_builder_set_member_name(builder, "major");
    json_builder_add_int_value(builder, cmsg->major);

    json_builder_set_member_name(builder, "minor");
    json_builder_add_int_value(builder, cmsg->minor);

    json_builder_set_member_name(builder, "patch");
    json_builder_add_int_value(builder, cmsg->patch);

    json_builder_set_member_name(builder, "human_readable");
    json_builder_add_string_value(builder, cmsg->human_readable != NULL ?
                                  cmsg->human_readable : "");
    json_builder_end_object(builder);

    root = json_builder_get_root(builder);

    g_object_unref(builder);

    return root;
}

CINetMsg *cinet_msg_version_read(JsonNode *root)
{
    CINetMsgVersion *msg = cinet_msg_alloc(CI_NET_MSG_VERSION);
    if (!JSON_NODE_HOLDS_OBJECT(root)) {
        cinet_msg_free((CINetMsg*)msg);
        return NULL;
    }

    JsonObject *obj;
    JsonNode *node;

    obj = json_node_get_object(root);

    msg->major = (guint32)json_object_get_int_member(obj, "major");
    msg->minor = (guint32)json_object_get_int_member(obj, "minor");
    msg->patch = (guint32)json_object_get_int_member(obj, "patch");
    msg->human_readable = g_strdup(json_object_get_string_member(obj, "human_readable"));

    return (CINetMsg*)msg;
}

void cinet_msg_version_set_value(CINetMsg *msg, const gchar *key, const gpointer value)
{
    if (!msg || !key)
        return;
    CINetMsgVersion *cmsg = (CINetMsgVersion*)msg; 
    if (!strcmp(key, "major"))
        cmsg->major = GPOINTER_TO_INT(value);
    if (!strcmp(key, "minor"))
        cmsg->minor = GPOINTER_TO_INT(value);
    if (!strcmp(key, "patch"))
        cmsg->patch = GPOINTER_TO_INT(value);
    if (!strcmp(key, "human_readable")) {
        g_free(cmsg->human_readable);
        cmsg->human_readable = g_strdup((const gchar*)value);
    }
}

void cinet_msg_version_free(CINetMsg *msg)
{
    CINetMsgVersion *cmsg = (CINetMsgVersion*)msg;
    g_free(cmsg->human_readable);
}
