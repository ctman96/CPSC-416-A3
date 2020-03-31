//
// Created by Cody on 3/30/2020.
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
#include "msg.h"

void usage(char * cmd) {
    printf("usage: %s  portNum\n",
           cmd);
}

int send_message(int sockfd, struct addrinfo* tmanager, msgType* message) {
    // printf("send_message: message: (msgId: %s, tid: %d)\n", txMsgKindToStr(message->msgID), message->tid);
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
    unsigned long  tw1_port;
    unsigned long  tw2_port;
    unsigned long  port;

    if (argc != 5) {
        usage(argv[0]);
        return -1;
    }
    char * end;

    tm_port = strtoul(argv[1], &end, 10);
    if (argv[1] == end) {
        printf("Port conversion error\n");
        exit(-1);
    }

    tw1_port = strtoul(argv[2], &end, 10);
    if (argv[2] == end) {
        printf("Port conversion error\n");
        exit(-1);
    }

    tw2_port = strtoul(argv[3], &end, 10);
    if (argv[3] == end) {
        printf("Port conversion error\n");
        exit(-1);
    }

    port = strtoul(argv[4], &end, 10);
    if (argv[4] == end) {
        printf("Port conversion error\n");
        exit(-1);
    }

    char  logFileName[128];

    /* got the port number create a logfile name */
    snprintf(logFileName, sizeof(logFileName), "TXworker_%u.log", tw1_port);

    int logfileFD1;

    logfileFD1 = open(logFileName, O_RDONLY , S_IRUSR | S_IWUSR );
    if (logfileFD1 < 0 ) {
        char msg[256];
        snprintf(msg, sizeof(msg), "Opening %s failed", logFileName);
        perror(msg);
        exit(-1);
    }

    struct logFile  *log1 = mmap(NULL, 512, PROT_READ, MAP_SHARED, logfileFD1, 0);
    if (log1 == NULL) {
        perror("Log file could not be mapped in:");
        exit(-1);
    }

    if (msync(log1, sizeof(struct logFile), MS_SYNC)) {
        perror("Msync problem");
    }

    /* got the port number create a logfile name */
    snprintf(logFileName, sizeof(logFileName), "TXworker_%u.log", tw2_port);

    int logfileFD2;

    logfileFD2 = open(logFileName, O_RDONLY , S_IRUSR | S_IWUSR );
    if (logfileFD2 < 0 ) {
        char msg[256];
        snprintf(msg, sizeof(msg), "Opening %s failed", logFileName);
        perror(msg);
        exit(-1);
    }

    struct logFile  *log2 = mmap(NULL, 512, PROT_READ, MAP_SHARED, logfileFD2, 0);
    if (log2 == NULL) {
        perror("Log file could not be mapped in:");
        exit(-1);
    }

    if (msync(log2, sizeof(struct logFile), MS_SYNC)) {
        perror("Msync problem");
    }


    char* test_name;
    char cmd[1024];


    // =================================
    test_name = "Begin";
    // =================================
    {
        unsigned long test_tid = 1001001;
        snprintf(cmd, sizeof(cmd), "./cmd begin localhost %d localhost %d %d", tw1_port, tm_port, test_tid);
        system(cmd);

        if (msync(log1, sizeof(struct logFile), MS_SYNC)) {
            perror("Msync problem");
        }

        if (!(log1->log.txState == WTX_ACTIVE && log1->log.txID == test_tid)) {
            printf("%s = Failed\n", test_name);
            exit(-1);
        } else {
            printf("%s = Passed\n", test_name);
        }
    }



    // =================================
    test_name = "A properly completed transaction";
    // =================================
    {

    }



    // =================================
    test_name = "Coordinator crashes before a request to commit - TX should abort";
    // =================================
    {



    }



    // =================================
    test_name = "Coordinator logs the decision to commit and then crashes";
    // =================================
    {



    }



    // =================================
    test_name = "Worker asks to commit while coordinator is crashed - TX should abort";
    // =================================
    {



    }



    // =================================
    test_name = "Worker crashes and recovers before TX commits, TX should abort.";
    // =================================
    {



    }



    // =================================
    test_name = "Worker crashes after recording prepared, but before sending its response TX should abort.";
    // =================================
    {



    }


}
