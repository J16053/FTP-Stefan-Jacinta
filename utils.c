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
    perror("Error: ");
    return EXIT_FAILURE;
  } else {
    return EXIT_SUCCESS;
  }
}
