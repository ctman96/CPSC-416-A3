
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

// might move this into new file
int send_message(int sockfd, struct sockaddr_in client, txMsgType* message) {
    printf("send_message: client: %d, message: (msgId: %d, tid: %d, state: %d)\n", client.sin_port, message->msgID, message->tid, message->state);
    int bytesSent;
    bytesSent = sendto(sockfd, (void *) message, sizeof(*message), 0, (struct sockaddr *) &client, sizeof(client));
    if (bytesSent != sizeof(*message)) {
        perror("send_message failed!\n");
        return -1;
    }
    return 0;
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
   } else {
     printf("Already existing logfile")
   }

   // Now map the file in.
   struct logFile  *log = mmap(NULL, 512, PROT_READ | PROT_WRITE, MAP_SHARED, logfileFD, 0);
   if (log == NULL) {
     perror("Log file could not be mapped in:");
     exit(-1);
   }

   // Some demo data
  //  strncpy(log->txData.IDstring, "Hi there!! :-)", IDLEN);
  //  log->txData.A = 10;
  //  log->txData.B = 100;
  //  log->log.oldA = 83;
  //  log->log.newA = 10;
  //  log->log.oldB = 100;
  //  log->log.newB = 1023;
  //  log->initialized = -1;
  //  if (msync(log, sizeof(struct logFile), MS_SYNC | MS_INVALIDATE)) {
  //    perror("Msync problem");
  //  }
  // Some demo data
  //  strncpy(log->log.newIDstring, "1234567890123456789012345678901234567890", IDLEN);
   
    printf("Worker Start")
    printf("Command port:  %d\n", cmdPort);
    printf("TX port:       %d\n", txPort);
    printf("Log file name: %s\n", logFileName);

  

  // Actual worker code
  // -------------------------------------------------------------

  // worker recovery after crash. Only really applicable to undo logging (as documented in README)
  // only should have to revert txData when active or prepared
  if ((log->log.txState == WTX_ACTIVE) || (log->log.txState == WTX_PREPARED)) {
    log->txData.A = log->log.oldA;
    log->txData.B = log->log.oldB;
    log->txData.IDstring = log->log.oldIDstring;

    log->log.txState = WTX_NOTACTIVE
  }


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

  // timeout vars
  bool waiting = false;
  clock_t begin;
  clock_t end;

  bool voteAbortFlag = false;

  txMsgType cmdMessage;
  socklen_t cmdLen;
  struct sockaddr_in cmdClient;

  txMsgType txMessage;
  socklen_t txLen;
  struct sockaddr_in txClient;

  int running = 1;
  while (running) {
    // check for messages
    // switch on recieved message id
    // send msgs or edit data depending on msg
    // ---------------------------------------------------
    // TODO:
    // - make sure coming back from crash is proper!
    // - timeouts. Might have to keep track of last message sent out to txmanager for normal procedures, and if haven't gotten
    // response in timeout (10 secs) then fail?
    // - voteabort must make NEXT commit abort. Conflicting piazza posts! Refer to @520 though.

    // CHECK:
    // - entering in progress states and exiting. Make sure that current approach is good with tas
    // - cleared important things in log when you commit or abort
    // - worker recovery after crash

    // Questions:
    // does it matter what msgId is?
    // check for commands. If currently waiting for a response, then skip this

    // at beginning, if we have an abort or a commit we know we can set state to notactive
    if ((log->log.txState == WTX_COMMITTED) || (log->log.txState == WTX_ABORTED)) {
      log->log.txState = WTX_NOTACTIVE;
    }

    if (!waiting) {
      cmdMessage.msgId = 0;
      cmdLen = sizeof(cmdClient);
      memset(&cmdClient, 0, sizeof(cmdClient));
      int size = recvfrom(sockfdCmd, &cmdMessage, sizeof(cmdMessage), 0, (struct sockaddr *) &cmdClient, &cmdLen);

      if (size == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
        perror("Receiving error:");
        running = 0;
        abort();
      } else if (n >= 0) {
        printf("Got a command packet\n");
    
      switch (cmdMessage.msgId) {

        case BEGINTX:
        // send a begin transaction message to manager (with txid TID). Read docs for errors
        // if manager returns error (BC id is already in use), worker exits
        struct txMsgType msg;
        msg.msgId = BEGIN_TX;
        msg.tid = cmdMessage.TID
        send_message(sockfdTx, txClient, msg);
        begin = clock();
        waiting = true;
          break;
          
        case JOINTX:
        // send a join transaction msg to txmanager to join tx TID.
        struct txMsgType msg;
        msg.msgId = JOIN_TX;
        msg.tid = cmdMessage.TID
        send_message(sockfdTx, txClient, msg);
        begin = clock();
        waiting = true;
          break;

        case NEW_A:
        // if a transaction not underway, update A. else, update log and then update data
        // if in not active state, edit A or B directly
        // if in transaction, then log 

        if (log->log.txState == WTX_NOTACTIVE) {
          log->txData.A = cmdMessage.newValue;
        } else {
          // check if old A has already been saved, if not, save it and mark oldSaved
          if ((A_MASK & log->log.oldSaved) != A_MASK) {
            log->log.oldA = log->txData.A;
            log->log.oldSaved = log->log.oldSaved | A_MASK;
            printf("Saved old A\n");\
          }

          log->txData.A = cmdMessage.newValue;
          log->log.newA = cmdMessage.newValue;
        }

        if (msync(log, sizeof(struct logFile), MS_SYNC | MS_INVALIDATE)) {
            perror("Msync problem"); 
        }
          break;

        case NEW_B:
        // if a transaction not underway, update B. else, update log and then update data

        if (log->log.txState == WTX_NOTACTIVE) {
          log->txData.B = cmdMessage.newValue;
        } else {
          // check if old B has already been saved, if not, save it and mark oldSaved
          if ((B_MASK & log->log.oldSaved) != B_MASK) {
            log->log.oldB = log->txData.B;
            log->log.oldSaved = log->log.oldSaved | B_MASK;
            printf("Saved old B\n");\
          }

          log->txData.B = cmdMessage.newValue;
          log->log.newB = cmdMessage.newValue;
        }

        if (msync(log, sizeof(struct logFile), MS_SYNC | MS_INVALIDATE)) {
          perror("Msync problem"); 
        }
          break;

        case NEW_IDSTR:

        if (log->log.txState == WTX_NOTACTIVE) {
          log->txData.A = cmdMessage.newValue;
        } else {

          // check if old IDstring has already been saved, if not, save it and mark oldSaved
          if ((ID_MASK & log->log.oldSaved) != ID_MASK) {
            log->log.oldIDstring = log->txData.IDstring;
            log->log.oldSaved = log->log.oldSaved | ID_MASK;
            printf("Saved old IDstring\n");
          }

          log->txData.IDstring = cmdMessage.newValue;
          log->log.newIDstring = cmdMessage.newValue;
        }

        if (msync(log, sizeof(struct logFile), MS_SYNC | MS_INVALIDATE)) {
          perror("Msync problem"); 
        }
          break;

        case DELAY_RESPONSE:
        if (cmdMessage.delay >= 0) {
          sleep(cmdMessage.delay);
        } else {
          sleep(abs(cmdMessage.delay));
          // TODO - need to "perform all the actions required by that decision but crash just after before responding to the the coordinator."?
          // if need to do additional processing, can set flag and then crash at end of processing.
          // What descisions need to be made??
          _exit();
        }
          break;

        case CRASH:
        // simulate crash by calling _exit()
        _exit();
          break;

        case COMMIT:
        // send commit message to txmanager
        struct txMsgType msg;
        msg.msgId = COMMIT_TX;
        msg.tid = log->log.txID;
        send_message(sockfdTx, txClient, msg);
          break;

        case COMMIT_CRASH:
        // send commit message to txmanager that also tells manager to crash
        struct txMsgType msg;
        msg.msgId = COMMIT_CRASH_TX;
        msg.tid = log->log.txID;
        send_message(sockfdTx, txClient, msg);
          break;

        case ABORT:
        // send abort message to txmanager
        struct txMsgType msg;
        msg.msgId = ABORT_TX;
        msg.tid = log->log.txID;
        send_message(sockfdTx, txClient, msg);
          break;

        case ABORT_CRASH:
        // send abort message to txmanager that also tells manager to crash
        struct txMsgType msg;
        msg.msgId = ABORT_CRASH_TX;
        msg.tid = log->log.txID;
        send_message(sockfdTx, txClient, msg);
          break;

        case VOTE_ABORT:
        // vote abort, ignore if no transaction
        // TODO - can we vote abort after committing?? Might need to add another state to the workerTxState that's just INTRANSACTIOn
        if ((log->log.txState != WTX_NOTACTIVE) && (log->log.txState != WTX_ABORTED)) {
          struct txMsgType msg;
          msg.msgId = ABORT_TX;
          msg.tid = log->log.txID;
          send_message(sockfdTx, txClient, msg);
        } else {
          voteAbortFlag = true;
        }
          break;

        default:
          break;
      }
    }


    // now handle txmanager packets
    txMessage.msgId = 0;

    txLen = sizeof(txClient);
    memset(&txClient, 0, sizeof(txClient));
    int size = recvfrom(sockfdtx, &txMessage, sizeof(txMessage), 0, (struct sockaddr *) &txClient, &txLen);

    if (size == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
      perror("Receiving error:");
      running = 0;
      abort();
    } else if (n >= 0) {
      printf("Got a txmanager packet\n");
  
    switch (txMessage.msgId) {

      case PREPARE_TX:
      // TODO
      struct txMsgType msg;
      msg.msgId = PREPARE_TX;
      send_message(sockfdTx, txClient, msg);
      log->log.txState = WTX_PREPARED;
        break;

      case COMMIT_TX:
      // got commit message from txmanager, so flush txData
      // copy newVals to txData just for procedure's sake
      log->txData.A = log->log.newA;
      log->txData.B = log->log.newB;
      log->txData.IDstring = log->log.newIDstring;
      
      // reset oldSaved to indicate no old values saved
      log->log.oldSaved = 0;

      // sync logfile
      if (msync(log, sizeof(struct logFile), MS_SYNC | MS_INVALIDATE)) {
        perror("Msync problem"); 
      }

      log->log.txState = WTX_COMMITTED;
        break;

      case ABORT_TX:
      // write abort to log, revert txData to oldValues
      log->txData.A = log->log.oldA;
      log->txData.B = log->log.oldB;
      log->txData.IDstring = log->log.oldIDstring;

      // reset oldSaved to indicate no old values saved
      log->log.oldSaved = 0;

      // sync logfile
      if (msync(log, sizeof(struct logFile), MS_SYNC | MS_INVALIDATE)) {
        perror("Msync problem"); 
      }

      log->log.txState = WTX_ABORTED;
        break;

      case SUCCESS_TX:
      // on success, set log transaction id to msg.tid
      log->log.txID = txMessage.tid;
      log->log.txState = WTX_ACTIVE;
      printf("Success")
      waiting = false;
        break;

      case FAILURE_TX:
      // TODO - print failure reason if there is one, or just print fail
      printf("Failure")
      // waiting = false;
      _exit();
        break;

      default:
        break;
    }

    end = clock();
    double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
    if (time_spent >= TIMEOUT) {
      waiting = false;
      // TODO - do whatever is required at this point
    }

  }
}


