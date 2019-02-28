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

#define MAXCLIENTS 30

int main(int argc, char * argv[])
{
  int master_socket, accepted_socket, client_socket;
  struct sockaddr_in server_addr, client_addr;
  char buf[1024];
  fd_set read_fd_set;
  int maxfd, i;
  int port = 9999;
  int clients[MAXCLIENTS][2];
  /* clients[i][0] contains socket descriptor value
   * clients[i][1] contains status as follows:
   * 0 = client not connected (should occur when clients[i][0] == -1)
   * 1 = connected with no USER
   * 2 = USER OK but no PASS
   * 3 = fully logged in (USER/PASS authenticated)
  */

  // initialize array of clients
  for (i = 0; i < MAXCLIENTS; i++) {
    clients[i][0] = -1;
    clients[i][1] = 0;
  }

  master_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (master_socket < 0) {
    printf("Can't open socket\n");
    exit(1);
  }

  int reuse = 1;
  setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int));

  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port);

  if (bind(master_socket, (struct sockaddr *) &server_addr, sizeof(server_addr))) {
    close(master_socket);
    printf("Can't bind socket\n");
    exit(1);
  }
  
  if (listen(master_socket, 5) < 0) {
    close(master_socket);
    printf("Can't listen on socket\n");
    exit(1);
  }

  while (1) {

    // clear the socket set
    FD_ZERO(&read_fd_set);
 
    // add master socket to file descriptor set
    FD_SET(master_socket, &read_fd_set);
    maxfd = master_socket;

    // add child sockets to set
    for (i = 0; i < MAXCLIENTS; i++) {
        
        // get socket descriptor
        client_socket = clients[i][0];
        
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
          printf("Can't accept connection\n");
          exit(1);
      }

      printf("Accepted connection from %s:%d\n", inet_ntoa(client_addr.sin_addr), client_addr.sin_port);

      // add new socket to array of clients
      for (i = 0; i < MAXCLIENTS; i++) {
          
          // if position is empty, add it
          if (clients[i][0] < 0) {
              clients[i][0] = accepted_socket;
              clients[i][1] = 1;
              printf("[%d]Adding client to list of sockets with socket\n", i);
              break;
          }
      }

      // new client can not be added, close socket
      if (i == MAXCLIENTS) {
        printf("Too many connections\n");
        close(accepted_socket);
      }
    }

    // loop through all clients and check for activity on all client sockets
    for (i = 0; i < MAXCLIENTS; i++) {

      // skip if array position is not used
      if (clients[i][0] < 0) {
        continue;
      }

      // check for activity
      if (FD_ISSET(clients[i][0], &read_fd_set)) {
        
        memset(buf, 0, sizeof(buf)); //reset the buffer
        int num = recv(clients[i][0], buf, 1024, 0); // read from socket
        
        // client closed the connection
        if (num == 0) {
          printf("[%d]Closing connection\n", i);
          close(clients[i][0]);
          FD_CLR(clients[i][0], &read_fd_set); // clear the file descriptor set for client[i]
          clients[i][0] = -1;
          clients[i][1] = 0;
        } else { // client sent a server request
          printf("[%d]Received: %s\n", i, buf);
          char input[1024];
          strncpy(input, buf, sizeof(buf));
          
          char * command = strtok(buf, " \n");
          char * arg1 = strtok(NULL, " ");
          
          // printf("%s\n", input); // uncomment to view input
          if (!strcmp(command, "USER")) {
            printf("Entered USERNAME\n");
            // verify username
            // send(clients[i][0], "331 Username OK, need password", 31, 0);
            // send(clients[i][0], "430 Invalid username", 21, 0);
          } else if (!strcmp(command, "PASS")) {
            printf("Entered PASSWORD\n");
            // verify password
          } else if (!strcmp(command, "PUT")) {
            // PUT file arg1 onto server
            printf("Entered PUT\n");
          } else if (!strcmp(command, "GET")) {
            // GET file arg1 from server and send result back to client
            printf("Entered GET\n");
          } else if (!strcmp(command, "CD") || !strcmp(command, "LS") || !strcmp(command, "PWD")) {
            // system call and send result back to client
          }

          send(clients[i][0], buf, num, 0); // echo the message back to client
        }
      }
    }
  }
}