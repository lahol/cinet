#include "cinet.h"
#include <string.h>
#include <json-glib/json-glib.h>
#include <json-glib/json-gobject.h>
#include <stdio.h>
#include <stdarg.h>

struct CINetMsgClass {
    CINetMsgType msgtype;
    gsize size;
    JsonNode *(*msg_build)(CINetMsg *);
    CINetMsg *(*msg_read)(JsonNode *);
    void (*msg_free)(CINetMsg *);
    void (*msg_set_value)(CINetMsg *, const gchar *, const gpointer);
};

JsonNode *cinet_msg_default_build(CINetMsg *msg);

JsonNode *cinet_msg_version_build(CINetMsg *msg);
CINetMsg *cinet_msg_version_read(JsonNode *root);
void cinet_msg_version_set_value(CINetMsg *msg, const gchar *key, const gpointer value);
void cinet_msg_version_free(CINetMsg *msg);

JsonNode *cinet_msg_event_ring_build(CINetMsg *msg);
CINetMsg *cinet_msg_event_ring_read(JsonNode *root);
void cinet_msg_event_ring_set_value(CINetMsg *msg, const gchar *key, const gpointer value);
void cinet_msg_event_ring_free(CINetMsg *msg);

static struct CINetMsgClass msgclasses[] = {
    { CI_NET_MSG_VERSION, sizeof(CINetMsgVersion), cinet_msg_version_build,
        cinet_msg_version_read, cinet_msg_version_free, cinet_msg_version_set_value},
    { CI_NET_MSG_EVENT_RING, sizeof(CINetMsgEventRing), cinet_msg_event_ring_build,
        cinet_msg_event_ring_read, cinet_msg_event_ring_free, cinet_msg_event_ring_set_value },
    { CI_NET_MSG_EVENT_CALL, sizeof(CINetMsg), NULL, NULL, NULL, NULL },
    { CI_NET_MSG_EVENT_CONNECT, sizeof(CINetMsg), NULL, NULL, NULL, NULL },
    { CI_NET_MSG_EVENT_DISCONNECT, sizeof(CINetMsg), NULL, NULL, NULL, NULL },
    { CI_NET_MSG_LEAVE, sizeof(CINetMsgLeave), NULL, NULL, NULL, NULL },
    { CI_NET_MSG_SHUTDOWN, sizeof(CINetMsgShutdown), NULL, NULL, NULL, NULL }
};

static struct CINetMsgClass *cinet_msg_get_class(CINetMsg *msg)
{
    if (!msg || msg->msgtype >= CI_NET_MSG_COUNT)
        return NULL;
    return &msgclasses[msg->msgtype];
}

static struct CINetMsgClass *cinet_msg_type_get_class(CINetMsgType msgtype)
{
    if (msgtype >= CI_NET_MSG_COUNT)
        return NULL;
    return &msgclasses[msgtype];
}

JsonNode *cinet_msg_build(CINetMsg *msg)
{
    struct CINetMsgClass *cls = cinet_msg_get_class(msg);
    if (!cls)
        return NULL;
    if (!cls->msg_build)
        return cinet_msg_default_build(msg);
    return cls->msg_build(msg);
}

CINetMsg *cinet_msg_read(CINetMsgType msgtype, JsonNode *root)
{
    struct CINetMsgClass *cls = cinet_msg_type_get_class(msgtype);
    if (!cls)
       return NULL;
    if (!cls->msg_read)
        return (CINetMsg*)cinet_msg_alloc(msgtype);
    return cls->msg_read(root);
}

gpointer cinet_msg_alloc(CINetMsgType msgtype)
{
    struct CINetMsgClass *cls = cinet_msg_type_get_class(msgtype);
    if (!cls || cls->size == 0)
        return NULL;
    CINetMsg *msg = g_malloc0(cls->size);
    msg->msgtype = msgtype;

    return msg;
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

gssize cinet_msg_write_header(gchar *data, gsize len, CINetMsgHeader *header)
{
    if (!data || len < CINET_HEADER_LENGTH || !header)
        return -1;
    /* magic string */
    strcpy(data, "ci-msg");
    /* msg length */
    CINET_HEADER_SET_LEN(data, header->msglen);
    CINET_HEADER_SET_TYPE(data, header->msgtype);

    return CINET_HEADER_LENGTH;
}

gssize cinet_msg_read_header(CINetMsgHeader *header, gchar *data, gsize len)
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

    if (JSON_NODE_HOLDS_OBJECT(node) && json_node_get_object(node) == NULL) {
        payload = g_strdup("{}");
        sz = 2;
        json_node_free(node);
    }
    else {
        gen = json_generator_new();
        json_generator_set_root(gen, node);
        payload = json_generator_to_data(gen, &sz);
        json_node_free(node);
        g_object_unref(gen);
    }

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

    if (sz > 0)
        memcpy(&(*buffer)[off], payload, sz);
    g_free(payload);

    return 0;
}

gint cinet_msg_read_msg(CINetMsg **msg, gchar *buffer, gsize len)
{
    if (!msg || !buffer)
        return -1;

    CINetMsgHeader header;
    gssize off;

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

CINetMsg *cinet_message_new_va(CINetMsgType msgtype, va_list args)
{
    CINetMsg *msg = cinet_msg_alloc(msgtype);
    struct CINetMsgClass *cls = cinet_msg_type_get_class(msgtype);
    gchar *key;
    gpointer val;

    if (!msg)
        return NULL;

    if (cls && cls->msg_set_value) {
        do {
            key = va_arg(args, gchar*);
            val = va_arg(args, gpointer);
            if (key) {
                cls->msg_set_value(msg, key, val);
            }
        } while (key);
    }

    return msg;
}

CINetMsg *cinet_message_new(CINetMsgType msgtype, ...)
{
    va_list ap;
    va_start(ap, msgtype);
    CINetMsg *msg = cinet_message_new_va(msgtype, ap);
    va_end(ap);

    return msg;
}

gint cinet_message_new_for_data(gchar **buffer, gsize *len, guint msgtype, ...)
{
    va_list ap;
    gint rc = -1;
    va_start(ap, msgtype);
    CINetMsg *msg = cinet_message_new_va(msgtype, ap);
    va_end(ap);

    if (msg) {
        rc = cinet_msg_write_msg(buffer, len, msg);
        cinet_msg_free(msg);
    }

    return rc;
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
    if (!JSON_NODE_HOLDS_OBJECT(root))
        return NULL;

    CINetMsgVersion *msg = cinet_msg_alloc(CI_NET_MSG_VERSION);

    JsonObject *obj = json_node_get_object(root);

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

JsonNode *cinet_msg_event_ring_build(CINetMsg *msg)
{
    CINetMsgEventRing *cmsg = (CINetMsgEventRing*)msg;
    JsonBuilder *builder = json_builder_new();
    JsonNode *root;

    json_builder_begin_object(builder);

    json_builder_set_member_name(builder, "stage");
    json_builder_add_int_value(builder, ((CINetMsgMultipart*)msg)->stage);

    json_builder_set_member_name(builder, "part");
    json_builder_add_int_value(builder, ((CINetMsgMultipart*)msg)->part);

    json_builder_set_member_name(builder, "msgid");
    json_builder_add_string_value(builder, ((CINetMsgMultipart*)msg)->msgid);

    if (cmsg->completenumber) {
        json_builder_set_member_name(builder, "completenumber");
        json_builder_add_string_value(builder, cmsg->completenumber);
    }
    if (cmsg->areacode) {
        json_builder_set_member_name(builder, "areacode");
        json_builder_add_string_value(builder, cmsg->areacode);
    }
    if (cmsg->number) {
        json_builder_set_member_name(builder, "number");
        json_builder_add_string_value(builder, cmsg->number);
    }
    if (cmsg->date) {
        json_builder_set_member_name(builder, "date");
        json_builder_add_string_value(builder, cmsg->date);
    }
    if (cmsg->time) {
        json_builder_set_member_name(builder, "time");
        json_builder_add_string_value(builder, cmsg->time);
    }
    if (cmsg->msn) {
        json_builder_set_member_name(builder, "msn");
        json_builder_add_string_value(builder, cmsg->msn);
    }
    if (cmsg->alias) {
        json_builder_set_member_name(builder, "alias");
        json_builder_add_string_value(builder, cmsg->alias);
    }
    if (cmsg->area) {
        json_builder_set_member_name(builder, "area");
        json_builder_add_string_value(builder, cmsg->area);
    }
    if (cmsg->name) {
        json_builder_set_member_name(builder, "name");
        json_builder_add_string_value(builder, cmsg->name);
    }

    json_builder_end_object(builder);

    root = json_builder_get_root(builder);

    g_object_unref(builder);

    return root;
}

CINetMsg *cinet_msg_event_ring_read(JsonNode *root)
{
    if (!JSON_NODE_HOLDS_OBJECT(root))
        return NULL;

    CINetMsgEventRing *msg = cinet_msg_alloc(CI_NET_MSG_EVENT_RING);

    JsonObject *obj = json_node_get_object(root);

    cinet_msg_event_ring_set_value((CINetMsg*)msg, "stage", GINT_TO_POINTER(json_object_get_int_member(obj, "stage")));
    cinet_msg_event_ring_set_value((CINetMsg*)msg, "part", GINT_TO_POINTER(json_object_get_int_member(obj, "part")));

#define MSG_STR_SET(arg) do {\
    if (json_object_has_member(obj, arg)) \
        cinet_msg_event_ring_set_value((CINetMsg*)msg, arg, (const gpointer)json_object_get_string_member(obj, arg));\
    } while (0)

    MSG_STR_SET("msgid");
    MSG_STR_SET("completenumber");
    MSG_STR_SET("areacode");
    MSG_STR_SET("number");
    MSG_STR_SET("date");
    MSG_STR_SET("time");
    MSG_STR_SET("msn");
    MSG_STR_SET("alias");
    MSG_STR_SET("area");
    MSG_STR_SET("name");

#undef MSG_STR_SET

    return (CINetMsg*)msg;
}

void cinet_msg_event_ring_set_value(CINetMsg *msg, const gchar *key, const gpointer value)
{
    if (!msg || !key)
        return;
    if (!strcmp(key, "msgid")) {
        strncpy(((CINetMsgMultipart*)msg)->msgid, value, 16);
        return;
    }
    if (!strcmp(key, "stage")) {
        ((CINetMsgMultipart*)msg)->stage = GPOINTER_TO_INT(value);
        return;
    }
    if (!strcmp(key, "part")) {
        ((CINetMsgMultipart*)msg)->part = GPOINTER_TO_INT(value);
        return;
    }
#define MSG_STR_SET(arg, flag) do {\
    if (!strcmp(key, #arg)) {\
        if (value) {\
            ((CINetMsgEventRing*)msg)->arg = g_strdup((const gchar*)value);\
            ((CINetMsgEventRing*)msg)->fields |= flag;\
        }\
        else {\
            g_free(((CINetMsgEventRing*)msg)->arg);\
            ((CINetMsgEventRing*)msg)->arg = NULL;\
            ((CINetMsgEventRing*)msg)->fields &= ~flag;\
        }\
        return;\
    }} while(0)

    MSG_STR_SET(completenumber, CIF_COMPLETENUMBER);
    MSG_STR_SET(areacode, CIF_AREACODE);
    MSG_STR_SET(number, CIF_NUMBER);
    MSG_STR_SET(date, CIF_DATE);
    MSG_STR_SET(time, CIF_TIME);
    MSG_STR_SET(msn, CIF_MSN);
    MSG_STR_SET(alias, CIF_ALIAS);
    MSG_STR_SET(area, CIF_AREA);
    MSG_STR_SET(name, CIF_NAME);

#undef MSG_STR_SET
}

void cinet_msg_event_ring_free(CINetMsg *msg)
{
    g_free(((CINetMsgEventRing*)msg)->completenumber);
    g_free(((CINetMsgEventRing*)msg)->areacode);
    g_free(((CINetMsgEventRing*)msg)->number);
    g_free(((CINetMsgEventRing*)msg)->date);
    g_free(((CINetMsgEventRing*)msg)->time);
    g_free(((CINetMsgEventRing*)msg)->msn);
    g_free(((CINetMsgEventRing*)msg)->alias);
    g_free(((CINetMsgEventRing*)msg)->area);
    g_free(((CINetMsgEventRing*)msg)->name);
}

JsonNode *cinet_msg_default_build(CINetMsg *msg)
{
    JsonNode *node = json_node_new(JSON_NODE_OBJECT);
    return node;
}
