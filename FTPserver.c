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
  int accepted_socket;
  struct sockaddr_in client_addr;
  fd_set read_fd_set;
  int maxfd, i;
  int port = 9999;
  char buf[MAX_BUF];
  
  struct client {
    int socket; // socket descriptor
    int status; // -1 = not connected, 1 = connected, 2 = username OK, 3 = logged in
    int user; // user index
    char work_dir[MAX_BUF]; 
  } clients[MAX_CLIENTS];

  
  // initialize array of clients
  callServerSystem("PWD", NULL, buf);  // default working directory of all clients is initial working directory of server
  for (i = 0; i < MAX_CLIENTS; i++) {
    clients[i].socket = UNINITIATED;
    clients[i].status = UNINITIATED;
    clients[i].user = UNINITIATED;
    strcpy(clients[i].work_dir, buf);
  }

  int reuse;
  struct serverSocket master_socket = serverSocketSetup(port, reuse);

  while (1) {

    // clear the socket set
    FD_ZERO(&read_fd_set);
 
    // add master_socket socket to file descriptor set
    FD_SET(master_socket.fd, &read_fd_set);
    maxfd = master_socket.fd;

    // add child sockets to set
    for (i = 0; i < MAX_CLIENTS; i++) {
        
        // get socket descriptor
        int client_socket = clients[i].socket;
        
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
    if (FD_ISSET(master_socket.fd, &read_fd_set)) {

      // a client tries to connect
      if ((accepted_socket = accept(master_socket.fd, (struct sockaddr *)(&client_addr), &master_socket.len)) < 0) {
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
        if (!strcmp(command, "PUT") || !strcmp(command, "GET")) {
  				// send OK      	
	  			strcpy(buf, "Ready to connect");
					send(clients[i].socket, buf, sizeof(buf), 0);

					// change directory to client working directory
          changeDir(clients[i].work_dir);
          
          // get socket information
          struct sockaddr_in address;
    	    socklen_t addrlen = sizeof(address);
          
          // store address of the client connected to the socket
    	    if (getpeername(clients[i].socket, (struct sockaddr *)&address, &addrlen) == -1) {
				perror("Error getting peer address");
			}

    	    char data_ip[MAX_BUF];
    	    inet_ntop(AF_INET, &address.sin_addr, data_ip, MAX_BUF);  // convert ip in address.sin_addr to char array
    	    int data_port = ntohs(address.sin_port) + 1; 
          
          // connect to socket
    	    int data_fd = connectSocket(data_ip, data_port);

          int result;
          if (!strcmp(command, "PUT")) {
            result = getFile(data_fd, arg1);   // flipped command on server side to recieve incoming file
          } else {
            result = putFile(data_fd, arg1);  // flipped command on server side to send file
          }
          if (result == EXIT_SUCCESS) {
            strcpy(buf, "File exchange successful");
          } else {
            strcpy(buf, "File exchange failed");
          }
        } else if (!strcmp(command, "LS") || !strcmp(command, "PWD")) {
          printf("[%d]Entered LS or PWD\n", i);
          changeDir(clients[i].work_dir);  // change server directory to client session working directory
          callServerSystem(command, arg1, buf);
        } else if (!strcmp(command, "CD")) {
          printf("[%d]Entered CD\n", i);
          changeDir(clients[i].work_dir);  // change server directory to client session working directory
          if (changeDir(arg1) == EXIT_FAILURE) {
            strcpy(buf, strerror(errno));
          } else { // update work_dir field of client
            callServerSystem("PWD", NULL, buf);
            strcpy(clients[i].work_dir, buf);
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
  
  // remove extra newline
  char *newlineChar = strrchr(response, '\n');
  if (newlineChar) {
    *newlineChar = '\0';  
  }

  // check for error
  if (pclose(fp) == -1) {
    strcpy(response, strerror(errno));
    return EXIT_FAILURE;
  } 

  // successful completion
  return EXIT_SUCCESS;
}
