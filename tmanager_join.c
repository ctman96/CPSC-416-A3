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
        // TODO reply failure
        return 0;
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
        // TODO reply failure
        return 0;
    }
    // Add worker to transaction
    txlog->transaction[t].worker[w] = client;

    // TODO Reply Success
    // TODO Logging

    return 0;
}
