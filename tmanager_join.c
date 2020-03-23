//
// Created by Cody on 3/23/2020.
//
// Handler code for Join
//

int join(struct transactionSet * txlog, struct txMsgType message, struct sockaddr_in client) {
    // Check valid request (TID exists)
    int t = -1;
    for (int i = 0; i < MAX_TX; i++) {
        if (txlog->transaction[i].tstate != TX_NOTINUSE && txlog->transaction[i].txID == message.tid)
            t = i;
    }

    if (t == -1) {
        // Reply Failure
        struct txMsgType reply;
        reply.msgId = FAILURE_TX;
        reply.tid = message.tid;
        return send_message(sockfd, client, &reply);
    }

    // Check for space or already existing
    txlog->transaction[t].txID = message.tid;
    int w = -1;
    for (int i = 0; i < MAX_WORKERS; i++) {
        if (txlog->transaction[t].worker[i].sin_port == 0) { // Treat port 0 default/unassigned
            w = i;
        }
        if (txlog->transaction[t].worker[i].sin_port == client.sin_port) {
            w = i; // Counting already being joined as success
            break;
        }
    }
    if (w == -1) {
        // Reply Failure
        struct txMsgType reply;
        reply.msgId = FAILURE_TX;
        reply.tid = message.tid;
        return send_message(sockfd, client, &reply);
    }
    // Add worker to transaction
    txlog->transaction[t].worker[w] = client;

    // Reply Success
    struct txMsgType reply;
    reply.msgId = SUCCESS_TX;
    reply.tid = message.tid;
    return send_message(sockfd, client, &reply)

    // TODO Logging
}
