
#ifndef TWORKER_H
#define TWORKER_H 1
#include <time.h>
#include <stdbool.h>

#define MAX_NODES 10
#define IDLEN     64

// bitmasks
#define A_MASK 1
#define B_MASK 2
#define ID_MASK 4

// timeouts
#define TIMEOUT 10
#define UNCERTAIN_TIMEOUT 30

// Feel free to modify anything in this file except the
// struct transactionData

enum workerTxState {
  WTX_NOTACTIVE  = 400,
  WTX_ACTIVE,
  WTX_ABORTED,
  WTX_PREPARED,
  WTX_COMMITTED,
  WTX_UNCERTAIN
};


struct  transactionData {
  char IDstring[IDLEN];   // This must be a null terminated ASCII string
  int A;
  int B;
} ;

struct workerLog {
  unsigned long txID;
  enum workerTxState txState;
  struct sockaddr_in transactionManager;
  unsigned int oldSaved; // use this to record if
                         // the old values of A, B and IDstring have been
                         // captured. Associated a different bit
                         // with each variable;
  int oldA;
  int oldB;
  char oldIDstring[IDLEN];
  int newA;
  int newB;
  char newIDstring[IDLEN];
};


struct logFile {
  int initialized;
  struct transactionData txData;
  struct workerLog log;
};

#endif /* TWORKER_H */
