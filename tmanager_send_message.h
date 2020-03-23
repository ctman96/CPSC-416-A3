//
// Created by Cody on 3/23/2020.
//

#ifndef A3_TMANAGER_SEND_MESSAGE_H
#define A3_TMANAGER_SEND_MESSAGE_H

#include <sys/socket.h>
#include "transaction_msg.h"

int send_message(int sockfd, struct sockaddr_in client, struct txMsgType* message);

#endif //A3_TMANAGER_SEND_MESSAGE_H
