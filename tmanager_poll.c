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

    if (t == -1 || txlog->transaction[t].tstate == TX_ABORTED) {
        if (t == -1)
            printf("Transaction %d does not exist, replying ABORT\n", tid);
        else
            printf("Transaction %d was Aborted, replying ABORT\n", tid);
        // Reply ABORT
        txMsgType reply;
        reply.msgID = ABORT_TX;
        reply.tid = tid;
        return send_message(sockfd, client, &reply);
    } else if (txlog->transaction[t].tstate == TX_COMMITTED) {
        printf("Transaction %d was Committed, replying COMMIT\n", tid);
        // Reply COMMIT
        txMsgType reply;
        reply.msgID = COMMIT_TX;
        reply.tid = tid;
        return send_message(sockfd, client, &reply);
    }
    // If voting or in progress, ignore
    return 0;
}