
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


  abortFlag = false;

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
    // All of the switch statements
    // add section for txmanager socket
    // How do we deal with responses from the manager?
    // Questions:
    // Do we update the log->log.newA AND log->txDataA whenever we do newA? (same for B)
    // when do we flush the logfile?
      // to flush the log it, we must also flush the txData; this means that we would be flushing 
      // data to disk to early. must instead just update log, and not txData UNTIL END OF TX.
      // confirm above! ^

    // check for commands
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
      send_message(sockfdCmd, cmdClient, msg);
        break;
        
      case JOINTX:
      // send a join transaction msg to txmanager to join tx TID.
      struct txMsgType msg;
      msg.msgId = JOIN_TX;
      msg.tid = cmdMessage.TID
      send_message(sockfdCmd, cmdClient, msg);
        break;

      case NEW_A:
      // if a transaction not underway, update A. else, update log and then update data
      // if in not active state, then not in tx, so edit A or B directly
      // if in anything but not active state, then in transaction so log 

      if (log->log.txState == WTX_NOTACTIVE) {
        log->txData.A = cmdMessage.newValue;
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

        log->txData.A = cmdMessage.newValue;
        log->log.newA = cmdMessage.newValue;
      }
        break;

      case NEW_B:
      // if a transaction not underway, update B. else, update log and then update data

      if (log->log.txState == WTX_NOTACTIVE) {
        log->txData.B = cmdMessage.newValue;
        if (msync(log, sizeof(struct logFile), MS_SYNC | MS_INVALIDATE)) {
          perror("Msync problem"); 
        }
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
        break;

      case NEW_IDSTR:

      if (log->log.txState == WTX_NOTACTIVE) {
        log->txData.A = cmdMessage.newValue;
        if (msync(log, sizeof(struct logFile), MS_SYNC | MS_INVALIDATE)) {
          perror("Msync problem"); 
        }
      } else {

        // check if old IDstring has already been saved, if not, save it and mark oldSaved
        if ((ID_MASK & log->log.oldSaved) != ID_MASK) {
          log->log.oldIDstring = log->txData.IDstring;
          log->log.oldSaved = log->log.oldSaved | ID_MASK;
          printf("Saved old IDstring\n");\
        }

        log->txData.IDstring = cmdMessage.newValue;
        log->log.newIDstring = cmdMessage.newValue;
      }
        break;

      case DELAY_RESPONSE:
      // TODO - more edge cases for this one
      if (cmdMessage.delay >= 0) {
        sleep(cmdMessage.delay);
      } else {
        sleep(abs(cmdMessage.delay));
        // need to "perform all the actions required by that decision but crash just after before responding to the the coordinator."?
        // if need to do additional processing, can set flag and then crash at end of processing
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
      // TODO - figure out with cody if commented line below is needed, as each worker will only be involved in one transaction
      // msg.tid = log->log.txID;
      send_message(sockfdCmd, cmdClient, msg);
        break;

      case COMMIT_CRASH:
      // send commit message to txmanager that also tells manager to crash
      struct txMsgType msg;
      msg.msgId = COMMIT_CRASH_TX;
      // msg.tid = log->log.txID;
      send_message(sockfdCmd, cmdClient, msg);
        break;

      case ABORT:
      // send abort message to txmanager
      struct txMsgType msg;
      msg.msgId = ABORT_TX;
      // msg.tid = log->log.txID;
      send_message(sockfdCmd, cmdClient, msg);
        break;

      case ABORT_CRASH:
      // send abort message to txmanager that also tells manager to crash
      struct txMsgType msg;
      msg.msgId = ABORT_CRASH_TX;
      // msg.tid = log->log.txID;
      send_message(sockfdCmd, cmdClient, msg);
        break;

      case VOTE_ABORT:
      // vote abort when coordinator next sends prepare message
      abortFlag = true;
        break;

      default:
        // No valid msgID. exit or do nothing?
        break;
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
      printf("Got a transaction packet\n");
  
    switch (txMessage.msgId) {

      case BEGIN_TX:
      // TODO
        break;

      case JOIN_TX:
      // TODO
        break;

      case COMMIT_TX:
      // TODO
        break;

      case COMMIT_CRASH_TX:
      // TODO
        break;

      case PREPARE_TX:
      // TODO
        break;

      case ABORT_TX:
      // TODO
        break;

      case ABORT_CRASH_TX:
      // TODO
        break;

      case SUCCESS_TX:
      // TODO
        break;

      case FAILURE_TX:
      // TODO
        break;

      default:
        // No valid msgID. exit or do nothing?
        break;
    }
  }
}


