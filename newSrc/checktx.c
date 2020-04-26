
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <stdio.h>
#include <errno.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <string.h>

#include "msg.h"
#include "tworker.h"


void usage(char * cmd) {
  printf("usage: %s  FileName ... \n",
	 cmd);
}


int main(int argc, char ** argv) {


  if (argc < 2) {
    usage(argv[0]);
    return -1;
  }

  int i;
  for (i = 1; i < argc; i++) {
    int logfileFD;
    char *logName = argv[i];
    
    logfileFD = open(logName, O_RDWR |O_SYNC);
    if (logfileFD < 0 ) {
      char msg[256];
      snprintf(msg, sizeof(msg), "Opening %s failed", logName);
      perror(msg);
      exit(-1);
    }

    // check the logfile size
    struct stat fstatus;
    if (fstat(logfileFD, &fstatus) < 0) {
      perror("Filestat failed");
      break;
    }
    
    // Now map the file in.
    struct logFile  *log = mmap(NULL, 512, PROT_READ | PROT_WRITE, MAP_SHARED, logfileFD, 0);
    if (log == NULL) {
      perror("Log file could not be mapped in:");
      close(logfileFD);
      break;
    }
    
    enum workerTxState txState;
    char *txString = NULL;
    unsigned long txID;
   
    txState = log->log.txState;
    txID = log->log.txID;

    switch (txState) {
    case 0:
      txString = "NOT SET";
    case WTX_NOTACTIVE :
      txString = "WTX_NOTACTIVE";
      break;
    case WTX_ABORTED :
      txString = "WTX_ABORTED";
      break;
    case WTX_PREPARED :
     txString = "WTX_PREPARED";
     break;
    case WTX_COMMITTED :
      txString = "WTX_COMMITTED";
      break;
    default:
      txString = "WTX_USER_SPECIFIED";
    }
    
    printf("%s TX (%u, 0x%x) State: %s, (%d, 0x%x)\n",
	   logName, txID, txID,
	   txString, txState, txState);
    munmap(log, 512);
    close(logfileFD);
  }
}
