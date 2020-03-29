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
    printf("send_message: message: (msgId: %d, tid: %d, state: %d)\n", message->msgID, message->tid, message->state);
    int bytesSent;
    bytesSent = sendto(sockfd, (void *) message, sizeof(*message), 0, tmanager->ai_addr, tmanager->ai_addrlen);
    if (bytesSent != sizeof(*message)) {
        perror("send_message failed!\n");
        return -1;
    }
    return 0;
}

int main(int argc, char ** argv) {
    unsigned long  tm_port;
    unsigned long  port = 8000;

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
    unsigned char buff[1024];
    txMsgType received_msg;
    socklen_t len;
    struct sockaddr_in client;
    memset(&client, 0, sizeof(client));

    // Test
    txMsgType test;
    test.msgID = BEGIN_TX;
    test.tid = 12345;

    if (send_message(sockfd, tmanagerAddr, &test) < 0) {
        return -1;
    }

    int size = recvfrom(sockfd, &received_msg, sizeof(received_msg), 0, (struct sockaddr *) &client, &len);

    printf("Received message (msgId: %d, tid: %d, state: %d\n", received_msg.msgID, received_msg.tid, received_msg.state);
}