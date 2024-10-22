
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/udp.h>
#include <stdio.h>
#include <errno.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <string.h>

#include "msg.h"
#include "transaction_msg.h"
#include "tworker.h"

// timeout vars
bool waiting = false;
unsigned int uncertainStateCtr = 0;
clock_t begin;
clock_t end;

int delayTimer = 0;

bool voteAbortFlag = false;

void usage(char * cmd) {
  printf("usage: %s  cmdportNum txportNum\n",
	 cmd);
}

int send_message(int sockfd, struct addrinfo* tmanager, txMsgType* message) {
    printf("send_message: message: (msgID: %s, tid: %d)\n", txMsgKindToStr(message->msgID), message->tid);
    int bytesSent;
    bytesSent = sendto(sockfd, (void *) message, sizeof(*message), 0, tmanager->ai_addr, tmanager->ai_addrlen);
    if (bytesSent != sizeof(*message)) {
        perror("send_message failed!\n");
        return -1;
    }
    return 0;
}

int send_message2(int sockfd, struct sockaddr_in client, txMsgType* message) {
    printf("send_message: client: %d, message: (msgId: %s, tid: %d)\n", ntohs(client.sin_port), txMsgKindToStr(message->msgID), message->tid);
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
     printf("Already existing logfile");
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
   
    printf("Worker Start");
    printf("Command port:  %d\n", cmdPort);
    printf("TX port:       %d\n", txPort);
    printf("Log file name: %s\n", logFileName);

  

  // Actual worker code
  // -------------------------------------------------------------

  // worker recovery after crash. Only really applicable to undo logging (as documented in README)
  // only should have to revert txData when active or prepared
  printf("Log initalized:  %d\n", log->initialized);
  printf("txState:  %d\n", log->log.txState);
  if (log->initialized && ((log->log.txState == WTX_ACTIVE) || (log->log.txState == WTX_PREPARED))) {
    printf("Recovery - aborting existing transaction\n");
    log->txData.A = log->log.oldA;
    log->txData.B = log->log.oldB;
    strcpy(log->txData.IDstring, log->log.oldIDstring);
    log->log.oldSaved = 0;
    log->log.txState = WTX_ABORTED;

    if (msync(log, sizeof(struct logFile), MS_SYNC | MS_INVALIDATE)) {
        perror("Msync problem"); 
    }
  } else if (! log->initialized) {
    printf("Initializing log\n");
    log->initialized = -1;
    log->log.txState = WTX_NOTACTIVE;
    if (msync(log, sizeof(struct logFile), MS_SYNC | MS_INVALIDATE)) {
      perror("Msync problem");
    }
  }

  printf("TXstate\n");
  printf("%d\n", log->log.txState);
  printf("A \n");
  printf("%d\n", log->txData.A);
  printf("B \n");
  printf("%d\n", log->txData.B);
  printf("IDstring\n");
  printf("%d\n", log->txData.IDstring);

  // create two different sockets, one for cmd and one for txmanager
  int sockfdCmd;
  struct sockaddr_in servAddrCmd;

  int sockfdTx;
  struct sockaddr_in servAddrTx;

  if ( (sockfdCmd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
    perror("Cmd socket creation failed"); 
    exit(-1); 
  }

  if ( (sockfdTx = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
    perror("Tx socket creation failed"); 
    exit(-1); 
  }

  // Set sockets as non-blocking
  int cmdFlags = fcntl(sockfdCmd, F_GETFL);
  cmdFlags |= O_NONBLOCK;
  fcntl(sockfdCmd, F_SETFL, cmdFlags);

  int txFlags = fcntl(sockfdTx, F_GETFL);
  txFlags |= O_NONBLOCK;
  fcntl(sockfdTx, F_SETFL, txFlags);

  // Setup my server information 
  memset(&servAddrCmd, 0, sizeof(servAddrCmd)); 
  servAddrCmd.sin_family = AF_INET; 
  servAddrCmd.sin_port = htons(cmdPort);
  // Accept on any of the machine's IP addresses.
  servAddrCmd.sin_addr.s_addr = INADDR_ANY;

  memset(&servAddrTx, 0, sizeof(servAddrTx)); 
  servAddrTx.sin_family = AF_INET; 
  servAddrTx.sin_port = htons(txPort);
  // Accept on any of the machine's IP addresses.
  servAddrTx.sin_addr.s_addr = INADDR_ANY;

  // Bind the sockets to the requested addresses and ports
  if ( bind(sockfdCmd, (const struct sockaddr *)&servAddrCmd,  
            sizeof(servAddrCmd)) < 0 )  { 
    perror("bind failed"); 
    exit(-1); 
  }

  if ( bind(sockfdTx, (const struct sockaddr *)&servAddrTx,  
            sizeof(servAddrTx)) < 0 )  { 
    perror("bind failed"); 
    exit(-1); 
  }

  msgType cmdMessage;
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
    // - What is the "required local processing" that occurs after waiting delay time and then crashing?
    // - voteabort must make NEXT commit abort. Conflicting piazza posts! Refer to @520 though.
    // but in the same vein, @392 Acton says that " If there is no transaction under way and 
    // you get a voteabort it can simply be ignored" which means that we should follow this approach?
    // ^ from above, we're following 392. 361 conflicts with this too though! JEEZ!
    // okay, from what I can tell it boils down to waiting until you're asked for a vote, and THEN voting 
    // abort. the abort command would just send an abort at any time, but voteabort waits until asked for
    // votes

    // CHECK:
    // - entering in progress states and exiting. Make sure that current approach is good with tas
    // - cleared important things in log when you commit or abort
    // - worker recovery after crash
    // - perhaps we should add an "uncertain" state, but would require removing the 
    // - timeouts working correctly

    // Questions:
    // does it matter what msgID is?
    // check for commands. If currently waiting for a response, then skip this

    // at beginning, if we have an abort or a commit we know we can set state to notactive as we're done last tx
    if (log->initialized && ((log->log.txState == WTX_COMMITTED) || (log->log.txState == WTX_ABORTED))) {
      log->log.txState = WTX_NOTACTIVE;
    }

    if (!waiting) {
      cmdMessage.msgID = 0;
      cmdLen = sizeof(cmdClient);
      memset(&cmdClient, 0, sizeof(cmdClient));
      int size = recvfrom(sockfdCmd, &cmdMessage, sizeof(cmdMessage), 0, (struct sockaddr *) &cmdClient, &cmdLen);

      if (size == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
        perror("Receiving error:");
        running = 0;
        abort();
      } else if (size >= 0) {
        // printf("Got a command packet\n");
        printf("Received cmd message (msgId: %d)\n", cmdMessage.msgID);
      }
    
      switch (cmdMessage.msgID) {

        case BEGINTX: {
          // Setup tmanager address
          char *hostname = "localhost";
          struct addrinfo hints, *tmanagerAddr;
          tmanagerAddr = NULL;
          memset(&hints, 0, sizeof(hints));
          hints.ai_socktype = SOCK_DGRAM;
          hints.ai_family = AF_INET;
          hints.ai_protocol = IPPROTO_UDP;
          char port_str[16];
          sprintf(port_str, "%d", cmdMessage.port);
          if (getaddrinfo(hostname, port_str, &hints, &tmanagerAddr)) {
              perror("Couldn't lookup hostname");
              return -1;
          }
          // send a begin transaction message to manager (with txid TID). Read docs for errors
          // if manager returns error (BC id is already in use), worker exits
          txMsgType msg;
          msg.msgID = BEGIN_TX;
          msg.tid = cmdMessage.tid;
          send_message(sockfdTx, tmanagerAddr, &msg);
          waiting = true;
          begin = time(NULL);
        } break;
          
        case JOINTX: {
          // Setup tmanager address
          char *hostname = "localhost";
          struct addrinfo hints, *tmanagerAddr;
          tmanagerAddr = NULL;
          memset(&hints, 0, sizeof(hints));
          hints.ai_socktype = SOCK_DGRAM;
          hints.ai_family = AF_INET;
          hints.ai_protocol = IPPROTO_UDP;
          char port_str[16];
          sprintf(port_str, "%d", cmdMessage.port);
          if (getaddrinfo(hostname, port_str, &hints, &tmanagerAddr)) {
              perror("Couldn't lookup hostname");
              return -1;
          }
          // send a join transaction msg to txmanager to join tx TID.
          txMsgType msg;
          msg.msgID = JOIN_TX;
          msg.tid = cmdMessage.tid;
          send_message(sockfdTx, tmanagerAddr, &msg);
          waiting = true;
          begin = time(NULL);
        } break;

        case NEW_A: {
          // if a transaction not underway, update A. else, update log and then update data
          // if in not active state, edit A or B directly
          // if in transaction, then log 

          printf("Before oldSaved: %d\n", log->log.oldSaved);
          printf("Before A: %d\n", log->txData.A);


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

          printf("After oldSaved: %d\n", log->log.oldSaved);
          printf("After A: %d\n", log->txData.A);

        } break;

        case NEW_B: {
          // if a transaction not underway, update B. else, update log and then update data
          printf("Before oldSaved: %d\n", log->log.oldSaved);


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

          printf("After oldSaved: %d\n", log->log.oldSaved);

        } break;

        case NEW_IDSTR: {
          printf("Before oldSaved: %d\n", log->log.oldSaved);

          if (log->log.txState == WTX_NOTACTIVE) {
            log->txData.A = cmdMessage.newValue;
          } else {

            // check if old IDstring has already been saved, if not, save it and mark oldSaved
            if ((ID_MASK & log->log.oldSaved) != ID_MASK) {
              strcpy(log->log.oldIDstring, log->txData.IDstring);
              log->log.oldSaved = log->log.oldSaved | ID_MASK;
              printf("Saved old IDstring\n");
            }

            strcpy(log->txData.IDstring, cmdMessage.strData.newID);
            strcpy(log->log.newIDstring, cmdMessage.strData.newID);
          }

          if (msync(log, sizeof(struct logFile), MS_SYNC | MS_INVALIDATE)) {
            perror("Msync problem");
          }

          printf("After oldSaved: %d\n", log->log.oldSaved);

        } break;

        case DELAY_RESPONSE: {
          delayTimer = cmdMessage.delay;
        } break;

        case CRASH: {
          // simulate crash by calling _exit()
          _exit(EXIT_SUCCESS);
        } break;

        case COMMIT: {
          // Setup tmanager address
          char *hostname = "localhost";
          struct addrinfo hints, *tmanagerAddr;
          tmanagerAddr = NULL;
          memset(&hints, 0, sizeof(hints));
          hints.ai_socktype = SOCK_DGRAM;
          hints.ai_family = AF_INET;
          hints.ai_protocol = IPPROTO_UDP;
          char port_str[16];
          sprintf(port_str, "%d", log->log.port);
          if (getaddrinfo(hostname, port_str, &hints, &tmanagerAddr)) {
              perror("Couldn't lookup hostname");
              return -1;
          }
          // send commit message to txmanager
          txMsgType msg;
          msg.msgID = COMMIT_TX;
          msg.tid = log->log.txID;

          log->log.txState = WTX_UNCERTAIN;

          if (msync(log, sizeof(struct logFile), MS_SYNC | MS_INVALIDATE)) {
            perror("Msync problem");
          }
          waiting = true;
          begin = time(NULL);
          send_message(sockfdTx, tmanagerAddr, &msg);
        } break;

        case COMMIT_CRASH: {
          // Setup tmanager address
          char *hostname = "localhost";
          struct addrinfo hints, *tmanagerAddr;
          tmanagerAddr = NULL;
          memset(&hints, 0, sizeof(hints));
          hints.ai_socktype = SOCK_DGRAM;
          hints.ai_family = AF_INET;
          hints.ai_protocol = IPPROTO_UDP;
          char port_str[16];
          sprintf(port_str, "%d", log->log.port);
          if (getaddrinfo(hostname, port_str, &hints, &tmanagerAddr)) {
              perror("Couldn't lookup hostname");
              return -1;
          }
          // send commit message to txmanager that also tells manager to crash
          txMsgType msg;
          msg.msgID = COMMIT_CRASH_TX;
          msg.tid = log->log.txID;

          log->log.txState = WTX_UNCERTAIN;

          if (msync(log, sizeof(struct logFile), MS_SYNC | MS_INVALIDATE)) {
            perror("Msync problem");
          }
          waiting = true;
          begin = time(NULL);
          send_message(sockfdTx, tmanagerAddr, &msg);
        } break;

        case ABORT: {
          // Setup tmanager address
          char *hostname = "localhost";
          struct addrinfo hints, *tmanagerAddr;
          tmanagerAddr = NULL;
          memset(&hints, 0, sizeof(hints));
          hints.ai_socktype = SOCK_DGRAM;
          hints.ai_family = AF_INET;
          hints.ai_protocol = IPPROTO_UDP;
          char port_str[16];
          sprintf(port_str, "%d", log->log.port);
          if (getaddrinfo(hostname, port_str, &hints, &tmanagerAddr)) {
              perror("Couldn't lookup hostname");
              return -1;
          }
          // send abort message to txmanager
          txMsgType msg;
          msg.msgID = ABORT_TX;
          msg.tid = log->log.txID;
          send_message(sockfdTx, tmanagerAddr, &msg);
        } break;

        case ABORT_CRASH: {
          // Setup tmanager address
          char *hostname = "localhost";
          struct addrinfo hints, *tmanagerAddr;
          tmanagerAddr = NULL;
          memset(&hints, 0, sizeof(hints));
          hints.ai_socktype = SOCK_DGRAM;
          hints.ai_family = AF_INET;
          hints.ai_protocol = IPPROTO_UDP;
          char port_str[16];
          sprintf(port_str, "%d", log->log.port);
          if (getaddrinfo(hostname, port_str, &hints, &tmanagerAddr)) {
              perror("Couldn't lookup hostname");
              return -1;
          }
          // send abort message to txmanager that also tells manager to crash
          txMsgType msg;
          msg.msgID = ABORT_CRASH_TX;
          msg.tid = log->log.txID;
          send_message(sockfdTx, tmanagerAddr, &msg);
        } break;

        case VOTE_ABORT: {
          // set flag, ignore if no transaction. Can prob remove the check for aborted state as well
          if ((log->log.txState != WTX_NOTACTIVE) && (log->log.txState != WTX_ABORTED)) {
            voteAbortFlag = true;
          } 
        } break;

        default:
          break;
      }
    }


    // now handle txmanager packets
    txMessage.msgID = 0;

    txLen = sizeof(txClient);
    memset(&txClient, 0, sizeof(txClient));
    int size = recvfrom(sockfdTx, &txMessage, sizeof(txMessage), 0, (struct sockaddr *) &txClient, &txLen);

    if (size == -1 && errno != EAGAIN && errno != EWOULDBLOCK) {
      perror("Receiving error:");
      running = 0;
      _exit(EXIT_SUCCESS);
    } else if (size >= 0) {
      printf("Got a txmanager packet\n");
      printf("Received tx message (msgId: %s, tid: %d)\n", txMsgKindToStr(txMessage.msgID), txMessage.tid);
    }
  
    switch (txMessage.msgID) {

      case PREPARE_TX: {
        waiting = false;
        // vote prepared by default; if have voteAbort flag set, then respond abort instead of prepared
        if (delayTimer != 0) {
          sleep(abs(delayTimer));
        }

        if (!voteAbortFlag) {
          txMsgType msg;
          msg.msgID = PREPARE_TX;
          msg.tid = log->log.txID;
          // enter into an uncertain state until we receive a response from the txmanager
          log->log.txState = WTX_PREPARED;
          begin = time(NULL);

          if (msync(log, sizeof(struct logFile), MS_SYNC | MS_INVALIDATE)) {
            perror("Msync problem"); 
          }
          
          if (delayTimer < 0) {
            perror("Negative Delay Crash\n");
            _exit(EXIT_SUCCESS);
          } else {
            delayTimer = 0;
          }

          send_message2(sockfdTx, txClient, &msg);

          log->log.txState = WTX_UNCERTAIN;
          if (msync(log, sizeof(struct logFile), MS_SYNC | MS_INVALIDATE)) {
            perror("Msync problem");
          }

        } else {
          txMsgType msg;
          msg.msgID = ABORT_TX;
          msg.tid = log->log.txID;
          voteAbortFlag = false;
          begin = time(NULL);

          log->log.txState = WTX_ABORTED;
          if (msync(log, sizeof(struct logFile), MS_SYNC | MS_INVALIDATE)) {
            perror("Msync problem"); 
          }

          if (delayTimer < 0) {
            perror("Negative Delay Crash\n");
            _exit(EXIT_SUCCESS);
          } else {
            delayTimer = 0;
          }

          send_message2(sockfdTx, txClient, &msg);
        }



      } break;

      case COMMIT_TX: {
        // got commit message from txmanager, so flush txData
        // copy newVals to txData just for procedure's sake

        log->txData.A = log->log.newA;
        log->txData.B = log->log.newB;
        strcpy(log->txData.IDstring, log->log.newIDstring);
              
        // reset oldSaved to indicate no old values saved
        log->log.oldSaved = 0;

        // sync logfile
        if (msync(log, sizeof(struct logFile), MS_SYNC | MS_INVALIDATE)) {
          perror("Msync problem"); 
        }

        log->log.txState = WTX_COMMITTED;
        uncertainStateCtr = 0;

        if (msync(log, sizeof(struct logFile), MS_SYNC | MS_INVALIDATE)) {
          perror("Msync problem"); 
        }

      } break;

      case ABORT_TX: {
        // write abort to log, revert txData to oldValues
        log->txData.A = log->log.oldA;
        log->txData.B = log->log.oldB;
        strcpy(log->txData.IDstring, log->log.oldIDstring);

        // reset oldSaved to indicate no old values saved
        log->log.oldSaved = 0;

        // sync logfile
        if (msync(log, sizeof(struct logFile), MS_SYNC | MS_INVALIDATE)) {
          perror("Msync problem"); 
        }

        log->log.txState = WTX_ABORTED;
        uncertainStateCtr = 0;

        if (msync(log, sizeof(struct logFile), MS_SYNC | MS_INVALIDATE)) {
          perror("Msync problem"); 
        }
      } break;

      case SUCCESS_TX: {
        // on success, set log transaction id to msg.tid
        log->initialized = -1;
        log->log.txID = txMessage.tid;
        log->log.txState = WTX_ACTIVE;
        log->log.port = ntohs(txClient.sin_port);
        printf("Success");
        waiting = false;

        if (msync(log, sizeof(struct logFile), MS_SYNC | MS_INVALIDATE)) {
          perror("Msync problem"); 
        }
      } break;

      case FAILURE_TX: {
        perror("Failure");
        _exit(EXIT_SUCCESS);
      } break;

      default:
        break;
    }


    // if in uncertain state, wait 30 seconds, then resend prepared every 10 seconds
    // if just waiting, then timeout after 10 seconds
    if (log->log.txState == WTX_UNCERTAIN) {
      // end = time(NULL);
      double time_spent = (double)(time(NULL) - begin);
      // if waiting and time spent waiting longer than uncertain_timeout, exit. if not waiting, then send a message every 10 seconds after timeout time
      if (waiting && (time_spent >= UNCERTAIN_TIMEOUT)) {
        // write abort to log, revert txData to oldValues
        log->txData.A = log->log.oldA;
        log->txData.B = log->log.oldB;
        strcpy(log->txData.IDstring, log->log.oldIDstring);

        // reset oldSaved to indicate no old values saved
        log->log.oldSaved = 0;

        // sync logfile
        if (msync(log, sizeof(struct logFile), MS_SYNC | MS_INVALIDATE)) {
          perror("Msync problem"); 
        }

        printf("Timeout waiting for response to COMMIT, aborting transaction\n");
        log->log.txState = WTX_ABORTED;
        waiting = false;

        if (msync(log, sizeof(struct logFile), MS_SYNC | MS_INVALIDATE)) {
          perror("Msync problem"); 
        }

      } else if (time_spent >= UNCERTAIN_TIMEOUT) {
        int waitTime = time_spent - UNCERTAIN_TIMEOUT - uncertainStateCtr * 10;
        if (waitTime >= 0) {
            // Setup tmanager address
          char *hostname = "localhost";
          struct addrinfo hints, *tmanagerAddr;
          tmanagerAddr = NULL;
          memset(&hints, 0, sizeof(hints));
          hints.ai_socktype = SOCK_DGRAM;
          hints.ai_family = AF_INET;
          hints.ai_protocol = IPPROTO_UDP;
          char port_str[16];
          sprintf(port_str, "%d", log->log.port);
          if (getaddrinfo(hostname, port_str, &hints, &tmanagerAddr)) {
              perror("Couldn't lookup hostname");
              return -1;
          }

          txMsgType msg;
          msg.msgID = POLL_STATE_TX;
          msg.tid = log->log.txID;
          send_message(sockfdTx, tmanagerAddr, &msg);
          uncertainStateCtr += 1;
        }
      }
    } else if (waiting) {
      double time_spent = (double)(time(NULL) - begin);
      if (time_spent >= TIMEOUT) {
        printf("State: %d\n", log->log.txState);
        perror("Waiting timeout exit\n");
        _exit(EXIT_SUCCESS);
      }
    }
  }
}