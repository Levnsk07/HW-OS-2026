#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>

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

  struct stat sb;
  if (fstat(fd, &sb) == -1) {
    perror ("fstat failed");
    close(fd);
    return EXIT_FAILURE;
  }

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

  char *file_data = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (file_data == MAP_FAILED) {
    perror("mmap failed");
    free(lines);
    close(fd);
    return EXIT_FAILURE;
  }

  for (off_t f = 0; f < sb.st_size; ++f) {
    if (file_data[f] == '\r') {
      len++;
      continue;
    }

    if (file_data[f] == '\n') {
      len++;

      line.offset = offset;
      line.len = len;

      if (append(&lines, &line_count, &capacity, &line) == EXIT_FAILURE) {
        free(lines);
        munmap(file_data, sb.st_size);
        close(fd);
        return EXIT_FAILURE;
      }

      offset += len;
      len = 0;
      continue;
    }

    len++;
  }
 
  if (len > 0) {
    line.offset = offset;
    line.len = len;

    if (append(&lines, &line_count, &capacity, &line) == EXIT_FAILURE) {
      free(lines);
      munmap(file_data, sb.st_size);
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
    munmap(file_data, sb.st_size);
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

      for (off_t offset = 0; offset < sb.st_size; ++offset) {
        putchar(file_data[offset]);
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

    for (off_t offset = cur->offset; offset < cur->offset + cur->len; ++offset) {
      putchar(file_data[offset]);
    }
  }

  munmap(file_data, sb.st_size);
  free(lines);
  close(fd);

  return EXIT_SUCCESS;
}
