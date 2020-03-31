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



struct properties {
    unsigned long  tm_port;
    char* tm_port_str;
    unsigned long  tw1_cmd_port;
    char* tw1_cmd_port_str;
    unsigned long  tw1_tx_port;
    char* tw1_tx_port_str;
    unsigned long  tw2_cmd_port;
    char* tw2_cmd_port_str;
    unsigned long  tw2_tx_port;
    char* tw2_tx_port_str;
    unsigned long  port;

    pid_t tw1_pid;
    pid_t tw2_pid;
    pid_t tm_pid;

    struct logFile  * tw1_log;
    struct logFile  * tw2_log;
    struct logFile  * tm_log;
};

void start_tw1(struct properties* properties) {
    char  logFileName[128];

    /* got the port number create a logfile name */
    snprintf(logFileName, sizeof(logFileName), "TXworker_%u.log", properties->tw1_cmd_port);

    // Remove existing log
    if (remove(logFileName) != 0) {
        perror("Unable to remove worker1 log: ");
    };

    // Start process
    properties->tw1_pid = fork();
    if (properties->tw1_pid == 0) {
        // Child process
        int fd = open("test-tworker1.log", O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        dup2(fd, 1);   // make stdout go to file
        close(fd);
        char* args[]={"./tworker", properties->tw1_cmd_port_str, properties->tw1_tx_port_str, NULL};
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

    properties->tw1_log = mmap(NULL, 512, PROT_READ, MAP_SHARED, logfileFD1, 0);
    if (properties->tw1_log == NULL) {
        perror("Log file could not be mapped in:");
        exit(-1);
    }

    if (msync(properties->tw1_log, sizeof(struct logFile), MS_SYNC)) {
        perror("Msync problem\n");
    }
}

void start_tw2(struct properties* properties) {
    char  logFileName[128];

    /* got the port number create a logfile name */
    snprintf(logFileName, sizeof(logFileName), "TXworker_%u.log", properties->tw2_cmd_port);

    // Remove existing log
    if (remove(logFileName) != 0) {
        perror("Unable to remove worker1 log: ");
    };

    // Start process
    properties->tw2_pid = fork();
    if (properties->tw2_pid == 0) {
        // Child process
        int fd = open("test-tworker2.log", O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        dup2(fd, 1);   // make stdout go to file
        close(fd);
        char* args[]={"./tworker", properties->tw2_cmd_port_str, properties->tw2_tx_port_str,  NULL};
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

    properties->tw2_log = mmap(NULL, 512, PROT_READ, MAP_SHARED, logfileFD2, 0);
    if (properties->tw2_log == NULL) {
        perror("Log file could not be mapped in:");
        exit(-1);
    }

    if (msync(properties->tw2_log, sizeof(struct logFile), MS_SYNC)) {
        perror("Msync problem");
    }
}

void start_tm(struct properties* properties) {
    char  logFileName[128];

    // Manager
    snprintf(logFileName, sizeof(logFileName), "TXMG_%u.log", properties->tm_port);

    // Remove existing log
    remove(logFileName);

    // Start process
    properties->tm_pid = fork();
    if (properties->tm_pid == 0) {
        // Child process
        int fd = open("test-tmanager.log", O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
        dup2(fd, 1);   // make stdout go to file
        close(fd);
        char* args[]={"./tmanager", properties->tm_port_str, NULL};
        execvp(args[0], args);
        exit(0);
    }
    sleep(1);

    int logfileFD3;

    logfileFD3 = open(logFileName, O_RDONLY , S_IRUSR | S_IWUSR );
    if (logfileFD3 < 0 ) {
        char msg[256];
        snprintf(msg, sizeof(msg), "Opening %s failed", logFileName);
        perror(msg);
        exit(-1);
    }

    properties->tm_log = mmap(NULL, 512, PROT_READ, MAP_SHARED, logfileFD3, 0);
    if (properties->tm_log == NULL) {
        perror("Log file could not be mapped in:");
        exit(-1);
    }

    if (msync(properties->tm_log, sizeof(struct logFile), MS_SYNC)) {
        perror("Msync problem");
    }
}

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
    struct properties properties;
    properties.tm_port_str = argv[1];
    properties.tw1_cmd_port_str = argv[2];
    properties.tw1_tx_port_str = argv[3];
    properties.tw2_cmd_port_str = argv[4];
    properties.tw2_tx_port_str = argv[5];

    if (argc != 7) {
        usage(argv[0]);
        return -1;
    }
    char * end;

    properties.tm_port = strtoul(argv[1], &end, 10);
    if (argv[1] == end) {
        printf("Port conversion error\n");
        exit(-1);
    }

    properties.tw1_cmd_port = strtoul(argv[2], &end, 10);
    if (argv[2] == end) {
        printf("Port conversion error\n");
        exit(-1);
    }

    properties.tw1_tx_port = strtoul(argv[3], &end, 10);
    if (argv[3] == end) {
        printf("Port conversion error\n");
        exit(-1);
    }

    properties.tw2_cmd_port = strtoul(argv[4], &end, 10);
    if (argv[4] == end) {
        printf("Port conversion error\n");
        exit(-1);
    }

    properties.tw2_tx_port = strtoul(argv[5], &end, 10);
    if (argv[5] == end) {
        printf("Port conversion error\n");
        exit(-1);
    }

    properties.port = strtoul(argv[6], &end, 10);
    if (argv[6] == end) {
        printf("Port conversion error\n");
        exit(-1);
    }

    start_tm(&properties);
    start_tw1(&properties);
    start_tw2(&properties);


    char* test_name;
    char cmd[1024];


    struct logFile  * tw1_log = properties.tw1_log;
    struct logFile  * tw2_log = properties.tw2_log;
    struct logFile  * tm_log = properties.tm_log;
    unsigned long tw1_cmd_port = properties.tw1_cmd_port;
    unsigned long tw2_cmd_port = properties.tw2_cmd_port;
    unsigned long tm_port = properties.tm_port;


    if (tw1_log->initialized || tw2_log->initialized) {
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

        if (msync(tw1_log, sizeof(struct logFile), MS_SYNC)) {
            perror("Msync problem");
        }

        if (!(tw1_log->log.txState == WTX_ACTIVE && tw1_log->log.txID == test_tid)) {
            printf("%s = FAILED\n", test_name);
            exit(-1);
        } else {
            printf("%s - Worker 1 Began\n", test_name);
        }

        // Worker 2 Joins
        snprintf(cmd, sizeof(cmd), "./cmd join localhost %d localhost %d %d", tw2_cmd_port, tm_port, test_tid);
        system(cmd);

        sleep(1);

        if (msync(tw2_log, sizeof(struct logFile), MS_SYNC)) {
            perror("Msync problem");
        }

        if (!(tw2_log->log.txState == WTX_ACTIVE && tw2_log->log.txID == test_tid)) {
            printf("%s = FAILED\n", test_name);
            exit(-1);
        } else {
            printf("%s - Worker 2 Joined\n", test_name);
        }

        // Worker 1 commits
        snprintf(cmd, sizeof(cmd), "./cmd commit localhost %d", tw1_cmd_port);
        system(cmd);

        sleep(2);

        if (msync(tw1_log, sizeof(struct logFile), MS_SYNC)) {
            perror("Msync problem");
        }
        if (msync(tw2_log, sizeof(struct logFile), MS_SYNC)) {
            perror("Msync problem");
        }

        if (!(tw1_log->log.txState == WTX_NOTACTIVE && tw2_log->log.txID == test_tid)) {
            printf("%s = FAILED\n", test_name);
            exit(-1);
        } else {
            printf("%s - Worker 1 Completed\n", test_name);
        }
        if (!(tw2_log->log.txState == WTX_NOTACTIVE && tw2_log->log.txID == test_tid)) {
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

        printf("=== %s ===\n", test_name);

        if (msync(tw1_log, sizeof(struct logFile), MS_SYNC)) {
            perror("Msync problem");
        }
        if (tw1_log->log.txState != WTX_NOTACTIVE) {
            printf("%d\n", tw1_log->log.txState);
            printf("%s = FAILED\n", test_name);
            exit(-1);
        }
        printf("%d\n", tw1_log->txData.A);

        // Set A initial value
        snprintf(cmd, sizeof(cmd), "./cmd newa localhost %d %d", tw1_cmd_port, tm_port, 1337);
        system(cmd);
        sleep(1);
        if (msync(tw1_log, sizeof(struct logFile), MS_SYNC)) {
            perror("Msync problem");
        }
        printf("%d\n", tw1_log->txData.A);
        if (tw1_log->txData.A == 1337) {
            printf("%s - Worker 1 Set A\n", test_name);
        } else {
            printf("%s = FAILED\n", test_name);
            exit(-1);
        }

        // Worker 1 Begins
        unsigned long test_tid = 1001002;
        snprintf(cmd, sizeof(cmd), "./cmd begin localhost %d localhost %d %d", tw1_cmd_port, tm_port, test_tid);
        system(cmd);

        sleep(1);

        if (msync(tw1_log, sizeof(struct logFile), MS_SYNC)) {
            perror("Msync problem");
        }

        if (!(tw1_log->log.txState == WTX_ACTIVE && tw1_log->log.txID == test_tid)) {
            printf("%s = FAILED\n", test_name);
            exit(-1);
        } else {
            printf("%s - Worker 1 Began\n", test_name);
        }

        // Set A new value
        snprintf(cmd, sizeof(cmd), "./cmd newa localhost %d %d", tw1_cmd_port, tm_port, 69420);
        system(cmd);
        sleep(1);
        if (msync(tw1_log, sizeof(struct logFile), MS_SYNC)) {
            perror("Msync problem");
        }
        if (tw1_log->log.newA == 69420) {
            printf("%s - Worker 1 Set A\n", test_name);
        } else {
            printf("%s = FAILED\n", test_name);
            exit(-1);
        }

        // Kill Manager
        kill(properties.tm_pid, SIGINT);
        printf("%s - Killed Manager\n", test_name);

        // Worker 1 commits
        snprintf(cmd, sizeof(cmd), "./cmd commit localhost %d", tw1_cmd_port);
        system(cmd);

        printf("%s - Waiting 31s for commit to abort...\n", test_name);
        sleep(31);

        if (msync(tw1_log, sizeof(struct logFile), MS_SYNC)) {
            perror("Msync problem");
        }

        if (!(tw1_log->log.txState == WTX_NOTACTIVE && tw2_log->log.txID == test_tid)) {
            printf("%s = FAILED\n", test_name);
            exit(-1);
        } else {
            printf("%s - Worker 1 Completed\n", test_name);
        }
        if (tw1_log->txData.A == 1337) {
            printf("%s - A reverted to oldA\n", test_name);
            printf("%s = PASSED\n", test_name);
        } else {
            printf("%s = FAILED\n", test_name);
            exit(-1);
        }
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


    kill(properties.tw1_pid, SIGINT);
    kill(properties.tw2_pid, SIGINT);
    kill(properties.tm_pid, SIGINT);
}
