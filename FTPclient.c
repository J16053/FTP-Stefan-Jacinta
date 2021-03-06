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
  
  // Determine data port
  struct sockaddr_in address;
  int len = sizeof(address);
  getsockname(server_fd, (struct sockaddr *)&address, (socklen_t*)&len);
  int data_port = ntohs(address.sin_port) + 1;
  
  // Bind and listen port for data transfer
  int reuse;
  struct serverSocket data_socket = serverSocketSetup(data_port, reuse);
  
  printf("My address and port: %s:%d\n", inet_ntoa(address.sin_addr), ntohs(address.sin_port));
  printf("My data address and port: %s:%d\n", inet_ntoa(data_socket.address.sin_addr), ntohs(data_socket.address.sin_port));
  
  // Create buffers
  char buf[MAX_BUF];
  char input[MAX_BUF];
  
  // loop on user requests
  while (true) {
    struct timeval timeout;
    timeout.tv_sec = 2;  // no specific reason for 2 second timeout
    timeout.tv_usec = 0;
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
      } else {  // some filename provided
        
        if (!strcmp(command, "PUT")) {
          if (!fileExists(arg1)) {
            perror("Error on PUT");
            continue;
          }
        }
        
        // clear the socket set
        fd_set read_fd_set;
        FD_ZERO(&read_fd_set);
        FD_SET(data_socket.fd, &read_fd_set);
        
        // send request to server
        server_request = true;
        write(server_fd, buf, strlen(buf));

        // wait for connection on data_socket
        int res = select(data_socket.fd+1, &read_fd_set, NULL, NULL, &timeout);  
        
        if(res == -1) {
          perror("Error on select"); // an error occured on select
        } else if (res == 0) {
          fprintf(stderr, "Timeout on data socket connection\n");; // a timeout occured on select
				} else {
          
          int data_fd;
          // wait for connection
          if ((data_fd = accept(data_socket.fd, (struct sockaddr *)(&data_socket.address), &data_socket.len)) < 0) {
            perror("Can't accept connection");
            exit(EXIT_FAILURE);
          }
          
          // retrieve or send file
          if (!strcmp(command, "PUT")) {
            putFile(data_fd, arg1);
          } else {
            getFile(data_fd, arg1);
          }
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
