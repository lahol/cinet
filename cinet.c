#include "cinet.h"
#include <string.h>
#include <json-glib/json-glib.h>
#include <json-glib/json-gobject.h>
#include <stdio.h>
#include <stdarg.h>
#include <glib/gprintf.h>

struct CINetMsgClass {
    CINetMsgType msgtype;
    gsize size;
    JsonNode *(*msg_build)(CINetMsg *);
    CINetMsg *(*msg_read)(JsonNode *);
    void (*msg_free)(CINetMsg *);
    void (*msg_set_value)(CINetMsg *, const gchar *, const gpointer);
};

JsonNode *cinet_msg_default_build(CINetMsg *msg);

void cinet_call_info_build(CICallInfo *info, JsonBuilder *builder);
void cinet_call_info_read(CICallInfo *info, JsonObject *obj);

void cinet_caller_info_build(CICallerInfo *info, JsonBuilder *builder);
void cinet_caller_info_read(CICallerInfo *info, JsonObject *obj);

JsonNode *cinet_msg_version_build(CINetMsg *msg);
CINetMsg *cinet_msg_version_read(JsonNode *root);
void cinet_msg_version_set_value(CINetMsg *msg, const gchar *key, const gpointer value);
void cinet_msg_version_free(CINetMsg *msg);

JsonNode *cinet_msg_event_ring_build(CINetMsg *msg);
CINetMsg *cinet_msg_event_ring_read(JsonNode *root);
void cinet_msg_event_ring_set_value(CINetMsg *msg, const gchar *key, const gpointer value);
void cinet_msg_event_ring_free(CINetMsg *msg);

JsonNode *cinet_msg_event_call_build(CINetMsg *msg);
CINetMsg *cinet_msg_event_call_read(JsonNode *root);
void cinet_msg_event_call_set_value(CINetMsg *msg, const gchar *key, const gpointer value);
void cinet_msg_event_call_free(CINetMsg *msg);

JsonNode *cinet_msg_db_num_calls_build(CINetMsg *msg);
CINetMsg *cinet_msg_db_num_calls_read(JsonNode *root);
void cinet_msg_db_num_calls_set_value(CINetMsg *msg, const gchar *key, const gpointer value);

JsonNode *cinet_msg_db_call_list_build(CINetMsg *msg);
CINetMsg *cinet_msg_db_call_list_read(JsonNode *root);
void cinet_msg_db_call_list_set_value(CINetMsg *msg, const gchar *key, const gpointer value);
void cinet_msg_db_call_list_free(CINetMsg *msg);

JsonNode *cinet_msg_db_get_caller_build(CINetMsg *msg);
CINetMsg *cinet_msg_db_get_caller_read(JsonNode *root);
void cinet_msg_db_get_caller_set_value(CINetMsg *msg, const gchar *key, const gpointer value);
void cinet_msg_db_get_caller_free(CINetMsg *msg);

JsonNode *cinet_msg_db_add_caller_build(CINetMsg *msg);
CINetMsg *cinet_msg_db_add_caller_read(JsonNode *root);
void cinet_msg_db_add_caller_set_value(CINetMsg *msg, const gchar *key, const gpointer value);
void cinet_msg_db_add_caller_free(CINetMsg *msg);

JsonNode *cinet_msg_db_del_caller_build(CINetMsg *msg);
CINetMsg *cinet_msg_db_del_caller_read(JsonNode *root);
void cinet_msg_db_del_caller_set_value(CINetMsg *msg, const gchar *key, const gpointer value);
void cinet_msg_db_del_caller_free(CINetMsg *msg);

JsonNode *cinet_msg_db_get_caller_list_build(CINetMsg *msg);
CINetMsg *cinet_msg_db_get_caller_list_read(JsonNode *root);
void cinet_msg_db_get_caller_list_set_value(CINetMsg *msg, const gchar *key, const gpointer value);
void cinet_msg_db_get_caller_list_free(CINetMsg *msg);

static struct CINetMsgClass msgclasses[] = {
    { CI_NET_MSG_VERSION, sizeof(CINetMsgVersion), cinet_msg_version_build,
        cinet_msg_version_read, cinet_msg_version_free, cinet_msg_version_set_value},
    { CI_NET_MSG_EVENT_RING, sizeof(CINetMsgEventRing), cinet_msg_event_ring_build,
        cinet_msg_event_ring_read, cinet_msg_event_ring_free, cinet_msg_event_ring_set_value },
    { CI_NET_MSG_EVENT_CALL, sizeof(CINetMsgEventCall), cinet_msg_event_call_build,
        cinet_msg_event_call_read, cinet_msg_event_call_free, cinet_msg_event_call_set_value },
    { CI_NET_MSG_EVENT_CONNECT, sizeof(CINetMsg), NULL, NULL, NULL, NULL },
    { CI_NET_MSG_EVENT_DISCONNECT, sizeof(CINetMsg), NULL, NULL, NULL, NULL },
    { CI_NET_MSG_LEAVE, sizeof(CINetMsgLeave), NULL, NULL, NULL, NULL },
    { CI_NET_MSG_SHUTDOWN, sizeof(CINetMsgShutdown), NULL, NULL, NULL, NULL },
    { CI_NET_MSG_DB_NUM_CALLS, sizeof(CINetMsgDbNumCalls), cinet_msg_db_num_calls_build,
        cinet_msg_db_num_calls_read, NULL, cinet_msg_db_num_calls_set_value },
    { CI_NET_MSG_DB_CALL_LIST, sizeof(CINetMsgDbCallList), cinet_msg_db_call_list_build,
        cinet_msg_db_call_list_read, cinet_msg_db_call_list_free, cinet_msg_db_call_list_set_value },
    { CI_NET_MSG_DB_GET_CALLER, sizeof(CINetMsgDbGetCaller), cinet_msg_db_get_caller_build,
        cinet_msg_db_get_caller_read, cinet_msg_db_get_caller_free, cinet_msg_db_get_caller_set_value },
    { CI_NET_MSG_DB_ADD_CALLER, sizeof(CINetMsgDbAddCaller), cinet_msg_db_add_caller_build,
        cinet_msg_db_add_caller_read, cinet_msg_db_add_caller_free, cinet_msg_db_add_caller_set_value },
    { CI_NET_MSG_DB_DEL_CALLER, sizeof(CINetMsgDbDelCaller), cinet_msg_db_del_caller_build,
        cinet_msg_db_del_caller_read, cinet_msg_db_del_caller_free, cinet_msg_db_del_caller_set_value },
    { CI_NET_MSG_DB_GET_CALLER_LIST, sizeof(CINetMsgDbGetCallerList), cinet_msg_db_get_caller_list_build,
        cinet_msg_db_get_caller_list_read, cinet_msg_db_get_caller_list_free, cinet_msg_db_get_caller_list_set_value },
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
                if (!g_strcmp0(key, "guid"))
                    cinet_message_set_value(msg, key, val);
                else
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
    if (g_strcmp0(key, "guid") == 0)
        msg->guid = GPOINTER_TO_UINT(value);
    else
        cls->msg_set_value(msg, key, value);
}

JsonNode *cinet_msg_version_build(CINetMsg *msg)
{
    CINetMsgVersion *cmsg = (CINetMsgVersion*)msg;
    JsonBuilder *builder = json_builder_new();
    JsonNode *root;

    json_builder_begin_object(builder);

    json_builder_set_member_name(builder, "guid");
    json_builder_add_int_value(builder, msg->guid);

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
    ((CINetMsg*)msg)->guid = (guint32)json_object_get_int_member(obj, "guid");

    msg->major = (guint32)json_object_get_int_member(obj, "major");
    msg->minor = (guint32)json_object_get_int_member(obj, "minor");
    msg->patch = (guint32)json_object_get_int_member(obj, "patch");
    msg->human_readable = g_strdup(json_object_get_string_member(obj, "human_readable"));

    return (CINetMsg*)msg;
}

void cinet_msg_version_set_value(CINetMsg *msg, const gchar *key, const gpointer value)
{
    if (!msg || !key || msg->msgtype != CI_NET_MSG_VERSION)
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

void cinet_call_info_build(CICallInfo *info, JsonBuilder *builder)
{
    if (info == NULL || builder == NULL)
        return;

    json_builder_set_member_name(builder, "id");
    json_builder_add_int_value(builder, info->id);

#define MSG_BUILD_STR(arg) do {\
    if (info->arg) {\
        json_builder_set_member_name(builder, #arg);\
        json_builder_add_string_value(builder, info->arg);\
    }\
} while (0)

    MSG_BUILD_STR(completenumber);
    MSG_BUILD_STR(areacode);
    MSG_BUILD_STR(number);
    MSG_BUILD_STR(date);
    MSG_BUILD_STR(time);
    MSG_BUILD_STR(msn);
    MSG_BUILD_STR(alias);
    MSG_BUILD_STR(area);
    MSG_BUILD_STR(name);
#undef MSG_BUILD_STR
}

void cinet_call_info_read(CICallInfo *info, JsonObject *obj)
{
    if (info == NULL || obj == NULL)
        return;

    if (json_object_has_member(obj, "id"))
        cinet_call_info_set_value(info, "id",
                GINT_TO_POINTER(json_object_get_int_member(obj, "id")));

#define MSG_STR_SET(arg) do {\
    if (json_object_has_member(obj, arg)) \
        cinet_call_info_set_value(info, arg, (const gpointer)json_object_get_string_member(obj, arg));\
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
}

void cinet_caller_info_build(CICallerInfo *info, JsonBuilder *builder)
{
    if (info == NULL || builder == NULL)
        return;

#define MSG_BUILD_STR(arg) do {\
    if (info->arg) {\
        json_builder_set_member_name(builder, #arg);\
        json_builder_add_string_value(builder, info->arg);\
    }\
} while (0)

    MSG_BUILD_STR(number);
    MSG_BUILD_STR(name);

#undef MSG_BUILD_STR
}

void cinet_caller_info_read(CICallerInfo *info, JsonObject *obj)
{
    if (info == NULL || obj == NULL)
        return;

#define MSG_STR_SET(arg) do {\
    if (json_object_has_member(obj, #arg))\
        cinet_caller_info_set_value(info, #arg, (const gpointer)json_object_get_string_member(obj, #arg));\
    } while (0)

    MSG_STR_SET(number);
    MSG_STR_SET(name);

#undef MSG_STR_SET
}

void cinet_call_info_set_value(CICallInfo *info, const gchar *key, const gpointer value)
{
    if (info == NULL || key == NULL)
        return;
    if (!strcmp(key, "id")) {
        info->id = GPOINTER_TO_INT(value);
        return;
    }
#define MSG_STR_SET(arg, flag) do {\
    if (!strcmp(key, #arg)) {\
        g_free(info->arg);\
        if (value) {\
            info->arg = g_strdup((const gchar*)value);\
            info->fields |= flag;\
        }\
        else {\
            info->arg = NULL;\
            info->fields &= ~flag;\
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

CICallInfo *cinet_call_info_new(void)
{
    return (CICallInfo*)g_malloc0(sizeof(CICallInfo));
}

void cinet_call_info_copy(CICallInfo *dst, CICallInfo *src)
{
    if (dst == NULL || src == NULL || dst == src)
        return;
    dst->id = src->id;
    dst->fields = src->fields;
#define CPY_STR(arg) do { g_free(dst->arg); dst->arg = g_strdup(src->arg); } while (0)
    CPY_STR(completenumber);
    CPY_STR(areacode);
    CPY_STR(number);
    CPY_STR(date);
    CPY_STR(time);
    CPY_STR(msn);
    CPY_STR(alias);
    CPY_STR(area);
    CPY_STR(name);
#undef CPY_STR
}

void cinet_call_info_free(CICallInfo *info)
{
    if (info == NULL)
        return;
    g_free(info->completenumber);
    g_free(info->areacode);
    g_free(info->number);
    g_free(info->date);
    g_free(info->time);
    g_free(info->msn);
    g_free(info->alias);
    g_free(info->area);
    g_free(info->name);
}

void cinet_call_info_free_full(CICallInfo *info)
{
    cinet_call_info_free(info);
    g_free(info);
}

CICallerInfo *cinet_caller_info_new(void)
{
    return (CICallerInfo*)g_malloc0(sizeof(CICallerInfo));
}

void cinet_caller_info_free(CICallerInfo *info)
{
    if (info == NULL)
        return;
    g_free(info->number);
    g_free(info->name);
}

void cinet_caller_info_free_full(CICallerInfo *info)
{
    cinet_caller_info_free(info);
    g_free(info);
}

void cinet_caller_info_copy(CICallerInfo *dst, CICallerInfo *src)
{
    if (dst == NULL || src == NULL || dst == src)
        return;

#define CPY_STR(arg) do { g_free(dst->arg); dst->arg = g_strdup(src->arg); } while (0)
    CPY_STR(number);
    CPY_STR(name);
#undef CPY_STR
}

void cinet_caller_info_set_value(CICallerInfo *info, const gchar *key, const gpointer value)
{
    if (info == NULL || key == NULL)
        return;
#define MSG_STR_SET(arg) do {\
    if (!strcmp(key, #arg)) {\
        g_free(info->arg);\
        if (value)\
            info->arg = g_strdup((const gchar*)value);\
        else {\
            info->arg = NULL;\
        }\
        return;\
    }} while (0)

    MSG_STR_SET(number);
    MSG_STR_SET(name);

#undef MSG_STR_SET
}

JsonNode *cinet_msg_event_ring_build(CINetMsg *msg)
{
    CINetMsgEventRing *cmsg = (CINetMsgEventRing*)msg;
    JsonBuilder *builder = json_builder_new();
    JsonNode *root;

    json_builder_begin_object(builder);

    json_builder_set_member_name(builder, "guid");
    json_builder_add_int_value(builder, msg->guid);

    json_builder_set_member_name(builder, "stage");
    json_builder_add_int_value(builder, ((CINetMsgMultipart*)msg)->stage);

    json_builder_set_member_name(builder, "part");
    json_builder_add_int_value(builder, ((CINetMsgMultipart*)msg)->part);

    json_builder_set_member_name(builder, "msgid");
    json_builder_add_string_value(builder, ((CINetMsgMultipart*)msg)->msgid);

    cinet_call_info_build(&cmsg->callinfo, builder);

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
    ((CINetMsg*)msg)->guid = (guint32)json_object_get_int_member(obj, "guid");

    cinet_msg_event_ring_set_value((CINetMsg*)msg, "stage", GINT_TO_POINTER(json_object_get_int_member(obj, "stage")));
    cinet_msg_event_ring_set_value((CINetMsg*)msg, "part", GINT_TO_POINTER(json_object_get_int_member(obj, "part")));

    if (json_object_has_member(obj, "msgid"))
        cinet_msg_event_ring_set_value((CINetMsg*)msg, "msgid",
                (gpointer)json_object_get_string_member(obj, "msgid"));

    cinet_call_info_read(&msg->callinfo, obj);

    return (CINetMsg*)msg;
}

void cinet_msg_event_ring_set_value(CINetMsg *msg, const gchar *key, const gpointer value)
{
    if (!msg || !key || msg->msgtype != CI_NET_MSG_EVENT_RING)
        return;
    if (!strcmp(key, "msgid")) {
        g_strlcpy(((CINetMsgMultipart*)msg)->msgid, value, 15);
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
    
    cinet_call_info_set_value(&((CINetMsgEventRing*)msg)->callinfo, key, value);
}

void cinet_msg_event_ring_free(CINetMsg *msg)
{
    cinet_call_info_free(&((CINetMsgEventRing*)msg)->callinfo);
}

JsonNode *cinet_msg_event_call_build(CINetMsg *msg)
{
    CINetMsgEventCall *cmsg = (CINetMsgEventCall*)msg;
    JsonBuilder *builder = json_builder_new();
    JsonNode *root;

    json_builder_begin_object(builder);

    json_builder_set_member_name(builder, "guid");
    json_builder_add_int_value(builder, msg->guid);

    cinet_call_info_build(&cmsg->callinfo, builder);

    json_builder_end_object(builder);

    root = json_builder_get_root(builder);

    g_object_unref(builder);

    return root;
}

CINetMsg *cinet_msg_event_call_read(JsonNode *root)
{
    if (!JSON_NODE_HOLDS_OBJECT(root))
        return NULL;

    CINetMsgEventCall *msg = cinet_msg_alloc(CI_NET_MSG_EVENT_CALL);

    JsonObject *obj = json_node_get_object(root);
    ((CINetMsg*)msg)->guid = (guint32)json_object_get_int_member(obj, "guid");

    cinet_call_info_read(&msg->callinfo, obj);

    return (CINetMsg*)msg;
}

void cinet_msg_event_call_set_value(CINetMsg *msg, const gchar *key, const gpointer value)
{
    if (!msg || !key || msg->msgtype != CI_NET_MSG_EVENT_CALL)
        return;
    
    cinet_call_info_set_value(&((CINetMsgEventRing*)msg)->callinfo, key, value);
}

void cinet_msg_event_call_free(CINetMsg *msg)
{
    cinet_call_info_free(&((CINetMsgEventCall*)msg)->callinfo);
}

JsonNode *cinet_msg_db_num_calls_build(CINetMsg *msg)
{
    CINetMsgDbNumCalls *cmsg = (CINetMsgDbNumCalls*)msg;

    JsonBuilder *builder = json_builder_new();
    JsonNode *root;

    json_builder_begin_object(builder);

    json_builder_set_member_name(builder, "guid");
    json_builder_add_int_value(builder, msg->guid);

    json_builder_set_member_name(builder, "count");
    json_builder_add_int_value(builder, cmsg->count);

    json_builder_end_object(builder);

    root = json_builder_get_root(builder);
    g_object_unref(builder);

    return root;
}

CINetMsg *cinet_msg_db_num_calls_read(JsonNode *root)
{
    if (!JSON_NODE_HOLDS_OBJECT(root))
        return NULL;
    CINetMsgDbNumCalls *msg = cinet_msg_alloc(CI_NET_MSG_DB_NUM_CALLS);

    JsonObject *obj = json_node_get_object(root);
    ((CINetMsg*)msg)->guid = (guint32)json_object_get_int_member(obj, "guid");

    cinet_msg_db_num_calls_set_value((CINetMsg*)msg, "count",
            GINT_TO_POINTER(json_object_get_int_member(obj, "count")));

    return (CINetMsg*)msg;
}

void cinet_msg_db_num_calls_set_value(CINetMsg *msg, const gchar *key, const gpointer value)
{
    if (!msg || !key || msg->msgtype != CI_NET_MSG_DB_NUM_CALLS)
        return;

    if (!strcmp(key, "count")) {
        ((CINetMsgDbNumCalls*)msg)->count = GPOINTER_TO_INT(value);
        return;
    }
}

JsonNode *cinet_msg_db_call_list_build(CINetMsg *msg)
{
    JsonBuilder *builder = json_builder_new();
    JsonNode *root;
    GList *tmp;

    CINetMsgDbCallList *cmsg = (CINetMsgDbCallList*)msg;

    json_builder_begin_object(builder);

    json_builder_set_member_name(builder, "guid");
    json_builder_add_int_value(builder, msg->guid);

    json_builder_set_member_name(builder, "user");
    json_builder_add_int_value(builder, cmsg->user);

    json_builder_set_member_name(builder, "offset");
    json_builder_add_int_value(builder, cmsg->offset);

    json_builder_set_member_name(builder, "count");
    json_builder_add_int_value(builder, cmsg->count);

    json_builder_set_member_name(builder, "calls");
    json_builder_begin_array(builder);
    for (tmp = cmsg->calls; tmp != NULL; tmp = g_list_next(tmp)) {
        json_builder_begin_object(builder);
        cinet_call_info_build((CICallInfo*)tmp->data, builder);
        json_builder_end_object(builder);
    }
    json_builder_end_array(builder);

    json_builder_end_object(builder);

    root = json_builder_get_root(builder);
    g_object_unref(builder);

    return root;
}

CINetMsg *cinet_msg_db_call_list_read(JsonNode *root)
{
    if (!JSON_NODE_HOLDS_OBJECT(root))
        return NULL;
    CINetMsgDbCallList *msg = cinet_msg_alloc(CI_NET_MSG_DB_CALL_LIST);

    JsonObject *obj = json_node_get_object(root);

    ((CINetMsg*)msg)->guid = (guint32)json_object_get_int_member(obj, "guid");

    cinet_msg_db_call_list_set_value((CINetMsg*)msg, "user",
            GINT_TO_POINTER(json_object_get_int_member(obj, "user")));
    cinet_msg_db_call_list_set_value((CINetMsg*)msg, "offset",
            GINT_TO_POINTER(json_object_get_int_member(obj, "offset")));
    cinet_msg_db_call_list_set_value((CINetMsg*)msg, "count",
            GINT_TO_POINTER(json_object_get_int_member(obj, "count")));

    /* calls=array of cicallinfo objects */
    JsonArray *arr = json_node_get_array(json_object_get_member(obj, "calls"));
    GList *calls = json_array_get_elements(arr);
    GList *tmp;
    CICallInfo *info;

    for (tmp = calls; tmp != NULL; tmp = g_list_next(tmp)) {
        info = cinet_call_info_new();
        cinet_call_info_read(info, json_node_get_object((JsonNode*)tmp->data));
        msg->calls = g_list_prepend(msg->calls, (gpointer)info);
    }

    msg->calls = g_list_reverse(msg->calls);

    g_list_free(calls);

    return (CINetMsg*)msg;
}

void cinet_msg_db_call_list_set_value(CINetMsg *msg, const gchar *key, const gpointer value)
{
    if (!msg || !key || msg->msgtype != CI_NET_MSG_DB_CALL_LIST)
        return;

    if (!strcmp(key, "user")) {
        ((CINetMsgDbCallList*)msg)->user = GPOINTER_TO_INT(value);
        return;
    }
    if (!strcmp(key, "offset")) {
        ((CINetMsgDbCallList*)msg)->offset = GPOINTER_TO_INT(value);
        return;
    }
    if (!strcmp(key, "count")) {
        ((CINetMsgDbCallList*)msg)->count = GPOINTER_TO_INT(value);
        return;
    }
    if (!strcmp(key, "call")) {
        ((CINetMsgDbCallList*)msg)->calls = g_list_append(
            ((CINetMsgDbCallList*)msg)->calls, value);
        return;
    }
}

void cinet_msg_db_call_list_free(CINetMsg *msg)
{
    g_list_free_full(((CINetMsgDbCallList*)msg)->calls, (GDestroyNotify)cinet_call_info_free_full);
}

JsonNode *cinet_msg_db_get_caller_build(CINetMsg *msg)
{
    JsonBuilder *builder = json_builder_new();
    JsonNode *root;

    CINetMsgDbGetCaller *cmsg = (CINetMsgDbGetCaller*)msg;

    json_builder_begin_object(builder);

    json_builder_set_member_name(builder, "guid");
    json_builder_add_int_value(builder, msg->guid);

    json_builder_set_member_name(builder, "user");
    json_builder_add_int_value(builder, cmsg->user);

    cinet_caller_info_build(&cmsg->caller, builder);

    json_builder_end_object(builder);

    root = json_builder_get_root(builder);
    g_object_unref(builder);

    return root;
}

CINetMsg *cinet_msg_db_get_caller_read(JsonNode *root)
{
    if (!JSON_NODE_HOLDS_OBJECT(root))
        return NULL;

    CINetMsgDbGetCaller *msg = cinet_msg_alloc(CI_NET_MSG_DB_GET_CALLER);
    
    JsonObject *obj = json_node_get_object(root);

    ((CINetMsg*)msg)->guid = (guint32)json_object_get_int_member(obj, "guid");

    cinet_msg_db_get_caller_set_value((CINetMsg*)msg, "user", GINT_TO_POINTER(json_object_get_int_member(obj, "user")));

    cinet_caller_info_read(&msg->caller, obj);

    return (CINetMsg*)msg;
}

void cinet_msg_db_get_caller_set_value(CINetMsg *msg, const gchar *key, const gpointer value)
{
    if (!msg || !key || msg->msgtype != CI_NET_MSG_DB_GET_CALLER)
        return;

    if (!strcmp(key, "user")) {
        ((CINetMsgDbGetCaller*)msg)->user = GPOINTER_TO_INT(value);
        return;
    }

    cinet_caller_info_set_value(&((CINetMsgDbGetCaller*)msg)->caller, key, value);
}

void cinet_msg_db_get_caller_free(CINetMsg *msg)
{
    cinet_caller_info_free(&((CINetMsgDbGetCaller*)msg)->caller);
}

JsonNode *cinet_msg_db_add_caller_build(CINetMsg *msg)
{
    JsonBuilder *builder = json_builder_new();
    JsonNode *root;

    CINetMsgDbAddCaller *cmsg = (CINetMsgDbAddCaller*)msg;

    json_builder_begin_object(builder);

    json_builder_set_member_name(builder, "guid");
    json_builder_add_int_value(builder, msg->guid);

    json_builder_set_member_name(builder, "user");
    json_builder_add_int_value(builder, cmsg->user);

    cinet_caller_info_build(&cmsg->caller, builder);

    json_builder_end_object(builder);

    root = json_builder_get_root(builder);
    g_object_unref(builder);

    return root;
}

CINetMsg *cinet_msg_db_add_caller_read(JsonNode *root)
{
    if (!JSON_NODE_HOLDS_OBJECT(root))
        return NULL;

    CINetMsgDbAddCaller *msg = cinet_msg_alloc(CI_NET_MSG_DB_ADD_CALLER);
    
    JsonObject *obj = json_node_get_object(root);

    ((CINetMsg*)msg)->guid = (guint32)json_object_get_int_member(obj, "guid");

    cinet_msg_db_add_caller_set_value((CINetMsg*)msg, "user", GINT_TO_POINTER(json_object_get_int_member(obj, "user")));

    cinet_caller_info_read(&msg->caller, obj);

    return (CINetMsg*)msg;
}

void cinet_msg_db_add_caller_set_value(CINetMsg *msg, const gchar *key, const gpointer value)
{
    if (!msg || !key || msg->msgtype != CI_NET_MSG_DB_ADD_CALLER)
        return;

    if (!strcmp(key, "user")) {
        ((CINetMsgDbAddCaller*)msg)->user = GPOINTER_TO_INT(value);
        return;
    }

    cinet_caller_info_set_value(&((CINetMsgDbAddCaller*)msg)->caller, key, value);
}

void cinet_msg_db_add_caller_free(CINetMsg *msg)
{
    cinet_caller_info_free(&((CINetMsgDbAddCaller*)msg)->caller);
}

JsonNode *cinet_msg_db_del_caller_build(CINetMsg *msg)
{
    JsonBuilder *builder = json_builder_new();
    JsonNode *root;

    CINetMsgDbDelCaller *cmsg = (CINetMsgDbDelCaller*)msg;

    json_builder_begin_object(builder);

    json_builder_set_member_name(builder, "guid");
    json_builder_add_int_value(builder, msg->guid);

    json_builder_set_member_name(builder, "user");
    json_builder_add_int_value(builder, cmsg->user);

    cinet_caller_info_build(&cmsg->caller, builder);

    json_builder_end_object(builder);

    root = json_builder_get_root(builder);
    g_object_unref(builder);

    return root;
}

CINetMsg *cinet_msg_db_del_caller_read(JsonNode *root)
{
    if (!JSON_NODE_HOLDS_OBJECT(root))
        return NULL;

    CINetMsgDbDelCaller *msg = cinet_msg_alloc(CI_NET_MSG_DB_DEL_CALLER);
    
    JsonObject *obj = json_node_get_object(root);

    ((CINetMsg*)msg)->guid = (guint32)json_object_get_int_member(obj, "guid");

    cinet_msg_db_del_caller_set_value((CINetMsg*)msg, "user", GINT_TO_POINTER(json_object_get_int_member(obj, "user")));

    cinet_caller_info_read(&msg->caller, obj);

    return (CINetMsg*)msg;
}

void cinet_msg_db_del_caller_set_value(CINetMsg *msg, const gchar *key, const gpointer value)
{
    if (!msg || !key || msg->msgtype != CI_NET_MSG_DB_DEL_CALLER)
        return;

    if (!strcmp(key, "user")) {
        ((CINetMsgDbDelCaller*)msg)->user = GPOINTER_TO_INT(value);
        return;
    }

    cinet_caller_info_set_value(&((CINetMsgDbDelCaller*)msg)->caller, key, value);
}

void cinet_msg_db_del_caller_free(CINetMsg *msg)
{
    cinet_caller_info_free(&((CINetMsgDbDelCaller*)msg)->caller);
}

JsonNode *cinet_msg_db_get_caller_list_build(CINetMsg *msg)
{
    JsonBuilder *builder = json_builder_new();
    JsonNode *root;
    GList *tmp;

    CINetMsgDbGetCallerList *cmsg = (CINetMsgDbGetCallerList*)msg;

    json_builder_begin_object(builder);

    json_builder_set_member_name(builder, "guid");
    json_builder_add_int_value(builder, msg->guid);

    json_builder_set_member_name(builder, "user");
    json_builder_add_int_value(builder, cmsg->user);

    json_builder_set_member_name(builder, "filter");
    json_builder_add_string_value(builder, cmsg->filter);

    json_builder_set_member_name(builder, "callers");
    json_builder_begin_array(builder);
    for (tmp = cmsg->callers; tmp != NULL; tmp = g_list_next(tmp)) {
        json_builder_begin_object(builder);
        cinet_caller_info_build((CICallerInfo*)tmp->data, builder);
        json_builder_end_object(builder);
    }
    json_builder_end_array(builder);

    json_builder_end_object(builder);

    root = json_builder_get_root(builder);
    g_object_unref(builder);

    return root;
}

CINetMsg *cinet_msg_db_get_caller_list_read(JsonNode *root)
{
    if (!JSON_NODE_HOLDS_OBJECT(root))
        return NULL;

    CINetMsgDbGetCallerList *msg = cinet_msg_alloc(CI_NET_MSG_DB_GET_CALLER_LIST);
    
    JsonObject *obj = json_node_get_object(root);

    ((CINetMsg*)msg)->guid = (guint32)json_object_get_int_member(obj, "guid");

    cinet_msg_db_get_caller_list_set_value((CINetMsg*)msg, "user", GINT_TO_POINTER(json_object_get_int_member(obj, "user")));

    if (json_object_has_member(obj, "filter"))
        cinet_msg_db_get_caller_list_set_value((CINetMsg*)msg, "filter",
                (gpointer)json_object_get_string_member(obj, "filter"));

    JsonArray *arr = json_node_get_array(json_object_get_member(obj, "callers"));
    GList *callers = json_array_get_elements(arr);
    GList *tmp;
    CICallerInfo *info;

    for (tmp = callers; tmp != NULL; tmp = g_list_next(tmp)) {
        info = cinet_caller_info_new();
        cinet_caller_info_read(info, json_node_get_object((JsonNode*)tmp->data));
        msg->callers = g_list_prepend(msg->callers, (gpointer)info);
    }

    msg->callers = g_list_reverse(msg->callers);

    g_list_free(callers);

    return (CINetMsg*)msg;
}

void cinet_msg_db_get_caller_list_set_value(CINetMsg *msg, const gchar *key, const gpointer value)
{
    if (!msg || !key || msg->msgtype != CI_NET_MSG_DB_GET_CALLER_LIST)
        return;

    if (!strcmp(key, "user")) {
        ((CINetMsgDbGetCallerList*)msg)->user = GPOINTER_TO_INT(value);
        return;
    }

    if (!strcmp(key, "caller")) {
        ((CINetMsgDbGetCallerList*)msg)->callers = g_list_append(
            ((CINetMsgDbGetCallerList*)msg)->callers, value);
        return;
    }
    
    if (!strcmp(key, "filter")) {
        g_free(((CINetMsgDbGetCallerList*)msg)->filter);
        ((CINetMsgDbGetCallerList*)msg)->filter = g_strdup((const gchar *)value);
        return;
    }
}

void cinet_msg_db_get_caller_list_free(CINetMsg *msg)
{
    g_list_free_full(((CINetMsgDbGetCallerList*)msg)->callers,
            (GDestroyNotify)cinet_caller_info_free_full);
    g_free(((CINetMsgDbGetCallerList*)msg)->filter);
}

JsonNode *cinet_msg_default_build(CINetMsg *msg)
{
    JsonNode *node = json_node_new(JSON_NODE_OBJECT);
    return node;
}
