//
// Created by Cody on 3/23/2020.
//

#include "tmanager.h"

int send_message(int sockfd, struct sockaddr_in client, txMsgType* message) {
    printf("send_message: client: %d, message: (msgId: %s, tid: %d)\n", ntohs(client.sin_port), txMsgKindToStr(message->msgID), message->tid);
    int bytesSent;
    bytesSent = sendto(sockfd, (void *) message, sizeof(*message), 0, (struct sockaddr *) &client, sizeof(client));
    if (bytesSent != sizeof(*message)) {
        perror("send_message failed!\n");
        return -1;
    }
    return 0;
}