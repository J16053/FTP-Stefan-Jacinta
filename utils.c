#include "utils.h"

int changeDir(const char *input) {
  int num_chars = strlen(input);
  char command[num_chars];
  strncpy(command, input, num_chars);
  strtok(command, "\n");
  const char *sep = strchr(command, ' ');
  const char *path;
  if (!sep) {
    path = "/home";   // default to home directory
  } else {
    path = sep + 1;
  }
  if (chdir(path) == -1) {
    perror(strerror(errno));
    return EXIT_FAILURE;
  } else {
    return EXIT_SUCCESS;
  }
}

// returns index to first non-space character in str
size_t first_not_space(const char* str) {
  int i = 0;
  while (isspace(str[i])) { ++i; }
  return i;
}
