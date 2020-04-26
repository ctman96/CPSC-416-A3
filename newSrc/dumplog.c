
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
  printf("usage: %s  <filename>  <filename> ...",
	 cmd);
}


void printTXinfo(struct logFile *log) {
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
    
    printf("          TX (%u, 0x%x) State: %s, (%d, 0x%x)\n",
	   txID, txID,
	   txString, txState, txState);
}




int main(int argc, char ** argv) {

  unsigned long cmdPort;
  unsigned long txPort;
  // This is some sample code feel free to delete it

  if (argc == 1) {
    usage(argv[0]);
    return -1;
  }

  int i;
  for (i = 1; i < argc; i++) {

    /* got the port number create a logfile name */
    printf(">> TX file: %s - ", argv[i]);
    int fd;
    
    fd = open(argv[i], O_RDONLY);
    if ( fd < 0 ) {
      char msg[256];
      printf("Open failed.\n");
      perror("Reason: ");
      break;
    }

    //printf("Opened.\n");
    // check the logfile size
    struct stat fstatus;
    if (fstat( fd, &fstatus ) < 0) {
      perror("Filestat failed");
      break;
    }
    
    // Let's see if the logfile has some entries in it by checking the size
   
    if (fstatus.st_size < sizeof(struct logFile)) {
      printf("File too small - not properly initialized\n");
      break;
    }
    
   // Now map the file in.
   struct logFile  *log = mmap(NULL, 512, PROT_READ, MAP_SHARED, fd, 0);
   if (log == NULL) {
     perror("Log file could not be mapped in:");
     break;
   }
   // Print out the data:
   printf("Int A: %-7d", log->txData.A);
   printf(" Int B: %-7d | ", log->txData.B);
   printf("ID String: %s\n", log->txData.IDstring);
   // printTXinfo(log);
   munmap(log, 512);
   close(fd);
  }
}
