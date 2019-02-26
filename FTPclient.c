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
  // require user to input server address and server port
  if (argc < 3) {
    fprintf(stderr, "Not enough arguments\n");
    printf("Usage: ./FTPclient ftp-server-ip-address ftp-server-port-number\n");
    exit(1);
  }

  // define server details
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
    fgets(buf, sizeof(buf), stdin);
    char input[1024];
    strncpy (input, buf, sizeof(buf));
    bool validInput = false;
    char * command = strtok(buf, " \n");
    char * arg1 = strtok(NULL, " ");
    printf("%s\n", input);
    if (!strcmp(command, "USER")) {
      printf("Entered USERNAME\n");
      if (!arg1) {  // No username provided
        printf("Please input username.\n");
      } else {
        validInput = true;
        write(sockfd, input, strlen(input+1));
      }
    } else if (!strcmp(command, "PASS")) {
      printf("Entered PASSWORD\n");
      if (!arg1) {  // No password provided
        printf("Please input password.\n");
      } else {
        validInput = true;
        write(sockfd, input, strlen(input+1));
      }
    } else if (!strcmp(command, "PUT")) {
      printf("Entered PUT\n");
      if (!arg1) {  // No filename provided
        printf("Please input filename.\n");
      } else {
        validInput = true;
        write(sockfd, input, strlen(input+1));
      }
    } else if (!strcmp(command, "GET")) {
      if (!arg1) {  // No filename provided
        printf("Please input filename.\n");
      } else {
        validInput = true;
        write(sockfd, input, strlen(input+1));
      }
      printf("Entered GET\n");
    }
    if (validInput) {
      if (read(sockfd, input, sizeof(input)) == 0) {
        printf("Server closed connection\n");
        exit(0);
      }
      printf("Server response: %s\n", input);
    }
  }
}