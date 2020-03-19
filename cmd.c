#include <stdio.h> 
#include <stdlib.h> 
#include <unistd.h> 
#include <string.h> 
#include <sys/types.h> 
#include <sys/socket.h> 
#include <arpa/inet.h> 
#include <netinet/in.h> 
#include <errno.h>
#include <time.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>


#include "msg.h"

void sendmessage(char * hostname, char * port, msgType * msg) {
	
  int sockfd;
  struct addrinfo h1;
  struct addrinfo hints, *servinfo, *p;
  int rv;
  int numbytes;
  
  // specify socket options
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_DGRAM;
  
  if ((rv = getaddrinfo(hostname, port, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    exit(1);
  }
  
  // loop through all the results and make a socket
  for(p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("talker: socket");
      continue;
    }
    
        break;
  }
  
  if (p == NULL) {
        fprintf(stderr, "talker: failed to bind socket\n");
        exit(1);
    }
  
  if ((numbytes = sendto(sockfd, msg, sizeof(msgType), 0, p->ai_addr, p->ai_addrlen)) == -1) {
    perror("talker: sendto");
    exit(1);
    }
  
    freeaddrinfo(servinfo);
    close(sockfd);
}

int main(int argc, char ** argv) {
  
  msgType * msg;
  char * cmd;
  
  // The cmd line options dictate what is done. The format is:
  // CMD  hostname port additional arguments as described in
  // the assignment.
  msg = malloc(sizeof(msgType));
  memset((msg->strData).hostName, 0, sizeof((msg->strData).hostName));
  memset((msg->strData).newID, 0, sizeof((msg->strData).newID));
  cmd = calloc(255, 1);
  
  // parse command
  if (strcmp(argv[1], "begin") == 0) {
    msg->msgID = BEGINTX;
    msg->tid = atoi(argv[6]);
    msg->port = atoi(argv[5]);
    strncpy((msg->strData).hostName, argv[4], strlen(argv[4]));
      printf("here: %s\n", (msg->strData).hostName);
      sendmessage(argv[2], argv[3], msg);
  }
  else if (strcmp(argv[1], "join") == 0) {
    msg->msgID = JOINTX;
    msg->tid = atoi(argv[6]);
    msg->port = atoi(argv[5]);
    strncpy((msg->strData).hostName, argv[4], strlen(argv[4]));
    printf("here: %s\n", (msg->strData).hostName);
    sendmessage(argv[2], argv[3], msg);
  }
  else if (strcmp(argv[1], "newa") == 0) {
    msg->msgID = NEW_A;
    msg->newValue = atoi(argv[4]);
    sendmessage(argv[2], argv[3], msg);
  }
  else if (strcmp(argv[1], "newb") == 0) {
    msg->msgID = NEW_B;
    msg->newValue = atoi(argv[4]);
    sendmessage(argv[2], argv[3], msg);
  }
  else if (strcmp(argv[1], "newid") == 0) {
    msg->msgID = NEW_IDSTR;
    strncpy((msg->strData).newID, argv[4], strlen(argv[4]));
    sendmessage(argv[2], argv[3], msg);
  }
  else if (strcmp(argv[1], "crash") == 0) {
    msg->msgID = CRASH;
    sendmessage(argv[2], argv[3], msg);
  }
  else if (strcmp(argv[1], "delay") == 0) {
    msg->msgID = DELAY_RESPONSE;
    sendmessage(argv[2], argv[3], msg);
    }
  else if (strcmp(argv[1], "commit") == 0) {
    msg->msgID = COMMIT;
    sendmessage(argv[2], argv[3], msg);
    }
  else if (strcmp(argv[1], "commitcrash") == 0) {
    msg->msgID = COMMIT_CRASH;
    sendmessage(argv[2], argv[3], msg);
  }
  else if (strcmp(argv[1], "abort") == 0) {
      msg->msgID = ABORT;
      sendmessage(argv[2], argv[3], msg);
  }
  else if (strcmp(argv[1], "abortcrash") == 0) {
    msg->msgID = ABORT_CRASH;
    sendmessage(argv[2], argv[3], msg);
    }
  else if (strcmp(argv[1], "voteabort") == 0) {
    msg->msgID = VOTE_ABORT;
      sendmessage(argv[2], argv[3], msg);
  }
  else {
    printf("error: not a valid command\n");
  }
}
