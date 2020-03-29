//
// Created by Cody on 3/28/2020.
//

#include "tmanager.h"

int tm_poll(int sockfd, struct transactionSet * txlog, uint32_t tid, struct sockaddr_in client) {
    // Check valid request (TID exists)
    int t = -1;
    for (int i = 0; i < MAX_TX; i++) {
        if (txlog->transaction[i].tstate != TX_NOTINUSE && txlog->transaction[i].txID == tid)
            t = i;
    }

    if (t == -1) {
        printf("Transaction %d does not exist, replying failure\n", tid);
        // Reply Failure
        txMsgType reply;
        reply.msgID = FAILURE_TX;
        reply.tid = tid;
        return send_message(sockfd, client, &reply);
    }

    // Reply Success with state
    txMsgType reply;
    reply.msgID = SUCCESS_TX;
    reply.tid = tid;
    reply.state = txlog->transaction[t].tstate;
    return send_message(sockfd, client, &reply);
}