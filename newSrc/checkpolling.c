#define _POSIX_C_SOURCE 1
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
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <strings.h>

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
  
  printf("Starting up Transaction Manager poll checker on %d\n", port);

  for (;;) {
    struct sockaddr_in client;
    socklen_t len;
    int n;
    unsigned char buff[50];
    bzero(&client, sizeof(client));
    n = recvfrom(sockfd, buff, sizeof(buff), MSG_WAITALL,
		 (struct sockaddr *) &client, &len);
    if (n < 0) {
      perror("Receiving error:");
      // abort();
    } else {
      unsigned long ip;
      unsigned long port;
      ip =client.sin_addr.s_addr;
      port = ntohs(client.sin_port);
      printf("Recieved from: %d.%d.%d.%d: %d\n",
	     (ntohl(ip) >> 24) & 0xFF,
	     (ntohl(ip) >> 16) & 0xFF,
	     (ntohl(ip) >> 8) & 0xFF,
	     ntohl(ip) & 0xFF, port
	     );
      fflush(NULL);
    }
  }
}
