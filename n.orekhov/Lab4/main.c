#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "array.h"

char *read_line(FILE *stream) {
  size_t capacity = 64;
  size_t len = 0;
  char *buf = malloc(capacity);
  if (!buf) return NULL;

  while (1) {
    if (fgets(buf + len, (int)(capacity - len), stream) == NULL) {
      if (len == 0) {
        free(buf);
        return NULL;
      }
      break;
    }

    len += strlen(buf + len);

    if (buf[len - 1] == '\n') {
      break;
    }

    if (len + 1 >= capacity) {
      size_t new_capacity = capacity * 2;
      char *new_buf = realloc(buf, new_capacity);
      if (!new_buf) {
        free(buf);
        return NULL;
      }
      buf = new_buf;
      capacity = new_capacity;
    }
  }

  return buf;
}

int main() {
  FILE* input = stdin;
  struct Node* str_arr = NULL;

  while (1) {
    char* line = read_line(input);
    if (line == NULL) {
      printf("Error");
      break;
    }

    if (line[0] == '.') {
      free(line);
      break;
    }

    str_arr = append_node(str_arr, line);
    free(line);
  }

  for (struct Node* cur = str_arr; cur != NULL; cur = cur->next) {
    printf("%s", cur->data);
  }

  str_arr = free_list(str_arr);

  scanf("%*[^\n]");
  return 0;
}
