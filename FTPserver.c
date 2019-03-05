#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include "utils.h"

#define MAX_CLIENTS 30
#define NUM_USERS 4
#define UNINITIATED -1
#define CONNECTED 1
#define USER_OK 2
#define LOGGED_IN 3

struct user {
  char *username;
  char *password;
} USERS[NUM_USERS] = {{"JACINTA", "AWESOME"}, {"STEFAN", "SUPER"}, {"YASIR", "ZAKI"}, {"THOMAS", "POTSCH"}};

static int callServerSystem(const char *command, const char *options, char *response);

int main(int argc, char *argv[])
{
  int master_socket, accepted_socket, client_socket;
  struct sockaddr_in server_addr, client_addr;
  char buf[MAX_BUF];
  fd_set read_fd_set;
  int maxfd, i;
  int port = 9999;
  
  struct client {
    int socket; // socket descriptor
    int status; // -1 = not connected, 1 = connected, 2 = username OK, 3 = logged in
    int user; // user index
    char pwd[MAX_BUF]; // working directory of client
  } clients[MAX_CLIENTS];

  // initialize array of clients
  for (i = 0; i < MAX_CLIENTS; i++) {
    clients[i].socket = UNINITIATED;
    clients[i].status = UNINITIATED;
    clients[i].user = UNINITIATED;
    callServerSystem("PWD", NULL, buf);
    strcpy(clients[i].pwd, buf);  // set default working directory of each client to the place where the server starts
  }

  master_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (master_socket < 0) {
    fprintf(stderr, "Can't open socket\n");
    exit(EXIT_FAILURE);
  }

  int reuse = 1;
  setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int));

  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port);

  if (bind(master_socket, (struct sockaddr *) &server_addr, sizeof(server_addr))) {
    close(master_socket);
    fprintf(stderr, "Can't bind socket\n");
    exit(EXIT_FAILURE);
  }
  
  if (listen(master_socket, 5) < 0) {
    close(master_socket);
    fprintf(stderr, "Can't listen on socket\n");
    exit(EXIT_FAILURE);
  }

  while (1) {

    // clear the socket set
    FD_ZERO(&read_fd_set);
 
    // add master socket to file descriptor set
    FD_SET(master_socket, &read_fd_set);
    maxfd = master_socket;

    // add child sockets to set
    for (i = 0; i < MAX_CLIENTS; i++) {
        
        // get socket descriptor
        client_socket = clients[i].socket;
        
        // if socket descriptor is valid, then add it to read list
        if(client_socket > 0) {
          FD_SET(client_socket, &read_fd_set);
        }

        // add highest file descriptor number to maxfd
        if(client_socket > maxfd)
            maxfd = client_socket;
    }
    
    // wait for activity on one of the sockets
    select(maxfd+1, &read_fd_set, NULL, NULL, NULL);
    
    // check for activity on the master_socket (if so, then it must be an incoming request)
    if (FD_ISSET(master_socket, &read_fd_set)) {

      // a client tries to connect
      socklen_t len = sizeof(client_addr);
      if ((accepted_socket = accept(master_socket, (struct sockaddr *)(&client_addr), &len)) < 0) {
          fprintf(stderr, "Can't accept connection\n");
          exit(EXIT_FAILURE);
      }

      printf("Accepted connection from %s:%d\n", inet_ntoa(client_addr.sin_addr), client_addr.sin_port);

      // add new socket to array of clients
      for (i = 0; i < MAX_CLIENTS; i++) {
          
          // if position is empty, add it
          if (clients[i].socket < 0) {
              clients[i].socket = accepted_socket;
              clients[i].status = CONNECTED;
              printf("[%d]Adding client to list of sockets with socket\n", i);
              break;
          }
      }

      // new client can not be added, close socket
      if (i == MAX_CLIENTS) {
        printf("Too many connections\n");
        close(accepted_socket);
      }
    }

    // loop through all clients and check for activity on all client sockets
    for (i = 0; i < MAX_CLIENTS; i++) {

      // skip if array position is not used or inactive
      if (clients[i].socket < 0 || !FD_ISSET(clients[i].socket, &read_fd_set)) {
        continue;
      }

      memset(buf, 0, sizeof(buf)); //reset the buffer
      int num = recv(clients[i].socket, buf, MAX_BUF, 0); // read from socket
      
      // client closed the connection
      if (num == 0) {
        printf("[%d]Closing connection\n", i);
        close(clients[i].socket);
        FD_CLR(clients[i].socket, &read_fd_set); // clear the file descriptor set for client[i]
        clients[i].socket = UNINITIATED;
        clients[i].status = UNINITIATED;
        continue;
      } 
      // client sent a server request
      printf("[%d]Received: %s\n", i, buf);
      char input[MAX_BUF];
      strncpy(input, buf, sizeof(buf));
      
      strtok(input, "\n");
      char *command = strtok(input, " ");
      char *arg1 = strtok(NULL, " ");
      
      // printf("%s\n", input); // uncomment to view input
      if (!strcmp(command, "USER")) {
        printf("[%d]Entered USERNAME\n", i);
        // verify username
        if (clients[i].status == USER_OK) {
          clients[i].status = CONNECTED;
          clients[i].user = UNINITIATED;
        }
        for (int u = 0; u < NUM_USERS; u++) {
          if (!strcmp(arg1, USERS[u].username)) {
            printf("[%d]VALID USERNAME\n", i);
            clients[i].status = USER_OK;
            clients[i].user = u;
            strcpy(buf, "331 Username OK, need password");
            break;
          }
        }
        if (clients[i].status != USER_OK) {
          strcpy(buf, "430 Invalid username");
          printf("[%d]INVALID USERNAME\n", i);
        }
      } else if (!strcmp(command, "PASS")) {
        printf("[%d]Entered PASSWORD\n", i);
        // verify password
        if (clients[i].status == LOGGED_IN) {
          strcpy(buf, "431 User already logged in");
        } else if (clients[i].status != USER_OK) {
          strcpy(buf, "530 User authentication is pending");
        } else { // clients[i].status == USER_OK
          if (!strcmp(arg1, USERS[clients[i].user].password)) {
            clients[i].status = LOGGED_IN;
            printf("[%d]PASSWORD OK\n", i);
            strcpy(buf, "230 User logged in, proceed");
          } else {
            printf("[%d]INCORRECT PASSWORD\n", i);
            strcpy(buf, "430 Incorrect password");
          }
        }
      } else if (clients[i].status != LOGGED_IN) {
        if (clients[i].status == USER_OK) {
          strcpy(buf, "530 Password authentication is pending");
        } else {
          strcpy(buf, "530 User authentication is pending");
        }
      } else {
        if (!strcmp(command, "PUT")) {
          printf("[%d]Entered PUT\n", i);
          // PUT file arg1 onto server
          // for now echo the message back to client by not modifying buf
        } else if (!strcmp(command, "GET")) {
          printf("[%d]Entered GET\n", i);
          // GET file arg1 from server and send result back to client
          // for now echo the message back to client by not modifying buf
        } else if (!strcmp(command, "LS") || !strcmp(command, "PWD")) {
          printf("[%d]Entered LS or PWD\n", i);
          callServerSystem(command, arg1, buf);
        } else if (!strcmp(command, "CD")) {
          printf("[%d]Entered CD\n", i);
          if (changeDir(arg1) == EXIT_FAILURE) {
            strcpy(buf, strerror(errno));
          } else {
            strcpy(buf, "250 Changed directory");
          }
        } else {
          strcpy(buf, "502 Invalid FTP command");
        }
      }
      send(clients[i].socket, buf, sizeof(buf), 0);
    }
  }
}

static int callServerSystem(const char *command, const char *options, char *response) {

  // create shell command
  char shell_command[32];           // 32 is enough space for options
  if (!strcmp(command, "LS")) {
    strcpy(shell_command, "ls ");
  } else {
    strcpy(shell_command, "pwd ");
  }
  if (options) {
    strcat(shell_command, options);
  }

  // execute shell command
  FILE *fp = popen(shell_command, "r");
  if (!fp) {
    strcpy(response, strerror(errno));
    return EXIT_FAILURE;
  }

  // retrieve output and store in response
  strcpy(response, "");   // sets a null terminator
  char line[128];
  while (fgets(line, sizeof(line), fp)) {
    strcat(response, line);
  }

  // check for error
  if (pclose(fp) == -1) {
    strcpy(response, strerror(errno));
    return EXIT_FAILURE;
  } 

  // successful completion
  return EXIT_SUCCESS;
}
