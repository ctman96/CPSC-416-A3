#define _POSIX_C_SOURCE 1

#include "tmanager.h"

void usage(char * cmd) {
  printf("usage: %s  portNum\n",
	 cmd);
}


int main(int argc, char ** argv) {

  // This is some sample code feel free to delete it
  
  unsigned long  port;
  char           logFileName[128];
  int            logfileFD;

  if (argc != 2) {
    usage(argv[0]);
    return -1;
  }
  char * end;
  int err = 0;

  port = strtoul(argv[1], &end, 10);
  if (argv[1] == end) {
    printf("Port conversion error\n");
    exit(-1);
  }

  // Create the TX manager socket
  int sockfd;
  struct sockaddr_in servAddr;

    
  if ( (sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
    perror("socket creation failed"); 
    exit(-1); 
  }

  // Set socket as non-blocking
  int flags = fcntl(sockfd, F_GETFL);
  flags |= O_NONBLOCK;
  fcntl(sockfd, F_SETFL, flags);

  // Setup my server information 
  memset(&servAddr, 0, sizeof(servAddr)); 
  servAddr.sin_family = AF_INET; 
  servAddr.sin_port = htons(port);
  // Accept on any of the machine's IP addresses.
  servAddr.sin_addr.s_addr = INADDR_ANY;
  
  // Bind the socket to the requested addresses and port 
  if ( bind(sockfd, (const struct sockaddr *)&servAddr,  
            sizeof(servAddr)) < 0 )  { 
    perror("bind failed"); 
    exit(-1); 
  }
  
  // At this point our socket is setup and ready to go so we can interact with 
  // workers.

  /* got the port number create a logfile name */
  snprintf(logFileName, sizeof(logFileName), "TXMG_%u.log", port);
  
  logfileFD = open(logFileName, O_RDWR | O_CREAT | O_SYNC, S_IRUSR | S_IWUSR );
  if (logfileFD < 0 ) {
    char msg[256];
    snprintf(msg, sizeof(msg), "Opening %s failed", logFileName);
    perror(msg);
    exit(-1);
  }

  // Let's see if the logfile has some entries in it by checking the size

  struct stat fstatus;
  if (fstat(logfileFD, &fstatus) < 0) {
    perror("Filestat failed");
    exit(-1);
  }
  
  if (fstatus.st_size < sizeof(struct transactionSet)) {
    // Just write out a zeroed file struct
    printf("Initializing the log file size\n");
    struct transactionSet tx;
    bzero(&tx, sizeof(tx));
    if (write(logfileFD, &tx, sizeof(tx)) != sizeof(tx)) {
      printf("Writing problem to log\n");
      exit(-1);
    }
  }

  

  struct transactionSet * txlog = mmap(NULL, 512, PROT_READ | PROT_WRITE, MAP_SHARED, logfileFD, 0);
  if (txlog == NULL) {
    perror("Log file could not be mapped in:");
    exit(-1);
  }

  

  
  if (! txlog->initialized) {
    int i;
    for (i = 0; i  < MAX_WORKERS ; i++) {
      txlog->transaction[i].tstate = TX_NOTINUSE;
    }

    txlog->initialized = -1;
    // Make sure in memory copy is flushed to disk
    msync(txlog, sizeof(struct transactionSet), MS_SYNC | MS_INVALIDATE); 
  }

  // TODO recovery
  
  printf("Starting up Transaction Manager on %d\n", port);
  printf("Port number:              %d\n", port);
  printf("Log file name:            %s\n", logFileName);

  int i;
  unsigned char buff[1024];
  txMsgType message;

  int running = 1;
  while (running) {
    message.msgID = 0;

    socklen_t len;
    struct sockaddr_in client;
    memset(&client, 0, sizeof(client));

    int size = recvfrom(sockfd, &message, sizeof(message), 0, (struct sockaddr *) &client, &len);

    if (size < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
      perror("Receiving error:");
      running = 0;
      abort();
    } else if (size >= 0) {
      switch(message.msgID) {
        case BEGIN_TX:
          if (tm_begin(sockfd, txlog, message.tid, client) < 0) {
            // TODO error?
          };
          break;
        case JOIN_TX:
          if (tm_join(sockfd, txlog, message.tid, client) < 0) {
            // TODO error?
          };
          break;
        case COMMIT_TX:
          if (tm_commit(sockfd, txlog, message.tid, client, 0) < 0) {
            // TODO error?
          };
          break;
        case COMMIT_CRASH_TX:
          if (tm_commit(sockfd, txlog, message.tid, client, 1) < 0) {
            // TODO error?
          };
          break;
        case PREPARE_TX:
          if (tm_prepared(sockfd, txlog, message.tid, client) < 0) {
            // TODO error?
          };
          break;
        case ABORT_TX:
          if (tm_abort(sockfd, txlog, message.tid, client, 0) < 0) {
            // TODO error?
          };
          break;
        case ABORT_CRASH_TX:
          if (tm_abort(sockfd, txlog, message.tid, client, 1) < 0) {
            // TODO error?
          };
          break;
        // TODO: Status polling?
        default:
          break;
      }

      // Check for timed out votes
      for (int i = 0; i < MAX_TX; i++) {
        if (txlog->transaction[i].tstate == TX_VOTING &&
            time(NULL) - txlog->transaction[i].voteTime >  10) {
          if (tm_abort(sockfd, txlog, txlog->transaction[i].txID, client, txlog->transaction[i].crash) < 0) {
            // TODO error
          }
        }
      }

    }
  }

  sleep(1000);


}
