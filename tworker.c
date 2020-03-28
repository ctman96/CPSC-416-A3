
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
  printf("usage: %s  cmdportNum txportNum\n",
	 cmd);
}


int main(int argc, char ** argv) {

  unsigned long cmdPort;
  unsigned long txPort;
  // This is some sample code feel free to delete it

  if (argc != 3) {
    usage(argv[0]);
    return -1;
  }

  // port to listen on for commands
   char * end;
   cmdPort = strtoul(argv[1], &end, 10);
   if (argv[1] == end) {
     printf("Command port conversion error\n");
     exit(-1);
   }

  // manager port
   txPort = strtoul(argv[2], &end, 10);
   if (argv[2] == end) {
     printf("Transaction port conversion error\n");
     exit(-1);
  }

   char  logFileName[128];

  /* got the port number create a logfile name */
   snprintf(logFileName, sizeof(logFileName), "TXworker_%u.log", cmdPort);

   int logfileFD;
   
   logfileFD = open(logFileName, O_RDWR | O_CREAT | O_SYNC, S_IRUSR | S_IWUSR );
   if (logfileFD < 0 ) {
     char msg[256];
     snprintf(msg, sizeof(msg), "Opening %s failed", logFileName);
     perror(msg);
     exit(-1);
   }

   // check the logfile size
   struct stat fstatus;
   if (fstat(logfileFD, &fstatus) < 0) {
     perror("Filestat failed");
     exit(-1);
   }

   // Let's see if the logfile has some entries in it by checking the size
   
   if (fstatus.st_size < sizeof(struct logFile)) {
     // Just write out a zeroed file struct
     printf("Initializing the log file size\n");
     struct logFile tx;
     bzero(&tx, sizeof(tx));
    if (write(logfileFD, &tx, sizeof(tx)) != sizeof(tx)) {
      printf("Writing problem to log\n");
      exit(-1);
    }
   }

   // Now map the file in.
   struct logFile  *log = mmap(NULL, 512, PROT_READ | PROT_WRITE, MAP_SHARED, logfileFD, 0);
   if (log == NULL) {
     perror("Log file could not be mapped in:");
     exit(-1);
   }

   // Some demo data
   strncpy(log->txData.IDstring, "Hi there!! :-)", IDLEN);
   log->txData.A = 10;
   log->txData.B = 100;
   log->log.oldA = 83;
   log->log.newA = 10;
   log->log.oldB = 100;
   log->log.newB = 1023;
   log->initialized = -1;
   if (msync(log, sizeof(struct logFile), MS_SYNC | MS_INVALIDATE)) {
     perror("Msync problem");
   }
   
    printf("Worker Start")
    printf("Command port:  %d\n", cmdPort);
    printf("TX port:       %d\n", txPort);
    printf("Log file name: %s\n", logFileName);
  // Some demo data
   strncpy(log->log.newIDstring, "1234567890123456789012345678901234567890", IDLEN);


  // Actual worker code
  // -------------------------------------------------------------

  // create two different sockets, one for cmd and one for txmanager
  int sockfdCmd;
  struct sockaddr_in servAddrCmd;

  int sockfdTx;
  struct sockaddr_in servAddrTx;

  if ( (sockfdCmd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
    perror("socket creation failed"); 
    exit(-1); 
  }

  if ( (sockfdTx = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
    perror("socket creation failed"); 
    exit(-1); 
  }

  // Set sockets as non-blocking
  int flags = fcntl(properties.sockfdCmd, F_GETFL);
  flags |= O_NONBLOCK;
  fcntl(properties.sockfdCmd, F_SETFL, flags);

  int flags = fcntl(properties.sockfdTx, F_GETFL);
  flags |= O_NONBLOCK;
  fcntl(properties.sockfdTx, F_SETFL, flags);

  // Setup my server information 
  memset(&servAddr, 0, sizeof(servAddr)); 
  servAddr.sin_family = AF_INET; 
  servAddr.sin_port = htons(port);
  // Accept on any of the machine's IP addresses.
  servAddr.sin_addr.s_addr = INADDR_ANY;

  // Bind the sockets to the requested addresses and ports
  if ( bind(sockfdCmd, (const struct sockaddr *)&servAddr,  
            sizeof(servAddr)) < 0 )  { 
    perror("bind failed"); 
    exit(-1); 
  }

  if ( bind(sockfdTx, (const struct sockaddr *)&servAddr,  
            sizeof(servAddr)) < 0 )  { 
    perror("bind failed"); 
    exit(-1); 
  }


  int running = 1;
  while (running) {
    // check for messages
    // switch on recieved message id
    // send msgs or edit data depending on msg
    // ---------------------------------------------------
    // TODO:
    // All of the switch statements
    // add section for txmanager socket
    // Questions:
    // Do we update the log->log.newA AND log->txDataA whenever we do newA? (same for B)

    // check for commands
    struct txMsgType message;
    message.msgId = 0;

    socklen_t len;
    struct sockaddr_in client;
    memset(&client, 0, sizeof(client));

    int size = recvfrom(sockfdCmd, &message, sizeof(message), 0, (struct sockaddr *) &client, &len);
    
    // should 'n' be size? where is n declared?
    if (n < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
      perror("Receiving error:");
      running = 0;
      abort();
    } else if (n >= 0) {
      printf("Got a command packet\n");
  
    switch (message.msgId) {

      case BEGINTX:
      // TODO
      // send a begin transaction message to manager (with txid TID). Read docs for errors
      struct txMsgType msg;
      msg.msgId = BEGIN_TX;
      msg.tid = message.TID
        break;
        
      case JOINTX:
      // TODO
      // send a join transaction msg to txmanager to join tx TID. 
        break;

      case NEW_A:
      // if a transaction not underway, update A. else, update log and then update data
      // if in not active state, then not in tx, so edit A or B directly
      // if in anything but not active state, then in transaction so log 

      if (log->log.txState == WTX_NOTACTIVE) {
        log->txData.A = message.newValue;
        if (msync(log, sizeof(struct logFile), MS_SYNC | MS_INVALIDATE)) {
          perror("Msync problem"); 
        }
      } else {

        // check if old A has already been saved, if not, save it and mark oldSaved
        if ((A_MASK & log->log.oldSaved) != A_MASK) {
          log->log.oldA = log->txData.A;
          log->log.oldSaved = log->log.oldSaved | A_MASK;
          printf("Saved old A\n");\
        }

        log->txData.A = message.newValue;
        log->log.newA = message.newValue;
      }
        break;

      case NEW_B:
      // if a transaction not underway, update B. else, update log and then update data

      if (log->log.txState == WTX_NOTACTIVE) {
        log->txData.B = message.newValue;
        if (msync(log, sizeof(struct logFile), MS_SYNC | MS_INVALIDATE)) {
          perror("Msync problem"); 
        }
      } else {

        // check if old A has already been saved, if not, save it and mark oldSaved
        if ((B_MASK & log->log.oldSaved) != B_MASK) {
          log->log.oldB = log->txData.B;
          log->log.oldSaved = log->log.oldSaved | B_MASK;
          printf("Saved old B\n");\
        }

        log->txData.B = message.newValue;
        log->log.newB = message.newValue;
      }
        break;

      case NEW_IDSTR:

      if (log->log.txState == WTX_NOTACTIVE) {
        log->txData.A = message.newValue;
        if (msync(log, sizeof(struct logFile), MS_SYNC | MS_INVALIDATE)) {
          perror("Msync problem"); 
        }
      } else {

        // check if old A has already been saved, if not, save it and mark oldSaved
        if ((ID_MASK & log->log.oldSaved) != ID_MASK) {
          log->log.oldIDstring = log->txData.IDstring;
          log->log.oldSaved = log->log.oldSaved | ID_MASK;
          printf("Saved old IDstring\n");\
        }

        log->txData.IDstring = message.newValue;
        log->log.newIDstring = message.newValue;
      }
        break;

      case DELAY_RESPONSE:
      // TODO
        break;
      case CRASH:
      // TODO
        break;
      case COMMIT:
      // TODO
        break;
      case COMMIT_CRASH:
      // TODO
        break;
      case ABORT:
      // TODO
        break;
      case ABORT_CRASH:
      // TODO
        break;
      case VOTE_ABORT:
      // TODO
        break;

      default:
        // No valid msgID. exit or do nothing?
        break;
    }

  }
}
