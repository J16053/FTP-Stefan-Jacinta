#ifndef UTILS_H
#define UTILS_H

#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/sendfile.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAX_BUF 1024
#define MAX_COMMAND_SIZE 4

struct serverSocket {
  int fd;
  struct sockaddr_in address;
  socklen_t len;
};

int changeDir(const char *path);
int connectSocket(const char *ip_address, int server_port);
struct serverSocket serverSocketSetup(const int port, int reuse);
int putFile(int data_fd, const char *file_name);
int getFile(int data_fd, const char *file_name);

#endif
