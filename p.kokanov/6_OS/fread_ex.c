#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>

#define BUFFER_SIZE 4096

typedef struct {
  off_t offset;
  size_t len;
} Line;

volatile sig_atomic_t timeout_hit = 0;

void on_alarm(int sig) {
  (void)sig;
  timeout_hit = 1;
}

int append(Line **lines, size_t *line_count, size_t *capacity, Line *line) {
  if ((*line_count) + 1 >= *capacity) {
    (*capacity) *= 2;
    Line *tmp = realloc(*lines, sizeof(Line) * (*capacity));

    if (tmp == NULL) {
      perror("realloc failed");
      free(*lines);
      return EXIT_FAILURE;
    }

    (*lines) = tmp;
  }

  (*lines)[*line_count].offset = line->offset;
  (*lines)[*line_count].len = line->len;
  (*line_count)++;

  return EXIT_SUCCESS;
}

int main() {
  int fd = open("file", O_RDONLY);

  if (fd == -1) {
    perror("open failed");
    return EXIT_FAILURE;
  }

  char buffer[BUFFER_SIZE];
  ssize_t bytes_read;
  Line *lines = malloc(sizeof(Line) * 2);
  size_t line_count = 0;
  size_t capacity = 2;

  if (lines == NULL) {
    perror("malloc failed");
    close(fd);
    return EXIT_FAILURE;
  }

  size_t len = 0;
  off_t offset = 0;
  Line line = {0, 0};

  while ((bytes_read = read(fd, buffer, BUFFER_SIZE)) > 0) {
    for (size_t f = 0; f < (size_t)bytes_read; ++f) {
      if (buffer[f] == '\r') {
        len++;
        continue;
      }

      if (buffer[f] == '\n') {
        len++;

        line.offset = offset;
        line.len = len;

        if (append(&lines, &line_count, &capacity, &line) == EXIT_FAILURE) {
          free(lines);
          close(fd);
          return EXIT_FAILURE;
        }

        offset += len;
        len = 0;
        continue;
      }

      len++;
    }
  }

  if (bytes_read == -1) {
    perror("read failed");
    free(lines);
    close(fd);
    return EXIT_FAILURE;
  }
 
  if (len > 0) {
    line.offset = offset;
    line.len = len;

    if (append(&lines, &line_count, &capacity, &line) == EXIT_FAILURE) {
      free(lines);
      close(fd);
      return EXIT_FAILURE;
    }
  }

  struct sigaction sa;
  sa.sa_handler = on_alarm;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;

  if (sigaction(SIGALRM, &sa, NULL) == -1) {
    perror("sigaction failed");
    free(lines);
    close(fd);
    return EXIT_FAILURE;
  }

  _Bool is_first = 1;
  int num;
  while (1) {
    printf("Line number (0 to exit): ");

    if (is_first) {
	printf("the whole file will be printed out in 5 seconds... ");
    	alarm(5);
    }

    int res = scanf("%d", &num);
	
    if (is_first) {
    	alarm(0);
	is_first = 0;
    }

    if (timeout_hit) {
      printf("\nTime is up! The whole file:\n");

      if (lseek(fd, 0, SEEK_SET) == -1) {
        perror("lseek failed");
        break;
      }

      while ((bytes_read = read(fd, buffer, BUFFER_SIZE)) > 0) {
        write(STDOUT_FILENO, buffer, bytes_read);
      }

      if (bytes_read == -1) {
        perror("read failed");
      }

      break;
    }

    if (res != 1) {
      printf("No number was inputed!\n");
      
      int c;
      while ((c = getchar()) != '\n' && c != EOF);

      continue;
    }

    if (num == 0) {
      break;
    }

    if (num < 1 || (size_t)num > line_count) {
      printf("There is no such line!\n");
      continue;
    }

    Line *cur = &lines[num - 1];

    if (lseek(fd, cur->offset, SEEK_SET) == -1) {
      perror("lseek failed");
      break;
    }

    ssize_t line_str = read(fd, buffer, cur->len);

    if (line_str == -1) {
      perror("read failed");
      break;
    }

    write(STDOUT_FILENO, buffer, line_str);
  }

  free(lines);
  close(fd);

  return EXIT_SUCCESS;
}
