#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#define BUFFER_SIZE 256

char *path_append(char *base, char *end) {
  int base_len = strlen(base);
  int end_len = strlen(end);
  int needsSlash = 0;
  if (base[base_len - 1] != '/') needsSlash = 1;
  char *result = malloc(sizeof(char) * (base_len + end_len + needsSlash));
  strcpy(result, base);
  if (needsSlash) {
    result[base_len] = '/';
  }
  strcpy(result + base_len + needsSlash, end);
  return result;
}

void preprocess(char *line) {
  char buffer[BUFFER_SIZE];
  int buffer_i = 0;

  int whitespace = 0;
  for (int i = 0; i < BUFFER_SIZE; i++) {
    if (line[i] == '\0') break;
    if (line[i] == '\n') continue;
    if (line[i] == '\r') continue;
    if (line[i] == '\t') continue;
    if (line[i] == '/' && line[i + 1] == '/') break;
    if (line[i] == ' ') {
      if (whitespace == 0) {
        buffer[buffer_i] = ' ';
        whitespace++;
        buffer_i++;
      }
      continue;
    }
    whitespace = 0;
    buffer[buffer_i++] = line[i];
  }

  if (buffer_i > 0) {
    strcpy(line, buffer);
  }
  line[buffer_i] = '\0';
}

void split(int *splits, char *line) {
  int line_len = strlen(line);
  int split_i = 0;
  for (int i = 0; i < line_len; i++) {
    if (line[i] == ' ') {
      splits[split_i++] = i;
    }
  }
  splits[split_i] = line_len;
}

int parse_integer(char *arg) {
  int r = 0;
  int i = 0;
  int d;
  int sign = 1;
  if (arg[0] == '-') {
    sign = -1;
    i++;
  }
  while (1) {
    d = arg[i] - '0';
    if (!(d >= 0 && d <= 10)) break;
    r *= 10;
    r += d;
    i++;
  }
  return sign * r;
}

int parse_segment_address(char *arg) {
  if (strncmp(arg, "constant", 8) == 0) return -1;
  if (strncmp(arg, "local", 5) == 0) return 1;
  if (strncmp(arg, "argument", 8) == 0) return 2;
  if (strncmp(arg, "this", 4) == 0) return 3;
  if (strncmp(arg, "that", 4) == 0) return 4;
  /* if (strncmp(arg, "static", 6) == 0) return -1; */
  /* if (strncmp(arg, "pointer", 7) == 0) return -1; */
  /* if (strncmp(arg, "temp", 4) == 0) return -1; */
  return -1;
}

void assemble_line(char *line, FILE *output_file) {
  int splits[4] = {-1, -1, -1, -1};
  split(splits, line);
  char *arg1 = line + (splits[0] + 1);
  char *arg2 = line + (splits[1] + 1);

  // TODO: we need to handle for the constant case where segment is -1.
  // In those cases, we should just load the value directly with an @ instruction

  if (strncmp(line, "push", 4) == 0) {
    int segment_address = parse_segment_address(arg1);
    int offset = parse_integer(arg2);
    fprintf(output_file, "// START %s\n", line);
    fprintf(output_file, "@%i\n", offset); // Load offset value
    fprintf(output_file, "D=A\n"); // Set offset to the data register
    fprintf(output_file, "@%i\n", segment_address); // Load segment index address
    fprintf(output_file, "D=D+M\n"); // Add segment index value to offset value
    fprintf(output_file, "A=D\n"); // Set address to segment location ^
    fprintf(output_file, "D=M\n"); // Set data to value of segment location
    fprintf(output_file, "@0\n"); // Load stack pointer index
    fprintf(output_file, "A=M\n"); // Set address to stack pointer value
    fprintf(output_file, "M=D\n"); // Set stack pointer address to previously loaded value
    fprintf(output_file, "@0\n"); // Load stack pointer address again
    fprintf(output_file, "M=M+1\n"); // Increase stack pointer by one
    fprintf(output_file, "// END %s\n", line);
  }

  if (strncmp(line, "pop", 3) == 0) {
    int segment_address = parse_segment_address(arg1);
    int offset = parse_integer(arg2);
    fprintf(output_file, "// START %s\n", line);
    fprintf(output_file, "@%i\n", offset); // Load offset value
    fprintf(output_file, "D=A\n"); // Set offset to the data register
    fprintf(output_file, "@%i\n", segment_address); // Load segment index address
    fprintf(output_file, "D=D+M\n"); // Add segment index value to offset value
    fprintf(output_file, "@13\n"); // Load general register 13
    fprintf(output_file, "M=D\n"); // Store segment with offset in general register
    fprintf(output_file, "@0\n"); // Load stack pointer
    fprintf(output_file, "A=M-1\n"); // Jump to address of stack pointer minus one
    fprintf(output_file, "D=M\n"); // Set the data register to that value
    fprintf(output_file, "@13\n"); // Load register 13 with out segment offset value
    fprintf(output_file, "A=M\n"); // Set address to the segment offset value
    fprintf(output_file, "M=D\n"); // Set the segment offset address to value from the stack
    fprintf(output_file, "@0\n"); // Load stack pointer address again
    fprintf(output_file, "M=M-1\n"); // Decrease stack pointer by one
    fprintf(output_file, "// END %s\n", line);
  }
}

void assemble_file(char *path, FILE *output_file) {
  char buffer[BUFFER_SIZE];
  FILE *input_file = fopen(path, "r");

  while (fgets(buffer, BUFFER_SIZE, input_file) != NULL) {
    preprocess(buffer);
    if (strlen(buffer) > 0) {
      assemble_line(buffer, output_file);
    }
  }

  fclose(input_file);
}

int main(int argc, char **argv) {
  if (argc < 3) {
    printf("Usage: VMTranslator [input | dir] output\n");
    return 1;
  }

  char *file_or_dir_path = argv[1];
  struct stat statbuf;
  stat(file_or_dir_path, &statbuf);

  FILE *output_file = fopen(argv[2], "w");

  if (S_ISDIR(statbuf.st_mode)) {
    DIR *dir = opendir(file_or_dir_path);
    struct dirent *dirent_buf;
    while ((dirent_buf = readdir(dir)) != NULL) {
      if (dirent_buf->d_name[0] == '.') continue;
      char *appended_path = path_append(file_or_dir_path, dirent_buf->d_name);
      assemble_file(appended_path, output_file);
      free(appended_path);
    }
    closedir(dir);
  } else {
    assemble_file(file_or_dir_path, output_file);
  }

  fclose(output_file);

  return 0;
}
