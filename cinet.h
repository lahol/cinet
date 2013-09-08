#ifndef __CINET_H__
#define __CINET_H__

#include <glib.h>
#include <cinetmsgs.h>

/* 6 bytes magic string, 4 bytes len, 4 bytes type */
#define CINET_HEADER_LENGTH            14

gssize cinet_msg_write_header(gchar *data, gsize len, CINetMsgHeader *header);
gssize cinet_msg_read_header(CINetMsgHeader *header, gchar *data, gsize len);

gint cinet_msg_write_msg(gchar **buffer, gsize *len, CINetMsg *msg);
gint cinet_msg_read_msg(CINetMsg **msg, gchar *buffer, gsize len);

gpointer cinet_msg_alloc(CINetMsgType msgtype);
void cinet_msg_free(CINetMsg *msg);

CINetMsg *cinet_message_new(CINetMsgType msgtype, ...);
gint cinet_message_new_for_data(gchar **buffer, gsize *len, guint msgtype, ...);
void cinet_message_set_value(CINetMsg *msg, const gchar *key, const gpointer value);

CICallInfo *cinet_call_info_new(void);
void cinet_call_info_free(CICallInfo *info);
void cinet_call_info_free_full(CICallInfo *info);
void cinet_call_info_copy(CICallInfo *dst, CICallInfo *src);
void cinet_call_info_set_value(CICallInfo *info, const gchar *key, const gpointer value);

CICallerInfo *cinet_caller_info_new(void);
void cinet_caller_info_free(CICallerInfo *info);
void cinet_caller_info_free_full(CICallerInfo *info);
void cinet_caller_info_copy(CICallerInfo *dst, CICallerInfo *src);
void cinet_caller_info_set_value(CICallerInfo *info, const gchar *key, const gpointer value);

#endif
