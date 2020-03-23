//
// Created by Cody on 3/23/2020.
//
// Handler code for committing / voting
//
#include "tmanager.h"
#include "transaction_msg.h"
#include "tmanager_send_message.h"

int commit(int sockfd, struct transactionSet * txlog, struct txMsgType message, struct sockaddr_in client) {
    // Check valid request (TID exists)
    int t = -1;
    for (int i = 0; i < MAX_TX; i++) {
        if (txlog->transaction[i].tstate != TX_NOTINUSE && txlog->transaction[i].txID == message.tid)
            t = i;
    }

    if (t == -1 || txlog->transaction[i].tstate != TX_INPROGRESS) {
        // Reply Failure?
        struct txMsgType reply;
        reply.msgId = FAILURE_TX;
        reply.tid = message.tid;
        return send_message(sockfd, client, &reply);
    }

    // Initialize vote
    txlog->transaction[t].tstate = TX_VOTING;
    txlog->transaction[t].voteTime = time(NULL);

    // Send prepareToCommit to all workers
    for (int i = 0; i < MAX_WORKERS; i++) {
        txlog->transaction[t].voteState[i] = 0;
        if (txlog->transaction[t].worker[i].sin_port != 0) {
            struct txMsgType ptc;
            ptc.msgId = PREPARE_TX;
            ptc.tid = message.tid;
            if (send_message(sockfd, client, &ptc) < 0) {
                // TODO Error?
                return -1;
            }
        }
    }

    return 0;
}

int commit_crash(int sockfd, struct transactionSet * txlog, struct txMsgType message, struct sockaddr_in client) {
    // TODO
}


int prepared(int sockfd, struct transactionSet * txlog, struct txMsgType message, struct sockaddr_in client) {
    // Check valid request (TID exists)
    int t = -1;
    for (int i = 0; i < MAX_TX; i++) {
        if (txlog->transaction[i].tstate != TX_NOTINUSE && txlog->transaction[i].txID == message.tid)
            t = i;
    }

    if (t == -1 || txlog->transaction[i].tstate != TX_VOTING) {
        // Reply Failure?
        struct txMsgType reply;
        reply.msgId = FAILURE_TX;
        reply.tid = message.tid;
        return send_message(sockfd, client, &reply);
    }

    // Set worker vote state as prepared
    for (int i = 0; i < MAX_WORKERS; i++) {
        if (txlog->transaction[t].worker[i].sin_port == client.sin_port){
            txlog->transaction[t].voteState[i] = 1;
            break;
        }
    }

    // Stop here if there are still unprepared workers
    for (int i = 0; i < MAX_WORKERS; i++) {
        if (txlog->transaction[t].worker[i].sin_port != 0){
            if (txlog->transaction[t].voteState[i] != 1)
                return 0;
        }
    }

    // Commit
    txlog->transaction[t].tstate = TX_COMMITTED;

    // Send Committed to all workers
    for (int i = 0; i < MAX_WORKERS; i++) {
        txlog->transaction[t].voteState[i] = 0;
        if (txlog->transaction[t].worker[i].sin_port != 0) {
            struct txMsgType cmt;
            cmt.msgId = COMMIT_TX;
            cmt.tid = message.tid;
            if (send_message(sockfd, client, &cmt) < 0) {
                // TODO Error?
                return -1;
            }
        }
    }

    return 0;
}


int abort(int sockfd, struct transactionSet * txlog, struct txMsgType message, struct sockaddr_in client) {
    // Check valid request (TID exists)
    int t = -1;
    for (int i = 0; i < MAX_TX; i++) {
        if (txlog->transaction[i].tstate != TX_NOTINUSE && txlog->transaction[i].txID == message.tid)
            t = i;
    }

    if (t == -1 || txlog->transaction[i].tstate != TX_VOTING) { // TODO what states should this be accepted?
        // Reply Failure?
        struct txMsgType reply;
        reply.msgId = FAILURE_TX;
        reply.tid = message.tid;
        return send_message(sockfd, client, &reply);
    }

    // Abort
    txlog->transaction[t].tstate = TX_ABORTED;

    // Send Abort to all workers
    for (int i = 0; i < MAX_WORKERS; i++) {
        txlog->transaction[t].voteState[i] = 0;
        if (txlog->transaction[t].worker[i].sin_port != 0) {
            struct txMsgType abt;
            abt.msgId = ABORT_TX;
            abt.tid = message.tid;
            if (send_message(sockfd, client, &abt) < 0) {
                // TODO Error?
                return -1;
            }
        }
    }

    return 0;
}

int abort_crash(int sockfd, struct transactionSet * txlog, struct txMsgType message, struct sockaddr_in client) {
    // TODO
}