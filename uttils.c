#include "uttils.h"
#include <stdio.h>
#include <stdlib.h>

void die(const char *message) {
  perror(message);
  exit(EXIT_FAILURE);
}
