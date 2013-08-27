#include "cinet.h"
#include <stdio.h>

int main(int argc, char **argv)
{
/*    CINetMsgVersion msg;
    ((CINetMsg*)&msg)->msgtype = CI_NET_MSG_VERSION;
    msg.major = 3;
    msg.minor = 0;
    msg.patch = 0;
    msg.human_readable = g_strdup("3.0.0 \"(branch master)\"");*/
    CINetMsg *msg = cinet_message_new(CI_NET_MSG_VERSION,
            "major", 3, "minor", 0, "patch", 0, "human_readable", "3.0.0 (branch master)", NULL, NULL);

    gchar *buffer = NULL;
    gsize i, len = 0;
    cinet_msg_write_msg(&buffer, &len, msg);

    fprintf(stderr, "len: %d\n", len);

/*    for (i = 0; i < len; ++i) {
        fprintf(stdout, "%c (%02x)\n", buffer[i], buffer[i]);
    }*/

    CINetMsg *message = NULL;

    cinet_msg_read_msg(&message, buffer, len);
    if (message && message->msgtype == CI_NET_MSG_VERSION) {
        fprintf(stdout, "major: %d\n", ((CINetMsgVersion*)message)->major);
        fprintf(stdout, "minor: %d\n", ((CINetMsgVersion*)message)->minor);
        fprintf(stdout, "patch: %d\n", ((CINetMsgVersion*)message)->patch);
        fprintf(stdout, "human: %s\n", ((CINetMsgVersion*)message)->human_readable);
    }

    cinet_msg_free(message);

    g_free(buffer);

    return 0;
}
