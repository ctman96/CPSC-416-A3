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
#include <signal.h>
#include "msg.h"

void usage(char * cmd) {
    printf("usage: %s  managerPort worker1CmdPort worker1TxPort worker2CmdPort worker2TxPort testPort\n",
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
    char* tm_port_str = argv[1];
    unsigned long  tw1_cmd_port;
    char* tw1_cmd_port_str = argv[2];
    unsigned long  tw1_tx_port;
    char* tw1_tx_port_str = argv[3];
    unsigned long  tw2_cmd_port;
    char* tw2_cmd_port_str = argv[4];
    unsigned long  tw2_tx_port;
    char* tw2_tx_port_str = argv[5];
    unsigned long  port;

    if (argc != 7) {
        usage(argv[0]);
        return -1;
    }
    char * end;

    tm_port = strtoul(argv[1], &end, 10);
    if (argv[1] == end) {
        printf("Port conversion error\n");
        exit(-1);
    }

    tw1_cmd_port = strtoul(argv[2], &end, 10);
    if (argv[2] == end) {
        printf("Port conversion error\n");
        exit(-1);
    }

    tw1_tx_port = strtoul(argv[3], &end, 10);
    if (argv[3] == end) {
        printf("Port conversion error\n");
        exit(-1);
    }

    tw2_cmd_port = strtoul(argv[4], &end, 10);
    if (argv[4] == end) {
        printf("Port conversion error\n");
        exit(-1);
    }

    tw2_tx_port = strtoul(argv[5], &end, 10);
    if (argv[5] == end) {
        printf("Port conversion error\n");
        exit(-1);
    }

    port = strtoul(argv[6], &end, 10);
    if (argv[6] == end) {
        printf("Port conversion error\n");
        exit(-1);
    }


    char  logFileName[128];

    /* got the port number create a logfile name */
    snprintf(logFileName, sizeof(logFileName), "TXworker_%u.log", tw1_cmd_port);

    // Remove existing log
    if (remove(logFileName) != 0) {
        perror("Unable to remove worker1 log: ");
    };

    // Start process
    pid_t tw1_pid = fork();
    if (tw1_pid == 0) {
        // Child process
        int fd = open("test-tworker1.log", O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        dup2(fd, 1);   // make stdout go to file
        close(fd);
        fprintf(stderr, "%s %s\n", tw1_cmd_port_str, tw1_tx_port_str);
        char* args[]={"./tworker", tw1_cmd_port_str, tw1_tx_port_str, NULL};
        execvp(args[0], args);
        exit(0);
    }
    sleep(1);

    int logfileFD1;

    logfileFD1 = open(logFileName, O_RDONLY , S_IRUSR | S_IWUSR );
    if (logfileFD1 < 0 ) {
        char msg[256];
        snprintf(msg, sizeof(msg), "Opening %s failed \n", logFileName);
        perror(msg);
        exit(-1);
    }

    struct logFile  *log1 = mmap(NULL, 512, PROT_READ, MAP_SHARED, logfileFD1, 0);
    if (log1 == NULL) {
        perror("Log file could not be mapped in:");
        exit(-1);
    }

    if (msync(log1, sizeof(struct logFile), MS_SYNC)) {
        perror("Msync problem\n");
    }


    /* got the port number create a logfile name */
    snprintf(logFileName, sizeof(logFileName), "TXworker_%u.log", tw2_cmd_port);

    // Remove existing log
    if (remove(logFileName) != 0) {
        perror("Unable to remove worker1 log: ");
    };

    // Start process
    pid_t tw2_pid = fork();
    if (tw2_pid == 0) {
        // Child process
        int fd = open("test-tworker2.log", O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        dup2(fd, 1);   // make stdout go to file
        close(fd);
        fprintf(stderr, "%s %s\n", tw2_cmd_port_str, tw2_tx_port_str);
        char* args[]={"./tworker", tw2_cmd_port_str, tw2_tx_port_str,  NULL};
        execvp(args[0], args);
        exit(0);
    }
    sleep(1);

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

    // Manager
    snprintf(logFileName, sizeof(logFileName), "TXMG_%u.log", tm_port);

    // Remove existing log
    remove(logFileName);

    // Start process
    pid_t tm_pid = fork();
    if (tm_pid == 0) {
        // Child process
        int fd = open("test-tmanager.log", O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        dup2(fd, 1);   // make stdout go to file
        close(fd);
        fprintf(stderr, "%s\n", tm_port_str);
        char* args[]={"./tmanager", tm_port_str, NULL};
        execvp(args[0], args);
        exit(0);
    }
    sleep(1);


    char* test_name;
    char cmd[1024];

    if (log1->initialized || log2->initialized) {
        printf("Delete logs before running!\n");
        exit(-1);
    }

    // =================================
    test_name = "A properly completed transaction";
    // =================================
    {
        printf("=== %s ===\n", test_name);

        // Worker 1 Begins
        unsigned long test_tid = 1001001;
        snprintf(cmd, sizeof(cmd), "./cmd begin localhost %d localhost %d %d", tw1_cmd_port, tm_port, test_tid);
        system(cmd);

        sleep(1);

        if (msync(log1, sizeof(struct logFile), MS_SYNC)) {
            perror("Msync problem");
        }

        if (!(log1->log.txState == WTX_ACTIVE && log1->log.txID == test_tid)) {
            printf("%s = FAILED\n", test_name);
            exit(-1);
        } else {
            printf("%s - Worker 1 Began\n", test_name);
        }

        // Worker 2 Joins
        snprintf(cmd, sizeof(cmd), "./cmd join localhost %d localhost %d %d", tw2_cmd_port, tm_port, test_tid);
        system(cmd);

        sleep(1);

        if (msync(log2, sizeof(struct logFile), MS_SYNC)) {
            perror("Msync problem");
        }

        if (!(log2->log.txState == WTX_ACTIVE && log2->log.txID == test_tid)) {
            printf("%s = FAILED\n", test_name);
            exit(-1);
        } else {
            printf("%s - Worker 2 Joined\n", test_name);
        }

        // Worker 1 commits
        snprintf(cmd, sizeof(cmd), "./cmd commit localhost %d", tw1_cmd_port);
        system(cmd);

        sleep(2);

        if (msync(log1, sizeof(struct logFile), MS_SYNC)) {
            perror("Msync problem");
        }
        if (msync(log2, sizeof(struct logFile), MS_SYNC)) {
            perror("Msync problem");
        }

        if (!(log1->log.txState == WTX_NOTACTIVE && log2->log.txID == test_tid)) {
            printf("%s = FAILED\n", test_name);
            exit(-1);
        } else {
            printf("%s - Worker 1 Completed\n", test_name);
        }
        if (!(log2->log.txState == WTX_NOTACTIVE && log2->log.txID == test_tid)) {
            printf("%s = FAILED\n", test_name);
            exit(-1);
        } else {
            printf("%s - Worker 2 Completed\n", test_name);
            printf("%s = PASSED\n", test_name);
        }
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


    kill(tw1_pid, SIGINT);
    kill(tw2_pid, SIGINT);
    kill(tm_pid, SIGINT);
}
