//
// Created by Cody on 3/23/2020.
//
// Handler code for Join
//
#include "tmanager.h"

int tm_join(int sockfd, struct transactionSet * txlog, uint32_t tid, struct sockaddr_in client) {
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

    // Check for space or already existing
    int w = -1;
    for (int i = 0; i < MAX_WORKERS; i++) {
        if (txlog->transaction[t].worker[i].sin_port == 0) { // Treat port 0 default/unassigned
            w = i;
        }
        if (txlog->transaction[t].worker[i].sin_port == client.sin_port) {
            printf("Transaction %d already contains worker %d\n", tid, client.sin_port);
            w = i; // Counting already being joined as success
            break;
        }
    }
    if (w == -1) {
        printf("Transaction %d has max workers, replying failure\n", tid);
        // Reply Failure
        txMsgType reply;
        reply.msgID = FAILURE_TX;
        reply.tid = tid;
        return send_message(sockfd, client, &reply);
    }
    // Add worker to transaction
    printf("Adding Worker %d to Transaction %d\n", client.sin_port, tid);
    txlog->transaction[t].worker[w] = client;
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
