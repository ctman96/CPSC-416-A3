//
// Created by Cody on 3/29/2020.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <errno.h>
#include <time.h>
#include <netdb.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <strings.h>
#include <errno.h>
#include <sys/socket.h>
#include "transaction_msg.h"

void usage(char * cmd) {
    printf("usage: %s  portNum\n",
           cmd);
}

int send_message(int sockfd, struct addrinfo* tmanager, txMsgType* message) {
    printf("send_message: message: (msgId: %s, tid: %d)\n", txMsgKindToStr(message->msgID), message->tid);
    int bytesSent;
    bytesSent = sendto(sockfd, (void *) message, sizeof(*message), 0, tmanager->ai_addr, tmanager->ai_addrlen);
    if (bytesSent != sizeof(*message)) {
        perror("send_message failed!\n");
        return -1;
    }
    return 0;
}

int recv_message(int sockfd, txMsgType* received_message, struct sockaddr_in* client) {
    socklen_t len = sizeof(*client);
    memset(client, 0, sizeof(*client));
    int size = recvfrom(sockfd, received_message, sizeof(*received_message), 0, (struct sockaddr *) client, &len);
    if (size == -1) {
        perror("Error receiving");
        return -1;
    }
    printf("Received message (msgId: %s, tid: %d)\n", txMsgKindToStr(received_message->msgID), received_message->tid);
}

int main(int argc, char ** argv) {
    unsigned long  tm_port;
    unsigned long  port = 111171;
    unsigned long  port2 = 111172;

    if (argc != 2) {
        usage(argv[0]);
        return -1;
    }
    char * end;

    tm_port = strtoul(argv[1], &end, 10);
    if (argv[1] == end) {
        printf("Port conversion error\n");
        exit(-1);
    }



    // Create the socket
    int sockfd;
    struct sockaddr_in servAddr;

    if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(-1);
    }

    // Setup my server information
    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(port);
    // Accept on any of the machine's IP addresses.
    servAddr.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket to the requested addresses and port
    if ( bind(sockfd, (const struct sockaddr *)&servAddr,
              sizeof(servAddr)) < 0 )  {
        perror("bind failed");
        exit(-1);
    }


    // Create the second socket
    int sockfd2;
    struct sockaddr_in servAddr2;

    if ( (sockfd2 = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) {
        perror("socket creation failed");
        exit(-1);
    }

    // Setup my server information
    memset(&servAddr2, 0, sizeof(servAddr2));
    servAddr2.sin_family = AF_INET;
    servAddr2.sin_port = htons(port2);
    // Accept on any of the machine's IP addresses.
    servAddr2.sin_addr.s_addr = INADDR_ANY;

    // Bind the socket to the requested addresses and port
    if ( bind(sockfd2, (const struct sockaddr *)&servAddr2,
              sizeof(servAddr2)) < 0 )  {
        perror("bind failed");
        exit(-1);
    }



    // Setup tmanager address
    char *hostname = "localhost";
    struct addrinfo hints, *tmanagerAddr;
    tmanagerAddr = NULL;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_family = AF_INET;
    hints.ai_protocol = IPPROTO_UDP;
    if (getaddrinfo(hostname, argv[1], &hints, &tmanagerAddr)) {
        perror("Couldn't lookup hostname");
        return -1;
    }


    // Setup receiving
    txMsgType received_msg;
    struct sockaddr_in client;



    char* test_name;

    // =================================
    test_name = "Begin Success";
    // =================================
    txMsgType test;
    test.msgID = BEGIN_TX;
    test.tid = 12345;

    if (send_message(sockfd, tmanagerAddr, &test) < 0) return -1;
    if (recv_message(sockfd, &received_msg, &client) < 0) return -1;

    if (!(received_msg.msgID == SUCCESS_TX && received_msg.tid == test.tid)) {
        printf("%s = Failed\n", test_name);
        exit(-1);
    } else {
        printf("%s = Passed\n", test_name);
    }



    // =================================
    test_name = "Begin Failure (TID Conflict)";
    // =================================
    test.msgID = BEGIN_TX;
    test.tid = 12345;

    if (send_message(sockfd, tmanagerAddr, &test) < 0) return -1;
    if (recv_message(sockfd, &received_msg, &client) < 0) return -1;

    if (!(received_msg.msgID == FAILURE_TX && received_msg.tid == test.tid)) {
        printf("%s = Failed\n", test_name);
        exit(-1);
    } else {
        printf("%s = Passed\n", test_name);
    }


    // =================================
    test_name = "Join Success";
    // =================================
    test.msgID = JOIN_TX;
    test.tid = 12345;

    if (send_message(sockfd2, tmanagerAddr, &test) < 0) return -1;
    if (recv_message(sockfd2, &received_msg, &client) < 0) return -1;

    if (!(received_msg.msgID == SUCCESS_TX && received_msg.tid == test.tid)) {
        printf("%s - Failed\n", test_name);
        exit(-1);
    } else {
        printf("%s = Passed\n", test_name);
    }



    // =================================
    test_name = "Join Failure (Invalid TID)";
    // =================================
    test.msgID = JOIN_TX;
    test.tid = 11111;

    if (send_message(sockfd2, tmanagerAddr, &test) < 0) return -1;
    if (recv_message(sockfd2, &received_msg, &client) < 0) return -1;

    if (!(received_msg.msgID == FAILURE_TX && received_msg.tid == test.tid)) {
        printf("%s - Failed\n", test_name);
        exit(-1);
    } else {
        printf("%s = Passed\n", test_name);
    }



    // =================================
    test_name = "Commit Failure (Invalid TID)";
    // =================================
    test.msgID = COMMIT_TX;
    test.tid = 11111;

    if (send_message(sockfd2, tmanagerAddr, &test) < 0) return -1;
    if (recv_message(sockfd2, &received_msg, &client) < 0) return -1;

    if (!(received_msg.msgID == FAILURE_TX && received_msg.tid == test.tid)) {
        printf("%s - Failed\n", test_name);
        exit(-1);
    } else {
        printf("%s = Passed\n", test_name);
    }



    // =================================
    test_name = "Commit Success";
    // =================================
    test.msgID = COMMIT_TX;
    test.tid = 12345;

    if (send_message(sockfd2, tmanagerAddr, &test) < 0) return -1;

    if (recv_message(sockfd2, &received_msg, &client) < 0) return -1;
    if (!(received_msg.msgID == PREPARE_TX && received_msg.tid == test.tid)) {
        printf("%s - Failed\n", test_name);
        exit(-1);
    }

    if (recv_message(sockfd, &received_msg, &client) < 0) return -1;
    if (!(received_msg.msgID == PREPARE_TX && received_msg.tid == test.tid)) {
        printf("%s - Failed\n", test_name);
        exit(-1);
    } else {
        printf("%s = Passed\n", test_name);
    }



    // =================================
    test_name = "Commit Failure (Invalid State)";
    // =================================
    test.msgID = COMMIT_TX;
    test.tid = 12345;

    if (send_message(sockfd2, tmanagerAddr, &test) < 0) return -1;
    if (recv_message(sockfd2, &received_msg, &client) < 0) return -1;

    if (!(received_msg.msgID == FAILURE_TX && received_msg.tid == test.tid)) {
        printf("%s - Failed\n", test_name);
        exit(-1);
    } else {
        printf("%s = Passed\n", test_name);
    }



    // =================================
    test_name = "Prepare Failure (Invalid TID)";
    // =================================
    test.msgID = PREPARE_TX;
    test.tid = 111111;

    if (send_message(sockfd2, tmanagerAddr, &test) < 0) return -1;
    if (recv_message(sockfd2, &received_msg, &client) < 0) return -1;

    if (!(received_msg.msgID == FAILURE_TX && received_msg.tid == test.tid)) {
        printf("%s - Failed\n", test_name);
        exit(-1);
    } else {
        printf("%s = Passed\n", test_name);
    }



    // =================================
    test_name = "Prepare Success";
    // =================================
    test.msgID = PREPARE_TX;
    test.tid = 12345;

    // Both workers respond prepared
    if (send_message(sockfd, tmanagerAddr, &test) < 0) return -1;
    if (send_message(sockfd2, tmanagerAddr, &test) < 0) return -1;
    // Should now receive a commit
    if (recv_message(sockfd, &received_msg, &client) < 0) return -1;
    if (!(received_msg.msgID == COMMIT_TX && received_msg.tid == test.tid)) {
        printf("%s - Failed\n", test_name);
        exit(-1);
    }

    if (recv_message(sockfd2, &received_msg, &client) < 0) return -1;
    if (!(received_msg.msgID == COMMIT_TX && received_msg.tid == test.tid)) {
        printf("%s - Failed\n", test_name);
        exit(-1);
    } else {
        printf("%s = Passed\n", test_name);
    }



    // =================================
    test_name = "Poll State (Committed)";
    // =================================
    test.msgID = POLL_STATE_TX;
    test.tid = 12345;

    if (send_message(sockfd, tmanagerAddr, &test) < 0) return -1;
    if (recv_message(sockfd, &received_msg, &client) < 0) return -1;
    if (!(received_msg.msgID == COMMIT_TX && received_msg.tid == test.tid)) {
        printf("%s - Failed\n", test_name);
        exit(-1);
    } else {
        printf("%s = Passed\n", test_name);
    }



    // =================================
    test_name = "Abort Failure (Invalid TID)";
    // =================================
    test.msgID = ABORT_TX;
    test.tid = 111111;

    if (send_message(sockfd2, tmanagerAddr, &test) < 0) return -1;
    if (recv_message(sockfd2, &received_msg, &client) < 0) return -1;

    if (!(received_msg.msgID == FAILURE_TX && received_msg.tid == test.tid)) {
        printf("%s - Failed\n", test_name);
        exit(-1);
    } else {
        printf("%s = Passed\n", test_name);
    }


    // =================================
    test_name = "Abort Failure (Invalid State)";
    // =================================
    test.msgID = ABORT_TX;
    test.tid = 12345;

    if (send_message(sockfd2, tmanagerAddr, &test) < 0) return -1;
    if (recv_message(sockfd2, &received_msg, &client) < 0) return -1;

    if (!(received_msg.msgID == FAILURE_TX && received_msg.tid == test.tid)) {
        printf("%s - Failed\n", test_name);
        exit(-1);
    } else {
        printf("%s = Passed\n", test_name);
    }



    // =================================
    test_name = "Abort Success (In Progress)";
    // =================================
    test.msgID = BEGIN_TX;
    test.tid = 23456;

    if (send_message(sockfd2, tmanagerAddr, &test) < 0) return -1;
    if (recv_message(sockfd2, &received_msg, &client) < 0) return -1;

    if (!(received_msg.msgID == SUCCESS_TX && received_msg.tid == test.tid)) {
        printf("%s = Failed\n", test_name);
        exit(-1);
    }

    test.msgID = ABORT_TX;

    if (send_message(sockfd2, tmanagerAddr, &test) < 0) return -1;
    if (recv_message(sockfd2, &received_msg, &client) < 0) return -1;

    if (!(received_msg.msgID == ABORT_TX && received_msg.tid == test.tid)) {
        printf("%s - Failed\n", test_name);
        exit(-1);
    } else {
        printf("%s = Passed\n", test_name);
    }



    // =================================
    test_name = "Abort Success (Voting)";
    // =================================
    test.msgID = BEGIN_TX;
    test.tid = 23456;

    if (send_message(sockfd, tmanagerAddr, &test) < 0) return -1;
    if (recv_message(sockfd, &received_msg, &client) < 0) return -1;

    if (!(received_msg.msgID == SUCCESS_TX && received_msg.tid == test.tid)) {
        printf("%s = Failed\n", test_name);
        exit(-1);
    }

    test.msgID = COMMIT_TX;

    if (send_message(sockfd, tmanagerAddr, &test) < 0) return -1;

    if (recv_message(sockfd, &received_msg, &client) < 0) return -1;
    if (!(received_msg.msgID == PREPARE_TX && received_msg.tid == test.tid)) {
        printf("%s - Failed\n", test_name);
        exit(-1);
    }

    test.msgID = ABORT_TX;

    if (send_message(sockfd, tmanagerAddr, &test) < 0) return -1;
    if (recv_message(sockfd, &received_msg, &client) < 0) return -1;

    if (!(received_msg.msgID == ABORT_TX && received_msg.tid == test.tid)) {
        printf("%s - Failed\n", test_name);
        exit(-1);
    } else {
        printf("%s = Passed\n", test_name);
    }



    // =================================
    test_name = "Poll State (Aborted)";
    // =================================
    test.msgID = POLL_STATE_TX;

    if (send_message(sockfd, tmanagerAddr, &test) < 0) return -1;
    if (recv_message(sockfd, &received_msg, &client) < 0) return -1;
    if (!(received_msg.msgID == ABORT_TX && received_msg.tid == test.tid)) {
        printf("%s - Failed\n", test_name);
        exit(-1);
    } else {
        printf("%s = Passed\n", test_name);
    }



    // =================================
    test_name = "Poll State (Aborted non-existent)";
    // =================================
    test.msgID = POLL_STATE_TX;
    test.tid = 54321;

    if (send_message(sockfd, tmanagerAddr, &test) < 0) return -1;
    if (recv_message(sockfd, &received_msg, &client) < 0) return -1;
    if (!(received_msg.msgID == ABORT_TX && received_msg.tid == test.tid)) {
        printf("%s - Failed\n", test_name);
        exit(-1);
    } else {
        printf("%s = Passed\n", test_name);
    }

    // TODO how to test crashing?
    // Can I start and stop the manager from this prorgram?

}