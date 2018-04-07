#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// #define DEBUG

#define VAR_MEM_START 16
#define BUFFER_SIZE 256

typedef enum bool bool;
enum bool {
  false, true
};

typedef struct st_node_t st_node_t;
struct st_node_t {
  char *sym;
  int val;
  st_node_t *next;
};

typedef struct st_t st_t;
struct st_t {
  st_node_t *head;
  st_node_t *tail;
  int varloc;
};

st_t *st_new();
void st_free(st_t *table);
void st_add(st_t *st, char *sym, int val);
int st_get(st_t *st, char *sym);

int read_address_inst(char *buffer, st_t *table);
int read_compute_inst(char *buffer, st_t *table);

void write_inst(FILE *file, int inst);
void preprocess(char *buffer);
int find(char *str, char c);
bool matches(char *str, int start, char *pat);

int main(int argc, char **argv) {
  if (argc < 3) {
    printf("Usage: ./assembler file.asm file.hack\n");
    return 0;
  }

  char *inname = argv[1];
  char *outname = argv[2];
  FILE *infile = fopen(inname, "r");
  FILE *outfile = fopen(outname, "w");
  st_t *table = st_new();
  char buffer[BUFFER_SIZE];
  int lineno = 0;

  // First Pass
  while (fgets(buffer, BUFFER_SIZE, infile) != NULL) {
    preprocess(buffer);
    if (buffer[0] == '\0') {
      continue;
    }
    if (buffer[0] == '(') {
      int end;
      for (end = 0; end < BUFFER_SIZE; end ++) {
        if (buffer[end] == ')') {
          buffer[end] = '\0';
          break;
        }
      }
      st_add(table, buffer + 1, lineno);
    } else {
      lineno++;
    }
  }

  fseek(infile, 0, SEEK_SET);
  lineno = 0;

  // Second Pass
  while (fgets(buffer, BUFFER_SIZE, infile) != NULL) {
    preprocess(buffer);
    if (buffer[0] == '\0') {
      continue;
    }
    if (buffer[0] == '(') {
      continue;
    }
    int inst = 0;
    if (buffer[0] == '@') {
      inst = read_address_inst(buffer, table);
    } else {
      inst = read_compute_inst(buffer, table);
    }
    write_inst(outfile, inst);
    lineno++;
  }

  st_free(table);
  fclose(infile);
  fclose(outfile);

  return 0;
}

st_t *st_new() {
  st_t *table = malloc(sizeof(st_t));

  table->head = NULL;
  table->tail = NULL;
  table->varloc = VAR_MEM_START;

  st_add(table, "SP", 0);
  st_add(table, "LCL", 1);
  st_add(table, "ARG", 2);
  st_add(table, "THIS", 3);
  st_add(table, "THAT", 4);
  st_add(table, "R0", 0);
  st_add(table, "R1", 1);
  st_add(table, "R2", 2);
  st_add(table, "R3", 3);
  st_add(table, "R4", 4);
  st_add(table, "R5", 5);
  st_add(table, "R6", 6);
  st_add(table, "R7", 7);
  st_add(table, "R8", 8);
  st_add(table, "R9", 9);
  st_add(table, "R10", 10);
  st_add(table, "R11", 11);
  st_add(table, "R12", 12);
  st_add(table, "R13", 13);
  st_add(table, "R14", 14);
  st_add(table, "R15", 15);
  st_add(table, "SCREEN", 16384);
  st_add(table, "LCL", 24576);

  return table;
}

void st_free(st_t *table) {
  st_node_t *n = table->head;
  st_node_t *p = NULL;
  while (n != NULL) {
    p = n;
    n = n->next;
    free(p->sym);
    free(p);
  }
  free(table);
}

void st_add(st_t *st, char *sym, int val) {
  st_node_t *n = malloc(sizeof(st_node_t));

  // We don't know if the value being passed in is stack or heap allocated.
  // Copy it to the heap for simple clean up later in the free function.
  char *sym_copy = malloc(sizeof(char) * (strlen(sym) + 1));
  strcpy(sym_copy, sym);

#ifdef DEBUG
  printf("Added sym '%s' with value: %d\n", sym_copy, val);
#endif

  n->sym = sym_copy;
  n->val = val;
  n->next = NULL;

  if (st->head == NULL && st->tail == NULL) {
    st->head = n;
    st->tail = n;
  } else {
    st->tail->next = n;
    st->tail = n;
  }
}

int st_get(st_t *st, char *sym) {
  st_node_t *n = st->head;
  while (n != NULL) {
    if (strcmp(n->sym, sym) == 0) {
      return n->val;
    }
    n = n->next;
  }
  return -1;
}

// TODO: clean this up.
// TODO: create more utility functions.
// TODO: organize utility functions into sections.
int read_address_inst(char *buffer, st_t *table) {
  int result = 0;
  if (buffer[1] >= '0' && buffer[1] <= '9') {
    int end = strlen(buffer);
    for (int i = 1; i < end; i++) {
      result += buffer[i] - '0';
      result *= 10;
    }
    result /= 10;
  } else {
    // TODO: clean this up and move this into the st_t logic.
    result = st_get(table, buffer + 1);
    if (result == -1) {
      st_add(table, buffer + 1, table->varloc++);
    }
    result = st_get(table, buffer + 1);
  }
#ifdef DEBUG
  printf("Parsed address inst: %d\n", result);
#endif
  return result;
}

int read_compute_inst(char *buffer, st_t *table) {
  int destEnd = find(buffer, '=');
  int compEnd = find(buffer, ';');
  int dest = 0;
  int jump = 0;
  int comp = 0;
  if (destEnd != -1) {
    // Order is important here given matching algorithm.
    if (matches(buffer, 0, "AMD")) {
      dest = 0b111;
    } else if (matches(buffer, 0, "MD")) {
      dest = 0b011;
    } else if (matches(buffer, 0, "AM")) {
      dest = 0b101;
    } else if (matches(buffer, 0, "AD")) {
      dest = 0b110;
    } else if (matches(buffer, 0, "M")) {
      dest = 0b001;
    } else if (matches(buffer, 0, "D")) {
      dest = 0b010;
    } else if (matches(buffer, 0, "A")) {
      dest = 0b100;
    } else {
      dest = 0b000;
    }
  }
  if (compEnd != -1) {
    int jumpStart = compEnd + 1;
    if (matches(buffer, jumpStart, "JGT")) {
      jump = 0b001;
    } else if (matches(buffer, jumpStart, "JEQ")) {
      jump = 0b010;
    } else if (matches(buffer, jumpStart, "JGE")) {
      jump = 0b011;
    } else if (matches(buffer, jumpStart, "JLT")) {
      jump = 0b100;
    } else if (matches(buffer, jumpStart, "JNE")) {
      jump = 0b101;
    } else if (matches(buffer, jumpStart, "JLE")) {
      jump = 0b110;
    } else if (matches(buffer, jumpStart, "JMP")) {
      jump = 0b111;
    } else {
      jump = 0b000;
    }
  }
  int compStart = 0;
  if (destEnd > -1) {
    compStart = destEnd + 1;
  }
  // Order is important below due to matching algorithm and wildcard.
  if (matches(buffer, compStart, "D+1")) {
    comp = 0b011111;
  } else if (matches(buffer, compStart, "*+1")) {
    comp = 0b110111;
  } else if (matches(buffer, compStart, "D-1")) {
    comp = 0b001110;
  } else if (matches(buffer, compStart, "*-1")) {
    comp = 0b110010;
  } else if (matches(buffer, compStart, "D+*")) {
    comp = 0b000010;
  } else if (matches(buffer, compStart, "D-*")) {
    comp = 0b010011;
  } else if (matches(buffer, compStart, "*-D")) {
    comp = 0b000111;
  } else if (matches(buffer, compStart, "D&*")) {
    comp = 0b000000;
  } else if (matches(buffer, compStart, "D|*")) {
    comp = 0b010101;
  } else if (matches(buffer, compStart, "0")) {
    comp = 0b101010;
  } else if (matches(buffer, compStart, "1")) {
    comp = 0b111111;
  } else if (matches(buffer, compStart, "-1")) {
    comp = 0b111010;
  } else if (matches(buffer, compStart, "!D")) {
    comp = 0b001101;
  } else if (matches(buffer, compStart, "!*")) {
    comp = 0b110001;
  } else if (matches(buffer, compStart, "-D")) {
    comp = 0b001111;
  } else if (matches(buffer, compStart, "-*")) {
    comp = 0b110011;
  } else if (matches(buffer, compStart, "D")) {
    comp = 0b001100;
  } else if (matches(buffer, compStart, "*")) {
    comp = 0b110000;
  } else {
    // TODO: make error case
    comp = 0b000000;
  }

  int inst = 0b0000000000000000;
  inst = inst | 0b1110000000000000;
  int compHasM = find(buffer + compStart, 'M');
  if ((compHasM > -1 && compEnd > -1 && compHasM < compEnd) || (compHasM > -1 && compEnd == -1)) {
    inst = inst | 0b0001000000000000;
  }
  inst = inst | (comp << 6);
  inst = inst | (dest << 3);
  inst = inst | jump;

  return inst;
}

void write_inst(FILE *file, int inst) {
  int mask = 1 << 16;
  for (int i = 0; i < 16; i++) {
    mask = mask >> 1;
    if ((inst & mask) > 0) {
      fprintf(file, "1");
    } else {
      fprintf(file, "0");
    }
  }
  fprintf(file, "\n");
}

void preprocess(char *buffer) {
  char temp[BUFFER_SIZE];
  int temp_i = 0;
  for (int i = 0; i < BUFFER_SIZE - 1; i++) {
    if (buffer[i] == '/' && buffer[i + 1] == '/') {
      break;
    }
    if (buffer[i] == '\0') {
      break;
    }
    if (buffer[i] == ' ') {
      continue;
    }
    if (buffer[i] == '\n') {
      continue;
    }
    if (buffer[i] == '\t') {
      continue;
    }
    temp[temp_i++] = buffer[i];
  }
  for (int i = 0; i < temp_i; i++) {
    buffer[i] = temp[i];
  }
  buffer[temp_i] = '\0';
}

int find(char *str, char c) {
  int end = strlen(str);
  for (int i = 0; i < end; i++) {
    if (str[i] == c) {
      return i;
    }
  }
  return -1;
}

bool matches(char *str, int start, char *pat) {
  int len = strlen(pat);
  for (int i = 0; i < len; i++) {
    if (pat[i] == '*') {
      continue;
    }
    if (str[start + i] != pat[i]) {
      return false;
    }
  }
  return true;
}
