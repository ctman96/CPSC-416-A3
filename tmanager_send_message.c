//
// Created by Cody on 3/23/2020.
//

#include "tmanager_send_message.h"

int send_message(int sockfd, struct sockaddr_in client, struct txMsgType* message) {
    // TODO logging?

    int bytesSent;
    bytesSent = sendto(sockfd, (void *) message, sizeof(*message), (struct sockaddr *) &client, sizeof(client));
    if (bytesSent != sizeof(*message)) {
        // TODO Error
        return -1;
    }
    return 0;
}