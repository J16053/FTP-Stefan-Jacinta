#ifndef UTILS_H
#define UTILS_H

#include <ctype.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_BUF 1024
#define MAX_COMMAND_SIZE 4

int changeDir(const char *path);

#endif
