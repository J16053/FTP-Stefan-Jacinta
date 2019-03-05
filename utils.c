#include "utils.h"

int changeDir(const char *path) {
  int response;
  if (!path) {
    response = chdir("/");
  } else {
    response = chdir(path);
  }
  if (response == -1) {
    perror("Error: ");
    return EXIT_FAILURE;
  } else {
    return EXIT_SUCCESS;
  }
}
