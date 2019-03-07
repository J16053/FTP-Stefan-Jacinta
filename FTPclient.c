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
  const char *server_address = argv[1]; 
  int server_port = atoi(argv[2]);
  
  // Open connection to server
  int server_fd = connectSocket(server_address, server_port);
  
  // Open data socket connection
  int data_port = ntohs(server_port) + 1;  
  int reuse;
  struct serverSocket data_socket = serverSocketSetup(data_port, reuse);
  
  
  // Create buffers
  char buf[MAX_BUF];
  char input[MAX_BUF];
  
  while (true) {
    bool server_request = false;
    
    printf("ftp > ");
    fgets(buf, sizeof(buf), stdin);
    strtok(buf, "\n");  // remove newline character
    strcpy(input, buf);
    char *command = strtok(input, " ");
    char *arg1 = strtok(NULL, " ");
    
    // printf("%s\n", input); // uncomment to view input
    if (!strcmp(command, "USER")) {
      // printf("Entered USERNAME\n");
      if (!arg1) {  // No username provided
        printf("Please input username.\n");
      } else {
        server_request = true;
        write(server_fd, buf, strlen(buf));
      }
    } else if (!strcmp(command, "PASS")) {
      // printf("Entered PASSWORD\n");
      if (!arg1) {  // No password provided
        printf("Please input password.\n");
      } else {
        server_request = true;
        write(server_fd, buf, strlen(buf));
      }
    } else if (!strcmp(command, "PUT") || !strcmp(command, "GET")) {
      if (!arg1) {  // No filename provided
        printf("Please input filename for %s.\n", command);
      } else {
        server_request = true;
        int data_fd;
        
        // send information to server and wait for connection
        write(server_fd, buf, strlen(buf));
        if ((data_fd = accept(data_socket.fd, (struct sockaddr *)(&data_socket.address), &data_socket.len)) < 0) {
            fprintf(stderr, "Can't accept connection\n");
            exit(EXIT_FAILURE);
        }
        
        // retrieve or send file
        if (!strcmp(command, "PUT")) {
          putFile(data_fd, arg1);
        } else {
          getFile(data_fd, arg1);
        }
      }
    } else if (!strcmp(command, "CD") || !strcmp(command, "LS") || !strcmp(command, "PWD")) {
      server_request = true;
      write(server_fd, buf, strlen(buf));
    } else if (!strcmp(command, "!LS") || !strcmp(command, "!PWD")) {
      callClientSystem(command, arg1);    
    } else if (!strcmp(command, "!CD")) {
      changeDir(arg1);
    } else if (!strcmp(command, "QUIT")) {
      exit(EXIT_SUCCESS);
    } else {
      fprintf(stderr, "Command not found\n");
    }
    if (server_request) {
      if (read(server_fd, buf, sizeof(buf)) == 0) {
        printf("Server closed connection\n");
        exit(EXIT_SUCCESS);
      }
      printf("Server response:\n%s\n", buf);
    }
  }
}

static void callClientSystem(const char *command, const char *options) {
  char shell_command[32];         // 32 is enough space for options
  if (!strcmp(command, "!LS")) {
    strcpy(shell_command, "ls ");
  } else {
    strcpy(shell_command, "pwd ");
  }
  if (options) {
    strcat(shell_command, options);
  }
  system(shell_command);
}
