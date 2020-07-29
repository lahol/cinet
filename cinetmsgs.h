#ifndef __CINETMSGS_H__
#define __CINETMSGS_H__

#include <glib.h>

/* CallerInfo messages used in the client-server communication. */

/* Message types */
typedef enum {
    CI_NET_MSG_VERSION = 0,           /* Version */
    CI_NET_MSG_EVENT_RING,            /* ring event */
    CI_NET_MSG_EVENT_CALL,            /* outgoing call event */
    CI_NET_MSG_EVENT_CONNECT,         /* connected call */
    CI_NET_MSG_EVENT_DISCONNECT,      /* disconnected call */
    CI_NET_MSG_LEAVE,                 /* client leaves */
    CI_NET_MSG_SHUTDOWN,              /* server shuts down */
    CI_NET_MSG_DB_NUM_CALLS,          /* get number of db entries */
    CI_NET_MSG_DB_CALL_LIST,          /* get call list */
    CI_NET_MSG_DB_GET_CALLER,         /* get caller information */
    CI_NET_MSG_DB_ADD_CALLER,         /* add caller to db */
    CI_NET_MSG_DB_DEL_CALLER,         /* delete caller from list */
    CI_NET_MSG_DB_GET_CALLER_LIST,    /* get list of callers matching a filter */
    CI_NET_MSG_COUNT,                 /* total number of messages */
    CI_NET_MSG_INVALID = 32767        /* invalid message type */
} CINetMsgType;

/* Message header */
typedef struct {
    CINetMsgType msgtype;             /* Type of the message. */
    guint32 msglen;                   /* Size of message payload. */
} CINetMsgHeader;

/* Base class for all messages. Not used directly. */
typedef struct {
    CINetMsgType msgtype;             /* Type of the message */
    guint32 guid;                     /* Application-specific identifier. This is intended to be
                                         returned in answers which may be useful for queries. */
} CINetMsg;

/* Version of the protocol. Should be at least 3.0.0. */
typedef struct {
    CINetMsg parent;                  /* Derived from CINetMsg. */
    gint major;                       /* Major version number */
    gint minor;                       /* Minor version number */
    gint patch;                       /* Patch version number */
    gchar *human_readable;            /* Version string to print. */
} CINetMsgVersion;

/* Stages for multipart messages.*/
typedef enum {
    MultipartStageInit = 0,
    MultipartStageUpdate,
    MultipartStageComplete
} CINetMsgMultipartStage;

/* Multipart message. Not used directly. 
 * This is used for a sequence of messages. At least two parts (Init and Complete)
 * should be sent. */
typedef struct {
    CINetMsg parent;                  /* Derived from CINetMsg. */
    CINetMsgMultipartStage stage;     /* Indicates the stage this particular message belongs to. */
    gint part;                        /* Consequtively numbered part of the message. Currently not used. */
    gchar msgid[16];                  /* Application defined id of the message. Should be the same for all stages
                                         and unique for this message. */
} CINetMsgMultipart;

/* Detailed information about a call. */
typedef struct {
    gint32 id;                        /* Index in the database. */
    gchar *completenumber;            /* Complete number */
    gchar *areacode;                  /* Area code obtained from the number. */
    gchar *number;                    /* Number without area code. */
    gchar *date;                      /* Date of the message. */
    gchar *time;                      /* Time of the message. */
    gchar *msn;                       /* Multiple subscriber number. */
    gchar *alias;                     /* Alias for the msn. */
    gchar *area;                      /* Area the number belongs to. */
    gchar *name;                      /* Name of the caller. */
    guint32 fields;                   /* Fields set. */
} CICallInfo;

/* Detailed information about a caller. */
typedef struct {
    gchar *number;                    /* The number of the caller including the area code. */
    gchar *name;                      /* The name of the caller. */
} CICallerInfo;

/* RING message. Someone calls. */
typedef struct {
    CINetMsgMultipart parent;         /* This is a multipart message. */
    CICallInfo callinfo;              /* Information about the call, embedded in the message. */
} CINetMsgEventRing;

/* CALL message. Outgoing call. */
typedef struct {
    CINetMsg parent;                  /* Derived from CINetMsg. */
    CICallInfo callinfo;              /* Information about the call, embedded in the message. */
} CINetMsgEventCall;

/* Flags used in CICallInfo indicating the fields that are valid. */
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

/* Clients send a leave message to the server to indicate that the connection may be terminated.
 * No further data is sent so this is just a typedef for convenience. */
typedef CINetMsg CINetMsgLeave;

/* The server sends all clients a shutdown message to indicate that the client should close
 * the connection. No further data is sent so this is just a typedef for convenience. */
typedef CINetMsg CINetMsgShutdown;

/* Get the total count of entries in the database. */
typedef struct {
    CINetMsg parent;                   /* Derived from CINetMsg. */
    gint count;                        /* Number of entries in the database. */
} CINetMsgDbNumCalls;

/* Get a list of calls from the database. */
typedef struct {
    CINetMsg parent;                   /* Derived from CINetMsg. */
    gint user;                         /* The user id for custom entries. */
    gint offset;                       /* Offset for the query. */
    gint count;                        /* Number of entries queried. */
    GList *calls;                      /* List of Calls. [element-type: CICallInfo] */
} CINetMsgDbCallList;

/* Get information about a caller. */
typedef struct {
    CINetMsg parent;                   /* Derived from CINetMsg. */
    gint user;                         /* The user id for custom entries. */
    CICallerInfo caller;               /* Information about the caller, embedded in the message. */
} CINetMsgDbGetCaller;

/* Add or update a caller in the database. */
typedef struct {
    CINetMsg parent;                   /* Derived from CINetMsg. */
    gint user;                         /* The user id for custom entries. */
    CICallerInfo caller;               /* Information about the caller, embedded in the message. */
} CINetMsgDbAddCaller;

/* Delete a caller from the database. */
typedef struct {
    CINetMsg parent;                   /* Derived from CINetMsg. */
    gint user;                         /* The user id for custom entries. */
    CICallerInfo caller;               /* Information about the caller, embedded in the message. */
} CINetMsgDbDelCaller;

/* Get a list of all callers. */
typedef struct {
    CINetMsg parent;                   /* Derived from CINetMsg. */
    gint user;                         /* The user id for custom entries. */
    gchar *filter;                     /* Only get entries containing this string. */
    GList *callers;                    /* List of all callers matching the filter. [element-type: CICallerInfo] */
} CINetMsgDbGetCallerList;

#endif
