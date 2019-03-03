#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include "utils.h"

static void callClientSystem(const char *command, const char *options);

int main(int argc, char *argv[])
{
  // require user to input server address and server port
  if (argc < 3) {
    fprintf(stderr, "Not enough arguments\n");
    printf("Usage: ./FTPclient ftp-server-ip-address ftp-server-port-number\n");
    exit(EXIT_FAILURE);
  }

  // define server details
  const char * const server_address = argv[1]; 
  int server_port = atoi(argv[2]);
  int sockfd, i;
  struct sockaddr_in addr;
  char buf[MAX_BUF];

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    printf("Can't open socket\n");
    exit(EXIT_FAILURE);
  }

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(server_port);
  inet_aton(server_address, &(addr.sin_addr));

  if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    printf("Can't connect\n");
    exit(EXIT_FAILURE);
  }

  char input[MAX_BUF];
  while (true) {
    bool server_request = false;
    
    printf("ftp > ");
    fgets(buf, sizeof(buf), stdin);
    strncpy(input, buf, sizeof(buf));
    
    char *command = strtok(buf, " \n");
    char *arg1 = strtok(NULL, " ");
    
    // printf("%s\n", input); // uncomment to view input
    if (!strcmp(command, "USER")) {
      // printf("Entered USERNAME\n");
      if (!arg1) {  // No username provided
        printf("Please input username.\n");
      } else {
        server_request = true;
        write(sockfd, input, strlen(input+1));
      }
    } else if (!strcmp(command, "PASS")) {
      // printf("Entered PASSWORD\n");
      if (!arg1) {  // No password provided
        printf("Please input password.\n");
      } else {
        server_request = true;
        write(sockfd, input, strlen(input+1));
      }
    } else if (!strcmp(command, "PUT")) {
      printf("Entered PUT\n");
      if (!arg1) {  // No filename provided
        printf("Please input filename from client.\n");
      } else {
        server_request = true;
        write(sockfd, input, strlen(input+1));
      }
    } else if (!strcmp(command, "GET")) {
      printf("Entered GET\n");
      if (!arg1) {  // No filename provided
        printf("Please input filename from host.\n");
      } else {
        server_request = true;
        write(sockfd, input, strlen(input+1));
      }
    } else if (!strcmp(command, "CD") || !strcmp(command, "LS") || !strcmp(command, "PWD")) {
      server_request = true;
      write(sockfd, input, strlen(input+1));
    } else if (!strcmp(command, "!LS") || !strcmp(command, "!PWD")) {
      callClientSystem(command, input);    
    } else if (!strcmp(command, "!CD")) {
      changeDir(input);
    } else if (!strcmp(command, "QUIT")) {
      exit(EXIT_SUCCESS);
    } else {
      fprintf(stderr, "Command not found\n");
    }
    if (server_request) {
      if (read(sockfd, input, sizeof(input)) == 0) {
        printf("Server closed connection\n");
        exit(EXIT_SUCCESS);
      }
      printf("Server response:\n%s\n", input);
    }
  }
}

static void callClientSystem(const char *command, const char *input) {
  char *options = strchr(input, ' ');
  char shell_command[32];         // 32 is enough space for options
  memset(&shell_command, 0, sizeof(shell_command));
  if (!strcmp(command, "!LS")) {
    strcat(shell_command, "ls");
  } else {
    strcat(shell_command, "pwd");
  }
  if (options) {
    strcat(shell_command, options);
  }
  system(shell_command);
}
