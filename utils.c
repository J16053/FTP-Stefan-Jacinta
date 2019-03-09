#include "utils.h"

int changeDir(const char *path) {
  int response;
  if (!path) {
    response = chdir("/");
  } else {
    response = chdir(path);
  }
  if (response == -1) {
    perror("Error");
    return EXIT_FAILURE;
  } else {
    return EXIT_SUCCESS;
  }
}

int connectSocket(const char *ip_address, int server_port) {
  
  int sockfd;
  struct sockaddr_in addr;

  if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    printf("Can't open socket\n");
    exit(EXIT_FAILURE);
  }

  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(server_port);
  if (!inet_aton(ip_address, &(addr.sin_addr))) {
    printf("Invalid address\n");
  }

  if (connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    printf("Can't connect\n");
    exit(EXIT_FAILURE);
  }
  
  return sockfd;
}

struct serverSocket serverSocketSetup(const int port, int reuse) {
  struct serverSocket sock;
  sock.len = sizeof(sock.address);
  
  sock.fd = socket(AF_INET, SOCK_STREAM, 0);
  if (sock.fd < 0) {
    fprintf(stderr, "Can't open socket\n");
    exit(EXIT_FAILURE);
  }

  setsockopt(sock.fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

  sock.address.sin_family = AF_INET;
  sock.address.sin_addr.s_addr = INADDR_ANY;
  sock.address.sin_port = htons(port);

  if (bind(sock.fd, (struct sockaddr *) &sock.address, sock.len)) {
    close(sock.fd);
    perror("Can't bind socket");
    exit(EXIT_FAILURE);
  }
  
  if (listen(sock.fd, 5) < 0) {
    close(sock.fd);
    fprintf(stderr, "Can't listen on socket\n");
    exit(EXIT_FAILURE);
  }
  return sock;
}

int putFile(int data_fd, const char *file_name) {
  
  // open file to send
  int file_fd = open(file_name, O_RDONLY);
  
  // check for errors
  if (file_fd < 0) {
    perror("Error opening file");
		return EXIT_FAILURE;
  } 
    
  // retrieve stats of file 
  struct stat st;  
  fstat(file_fd, &st);

  // send file
  int sent_bytes = sendfile(data_fd, file_fd, NULL, st.st_size);
  
  // check for errors in sending file
  if (sent_bytes != st.st_size) {
    perror("Failed to transfer file");
    return EXIT_FAILURE;
  } 
  
  // close the file and data socket
  close(file_fd);
  close(data_fd);
  return EXIT_SUCCESS;
}

int getFile(int data_fd, const char *file_name) {
  
  // open file for write
  FILE *file = fopen(file_name, "w");
  
  // check for errors when opening file to write
  if (file == NULL) {
    perror("Error opening file for writing");
    return EXIT_FAILURE;
  } 
  
  // wait for connection
  struct timeval timeout;
  timeout.tv_sec = 2;  // no specific reason for 2 second timeout
  timeout.tv_usec = 0;
  
  fd_set read_fd_set;
  FD_SET(data_fd, &read_fd_set);
  int res = select(data_fd+1, &read_fd_set, NULL, NULL, &timeout);  
  
  if (res == -1) {
    perror("Error on getFile"); // an error accured on select
    return EXIT_FAILURE;
  } else if (res == 0) {
    printf("Timeout on getFile\n"); // a timeout occured  on select
    return EXIT_FAILURE;
  }  
  
  // write data until nothing else is recieved
  char buffer[MAX_BUF];
  ssize_t bytes_received;
  while ((bytes_received = read(data_fd, buffer, sizeof(buffer))) > 0) {
    fwrite(buffer, sizeof(char), bytes_received, file);
  }
  
  // close the file and data socket
  fclose(file);
  close(data_fd);
  return EXIT_SUCCESS;
}
