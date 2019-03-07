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
    fprintf(stderr, "Can't bind socket\n");
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
  // server_request = true;
  // FILE * file = fopen(arg1, "r");
  // if (file == NULL) {
  //   perror("Error opening file");
  // } else {
  //   write(sockfd, buf, strlen(buf));
  //   int data_sock_fd;
  //   if ((data_sock_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
  //     printf("Can't open socket\n");
  //     exit(EXIT_FAILURE);
  //   }
  //   int connected;
  //   if ((connected = connect(data_sock_fd, (struct sockaddr *)&addr, sizeof(addr))) < 0) {
  //     printf("Can't connect\n");
  //     exit(EXIT_FAILURE);
  //   } else if (connected == 0) {
  //     char buffer[MAX_BUF];
  //     while(fgets(buffer, sizeof(buffer), file) != NULL) {
  //       if (write(data_sock_fd, buffer, sizeof(buffer)) < 0) {
  //         perror("Error writing to socket");
  //       }
  //     }
  //     if (ferror(file)) {
  //       perror("Error");
  //     }
  //     fclose(file);
  //     close(data_sock_fd);
  //   }
  // }
  return EXIT_FAILURE;
}

int getFile(int data_fd, const char *file_name) {
  // FILE * file = fopen(arg1, "w");
  // if (file == NULL) {
  //   strcpy(buf, "Error opening file for writing"); // include code here
  // } else {
  //   // accept incoming connection request: but will this break if another client tries to connect at the same time?
  //   if ((accepted_socket = accept(master_socket.fd, (struct sockaddr *)(&client_addr), &master_socket.len)) < 0) {
  //     fprintf(stderr, "Can't accept connection\n");
  //     exit(EXIT_FAILURE);
  //   }
  //   int bytes_received = 1;
  //   char buffer[MAX_BUF];
  //   while (bytes_received > 0) {
  //     bytes_received = read(accepted_socket, buffer, sizeof(buffer));
  //     fprintf(file, "%s", buffer);
  //   }
  //   fclose(file);
  //   close(accepted_socket);
  //   strcpy(buf, "File uploaded to server");
    return EXIT_FAILURE;
}
