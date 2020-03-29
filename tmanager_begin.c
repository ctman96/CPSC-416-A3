//
// Created by Cody on 3/23/2020.
//
// Handler code for Begin
//
#include "tmanager.h"

int tm_begin(int sockfd, struct transactionSet * txlog, uint32_t tid, struct sockaddr_in client) {
    // Check valid request (TID doesn't conflict and can handle it)
    int t = -1;
    int id_conflict = 0;
    for (int i = 0; i < MAX_TX; i++) {
        if (txlog->transaction[i].tstate == TX_NOTINUSE ||
            txlog->transaction[i].tstate == TX_ABORTED ||
            txlog->transaction[i].tstate == TX_COMMITTED)
            t = i;
        else if (txlog->transaction[i].tstate != TX_NOTINUSE && txlog->transaction[i].txID == tid)
            id_conflict = 1;
    }

    if (t == -1 || id_conflict) {
        if (t == -1)
            printf("Already handling max transactions, replying failure\n");
        else
            printf("Transaction %d already exists, replying failure\n", tid);
        // Reply Failure
        txMsgType reply;
        reply.msgID = FAILURE_TX;
        reply.tid = tid;
        return send_message(sockfd, client, &reply);
    }

    // Begin transaction
    printf("Beginning transaction %d\n", tid);
    txlog->transaction[t].txID = tid;
    for (int i = 0; i < MAX_WORKERS; i++) {
        txlog->transaction[t].worker[i].sin_port = 0; // Treat port 0 default/unassigned
    }
    txlog->transaction[t].worker[0] = client;
    txlog->transaction[t].tstate = TX_INPROGRESS;
    txlog->transaction[t].crash = 0;
    if (msync(txlog, sizeof(struct transactionSet), MS_SYNC | MS_INVALIDATE)) {
        perror("Msync problem");
    }

    // Reply Success
    txMsgType reply;
    reply.msgID = SUCCESS_TX;
    reply.tid = tid;
    return send_message(sockfd, client, &reply);

    // TODO Logging
}
