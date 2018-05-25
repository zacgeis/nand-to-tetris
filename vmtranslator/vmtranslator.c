#include <stdlib.h>
#include <stdio.h>

#define BUFFER_SIZE 256

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("Usage: VMTranslator [filename | dirname]\n");
    return 1;
  }

  char buffer[BUFFER_SIZE];
  FILE *file = fopen(argv[1], "r");

  while (fgets(buffer, BUFFER_SIZE, file) != NULL) {
    printf("%s", buffer);
  }

  return 1;
}
