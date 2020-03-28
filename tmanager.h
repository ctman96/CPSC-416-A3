
#ifndef TMANAGER_h
#define TMANGER_h
#define MAX_WORKERS 6
#define MAX_TX 4

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

enum txState {
  TX_NOTINUSE = 100,
  TX_INPROGRESS,
  TX_VOTING,
  TX_ABORTED,
  TX_COMMITTED
};

struct tx {
  unsigned long txID;
  enum txState tstate;
  struct  sockaddr_in worker[MAX_WORKERS];
  int voteTime; // Time vote started
  int crash; // Whether to crash
  int voteState[MAX_WORKERS]; // 0 default, 1 if prepared
};


struct transactionSet{
  int initialized;
  struct tx transaction[MAX_TX];
};

int send_message(int sockfd, struct sockaddr_in client, txMsgType* message);

int tm_begin(int sockfd, struct transactionSet * txlog, uint32_t tid, struct sockaddr_in client);
int tm_join(int sockfd, struct transactionSet * txlog, uint32_t tid, struct sockaddr_in client);
int tm_commit(int sockfd, struct transactionSet * txlog, uint32_t tid, struct sockaddr_in client, int crash);
int tm_prepared(int sockfd, struct transactionSet * txlog, uint32_t tid, struct sockaddr_in client);
int tm_abort(int sockfd, struct transactionSet * txlog, uint32_t tid, struct sockaddr_in client, int crash);

#endif
