#ifndef __CINET_H__
#define __CINET_H__

#include <glib.h>

typedef struct {
    guint32 msgtype;
    guint32 msglen;
} CINetMsgHeader;

typedef struct {
    guint32 msgtype;
} CINetMsg;

typedef struct {
    CINetMsg parent;
    gint major;
    gint minor;
    gint patch;
    gchar *human_readable;
} CINetMsgVersion;

typedef struct {
    CINetMsg parent;
    gint stage;            /* init, update, complete */
    gint part;             /* 0, 1, … (for future use) */
    gchar msgid[16];       /* e.g. YYYYMMDDHHMMSSR */
} CINetMsgMultipart;

typedef struct {
    CINetMsgMultipart parent;
    gchar *completenumber;
    gchar *areacode;
    gchar *number;
    gchar *date;
    gchar *time;
    gchar *msn;
    gchar *alias;
    gchar *area;
    gchar *name;
    guint32 fields;
} CINetMsgRing;

typedef enum {
    CIF_COMPLETENUMBER = (1<<0),
    CIF_AREACODE = (1<<1),
    CIF_NUMBER = (1<<2),
    CIF_DATE = (1<<3),
    CIF_TIME = (1<<4),
    CIF_MSN = (1<<5),
    CIF_ALIAS = (1<<6),
    CIF_AREA = (1<<7),
    CIF_NAME = (1<<8)
} CINetMsgCallFields;

#define CI_NET_MSG_VERSION              0
#define CI_NET_MSG_EVENT_RING           1
#define CI_NET_MSG_EVENT_CALL           2
#define CI_NET_MSG_EVENT_CONNECT        3
#define CI_NET_MSG_EVENT_DISCONNECT     4

#define CI_NET_MSG_COUNT                5

/* 6 bytes magic string, 4 bytes len, 4 bytes type */
#define CINET_HEADER_LENGTH            14

gsize cinet_msg_write_header(gchar *data, gsize len, CINetMsgHeader *header);
gsize cinet_msg_read_header(CINetMsgHeader *header, gchar *data, gsize len);

gint cinet_msg_write_msg(gchar **buffer, gsize *len, CINetMsg *msg);
gint cinet_msg_read_msg(CINetMsg **msg, gchar *buffer, gsize len);

gpointer cinet_msg_alloc(guint32 msgtype);
void cinet_msg_free(CINetMsg *msg);

CINetMsg *cinet_message_new(guint32 msgtype, ...);
void cinet_message_set_value(CINetMsg *msg, const gchar *key, const gpointer value);

#endif
