//
// Created by Cody on 3/23/2020.
//

#include "tmanager.h"

int send_message(int sockfd, struct sockaddr_in client, txMsgType* message) {
    // TODO logging?

    int bytesSent;
    bytesSent = sendto(sockfd, (void *) message, sizeof(*message), 0, (struct sockaddr *) &client, sizeof(client));
    if (bytesSent != sizeof(*message)) {
        // TODO Error
        return -1;
    }
    return 0;
}