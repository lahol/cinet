#ifndef __CINET_H__
#define __CINET_H__

#include <glib.h>
#include <cinetmsgs.h>

/* 6 bytes magic string, 4 bytes len, 4 bytes type */
#define CINET_HEADER_LENGTH            14

/* Write data from @header to the buffer given by @data which is at least
 * @len bytes long. The buffer should be at least @CINET_HEADER_LENGTH
 * bytes long. On success the number of bytes written is returned, -1 otherwise.
 *
 * @data:   A buffer to write the data to.
 * @len:    The size of the buffer.
 * @header: Header data.
 *
 * @return: Number of bytes written or -1 if an error occured.
 */
gssize cinet_msg_write_header(gchar *data, gsize len, CINetMsgHeader *header);

/* Read header data from raw @data to @header. @len is the size of @data.
 * Returns the number of bytes read or -1 on failure.
 *
 * @header: Pointer to the header data.
 * @data:   Buffer holding the message for which the header should be read.
 * @len:    Number of bytes in the buffer.
 *
 * @return: Number of bytes read or -1 if an error occured.
 */
gssize cinet_msg_read_header(CINetMsgHeader *header, gchar *data, gsize len);

/* Convert a message to raw data to be passed over the network.
 * The buffer is allocated to fit the data an its size is returned in @len.
 *
 * @buffer: Pointer to hold the newly allocated message data. Free with @g_free().
 * @len:    Number of bytes in the buffer (header and payload).
 * @msg:    The message to be converted.
 *
 * @return: 0 on success, -1 otherwise.
 */
gint cinet_msg_write_msg(gchar **buffer, gsize *len, CINetMsg *msg);

/* Convert raw message data from the network to a CINetMsg.
 * The memory for the message will be allocated according to the message type.
 *
 * @msg:    Return location of the newly allocated message. Free with
 *          @cinet_msg_free().
 * @buffer: Buffer holding the raw message data.
 * @len:    Size of the buffer in bytes.
 *
 * @return: 0 on success, -1 otherwise.
 */
gint cinet_msg_read_msg(CINetMsg **msg, gchar *buffer, gsize len);

/* Allocate memory for a message of a given type.
 *
 * @msgtype: The type of message.
 *
 * @return:  A newly allocated @CINetMsg of the given type or NULL if the type
 *           is not known. Free with @cinet_msg_free().
 */
CINetMsg *cinet_msg_alloc(CINetMsgType msgtype);

/* Free memory used by a @CINetMsg and associated data.
 *
 * @msg:     The message to be freed.
 */
void cinet_msg_free(CINetMsg *msg);

/* Create a new @CINetMsg of the given type and initialize with the given data.
 *
 * @msgtype: Type of the new message.
 * @...:     List of key-value-pairs with initial data. End with two NULL.
 *
 * @return:  The new message. Free with @cinet_msg_free().
 */
CINetMsg *cinet_message_new(CINetMsgType msgtype, ...);

/* Convenince wrapper around @cinet_message_new() and @cinet_msg_write_msg().
 *
 * @buffer:  Return location for the message data. Free with @g_free().
 * @len:     Size of the buffer.
 * @msgtype: Type of the new message.
 * @...:     List of key-value-pairs with the message data. End with two NULL.
 *
 * @return:  0 on success. -1 on error.
 */
gint cinet_message_new_for_data(gchar **buffer, gsize *len, guint msgtype, ...);

/* Set a member of a message to the given value.
 *
 * @msg:     The message.
 * @key:     The key of the member to be set.
 * @value:   The new value. For integers and boolean use GPOINTER_TO_INT (or _UINT).
 */
void cinet_message_set_value(CINetMsg *msg, const gchar *key, const gpointer value);

/* Allocate memory for a new call info.
 *
 * @return:  The new @CICallInfo. Free with @cinet_call_info_free_full().
 */
CICallInfo *cinet_call_info_new(void);

/* Initialize a @CICallInfo. Should be used for static members.
 *
 * @info:    A pointer to the @CICallInfo.
 */
void cinet_call_info_init(CICallInfo *info);

/* Free memory used by a @CICallInfo. This should be used for static members since it
 * doesn’t free the structure itself.
 *
 * @info:    A pointer to the @CICallInfo.
 */
void cinet_call_info_free(CICallInfo *info);

/* Free memory used by a @CICallInfo and free the structure itself.
 *
 * @info:    A pointer to the @CICallInfo.
 */
void cinet_call_info_free_full(CICallInfo *info);

/* Copy data from one @CICallInfo to another.
 *
 * @dst:     Pointer to the location to copy to.
 * @src:     Pointer to the location copied from.
 */
void cinet_call_info_copy(CICallInfo *dst, CICallInfo *src);

/* Set a member of a @CICallInfo to the given value.
 *
 * @info:    The @CICallInfo.
 * @key:     The key of the member to be set.
 * @value:   The new value. For integers an boolean use GPOINTER_TO_INT (or _UINT).
 */
void cinet_call_info_set_value(CICallInfo *info, const gchar *key, const gpointer value);

CICallerInfo *cinet_caller_info_new(void);
void cinet_caller_info_init(CICallerInfo *info);
void cinet_caller_info_free(CICallerInfo *info);
void cinet_caller_info_free_full(CICallerInfo *info);
void cinet_caller_info_copy(CICallerInfo *dst, CICallerInfo *src);
void cinet_caller_info_set_value(CICallerInfo *info, const gchar *key, const gpointer value);

#endif
