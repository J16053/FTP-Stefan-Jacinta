#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

int main(int argc, char * argv[])
{
  if (argc < 3) {
    fprintf(stderr, "Not enough arguments\n");
    printf("Usage: ./FTPclient ftp-server-ip-address ftp-server-port-number\n");
    exit(1);
  }
  const char * const server_address = argv[1]; // CAN'T CHANGE server_address
  int server_port = atoi(argv[2]);
  int sockfd, i;
  struct sockaddr_in addr;
  char buf[1024];

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    printf("Can't open socket\n");
    exit(1);
  }

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(server_port);
  inet_aton(server_address, &(addr.sin_addr));

  if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    printf("Can't connect\n");
    exit(1);
  }

  while (true) {
    printf("ftp > ");
    fgets(buf, 1024, stdin);
    write(sockfd, buf, strlen(buf+1));
    if (read(sockfd, buf, 1024) == 0) {
      printf("Server closed connection\n");
      exit(0);
    }
    printf("Server response: %s\n", buf);
  }
}