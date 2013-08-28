#ifndef __CINETMSGS_H__
#define __CINETMSGS_H__

#include <glib.h>

typedef enum {
    CI_NET_MSG_VERSION = 0,           /* Version */
    CI_NET_MSG_EVENT_RING,            /* ring event */
    CI_NET_MSG_EVENT_CALL,            /* outgoing call event */
    CI_NET_MSG_EVENT_CONNECT,         /* connected call */
    CI_NET_MSG_EVENT_DISCONNECT,      /* disconnected call */
    CI_NET_MSG_LEAVE,                 /* client leaves */
    CI_NET_MSG_SHUTDOWN,              /* server shuts down */
    CI_NET_MSG_COUNT                  /* total number of messages */
} CINetMsgType;

typedef struct {
    CINetMsgType msgtype;
    guint32 msglen;
} CINetMsgHeader;

typedef struct {
    CINetMsgType msgtype;
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

typedef enum {
    MultipartStageInit = 0,
    MultipartStageUpdate,
    MultipartStageComplete
} CINetMsgMultipartStage;

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
} CINetMsgEventRing;

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

typedef CINetMsg CINetMsgLeave;
typedef CINetMsg CINetMsgShutdown;

#endif
