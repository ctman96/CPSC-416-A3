//
// Created by Cody on 3/23/2020.
//
// Handler code for committing / voting
//
#include "tmanager.h"

int tm_commit(int sockfd, struct transactionSet * txlog, uint32_t tid, struct sockaddr_in client, int crash) {
    // Check valid request (TID exists)
    int t = -1;
    for (int i = 0; i < MAX_TX; i++) {
        if (txlog->transaction[i].tstate != TX_NOTINUSE && txlog->transaction[i].txID == tid)
            t = i;
    }

    if (t == -1 || txlog->transaction[t].tstate != TX_INPROGRESS) {
        if (t == -1)
            printf("Transaction %d does not exist, replying failure\n", tid);
        else
            printf("Transaction %d is not INPROGRESS, replying failure\n", tid);
        // Reply Failure?
        txMsgType reply;
        reply.msgID = FAILURE_TX;
        reply.tid = tid;
        return send_message(sockfd, client, &reply);
    }

    // Initialize vote
    printf("Beginning voting for Transaction %d\n", tid);
    txlog->transaction[t].tstate = TX_VOTING;
    txlog->transaction[t].voteTime = time(NULL);
    txlog->transaction[t].crash = crash;
    if (msync(txlog, sizeof(struct transactionSet), MS_SYNC | MS_INVALIDATE)) {
        perror("Msync problem");
    }

    // Send prepareToCommit to all workers
    for (int i = 0; i < MAX_WORKERS; i++) {
        txlog->transaction[t].voteState[i] = 0;
        if (txlog->transaction[t].worker[i].sin_port != 0) {
            txMsgType ptc;
            ptc.msgID = PREPARE_TX;
            ptc.tid = tid;
            if (send_message(sockfd, txlog->transaction[t].worker[i], &ptc) < 0) {
                return -1;
            }
        }
    }

    return 0;
}


int tm_prepared(int sockfd, struct transactionSet * txlog, uint32_t tid, struct sockaddr_in client) {
    // Check valid request (TID exists)
    int t = -1;
    for (int i = 0; i < MAX_TX; i++) {
        if (txlog->transaction[i].tstate != TX_NOTINUSE && txlog->transaction[i].txID == tid)
            t = i;
    }

    if (t == -1 || txlog->transaction[t].tstate != TX_VOTING) {
        if (t == -1)
            printf("Transaction %d does not exist, replying failure\n", tid);
        else
            printf("Transaction %d is not VOTING, replying failure\n", tid);
        // Reply Failure?
        txMsgType reply;
        reply.msgID = FAILURE_TX;
        reply.tid = tid;
        return send_message(sockfd, client, &reply);
    }

    // Set worker vote state as prepared
    printf("Worker %d has voted prepared\n", client.sin_port);
    for (int i = 0; i < MAX_WORKERS; i++) {
        if (txlog->transaction[t].worker[i].sin_port == client.sin_port){
            txlog->transaction[t].voteState[i] = 1;
            break;
        }
        // This will I guess accept votes from any worker, not just those assigned, but those votes won't actually do anything
        // Maybe TODO: reply failure if worker isn't part of the transaction
    }

    // Stop here if there are still unprepared workers
    for (int i = 0; i < MAX_WORKERS; i++) {
        if (txlog->transaction[t].worker[i].sin_port != 0) {
            if (txlog->transaction[t].voteState[i] != 1)
                return 0;
        }
    }

    printf("Transaction %d has completed voting, Committing\n", tid);

    // Commit
    txlog->transaction[t].tstate = TX_COMMITTED;
    if (msync(txlog, sizeof(struct transactionSet), MS_SYNC | MS_INVALIDATE)) {
        perror("Msync problem");
    }

    // Crash if was told to
    if (txlog->transaction[t].crash) {
        printf("Transaction %d - Crashing\n", tid);
        _exit(EXIT_SUCCESS);
    }

    // Send Committed to all workers
    for (int i = 0; i < MAX_WORKERS; i++) {
        txlog->transaction[t].voteState[i] = 0;
        if (txlog->transaction[t].worker[i].sin_port != 0) {
            txMsgType cmt;
            cmt.msgID = COMMIT_TX;
            cmt.tid = tid;
            if (send_message(sockfd, txlog->transaction[t].worker[i], &cmt) < 0) {
                return -1;
            }
        }
    }

    return 0;
}


int tm_abort(int sockfd, struct transactionSet * txlog, uint32_t tid, struct sockaddr_in client, int crash) {
    // Check valid request (TID exists)
    int t = -1;
    for (int i = 0; i < MAX_TX; i++) {
        if (txlog->transaction[i].tstate != TX_NOTINUSE && txlog->transaction[i].txID == tid)
            t = i;
    }

    if (t == -1 || txlog->transaction[t].tstate != TX_VOTING || txlog->transaction[t].tstate != TX_INPROGRESS) {
        if (t == -1)
            printf("Transaction %d does not exist, replying failure\n", tid);
        else
            printf("Transaction %d is not VOTING or INPROGRESS, replying failure\n", tid);
        // Reply Failure?
        txMsgType reply;
        reply.msgID = FAILURE_TX;
        reply.tid = tid;
        return send_message(sockfd, client, &reply);
    }

    return tm_abort_inner(sockfd, txlog, tid, t, crash);
}

int tm_abort_inner(int sockfd, struct transactionSet * txlog, uint32_t tid, int t, int crash) {
    printf("Aborting Transaction %d\n", tid);

    // Abort
    txlog->transaction[t].tstate = TX_ABORTED;
    if (msync(txlog, sizeof(struct transactionSet), MS_SYNC | MS_INVALIDATE)) {
        perror("Msync problem");
    }

    // Crash
    if (crash) {
        printf("Transaction %d - Crashing\n", tid);
        _exit(EXIT_SUCCESS);
    }

    // Send Abort to all workers
    for (int i = 0; i < MAX_WORKERS; i++) {
        txlog->transaction[t].voteState[i] = 0;
        if (txlog->transaction[t].worker[i].sin_port != 0) {
            txMsgType abt;
            abt.msgID = ABORT_TX;
            abt.tid = tid;
            if (send_message(sockfd, txlog->transaction[t].worker[i], &abt) < 0) {
                return -1;
            }
        }
    }

    return 0;
}