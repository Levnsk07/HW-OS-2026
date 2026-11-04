#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int main(void) {
  FILE *file;

  printf("Before setuid: UID=%lu, EUID=%lu\n",
         (unsigned long) getuid(), (unsigned long) geteuid());

  file = fopen("data.txt", "r+");
  if (file == NULL) {
    perror("fopen");
  } else {
    puts("File opened successfully");
    if (fclose(file) == EOF) {
      perror("fclose");
      return 1;
    }
  }

  if (setuid(getuid()) == -1) {
    perror("setuid");
    return 1;
  }

  printf("After setuid: UID=%lu, EUID=%lu\n",
         (unsigned long) getuid(), (unsigned long) geteuid());

  file = fopen("data.txt", "r+");
  if (file == NULL) {
    perror("fopen");
  } else {
    puts("File opened successfully");
    if (fclose(file) == EOF) {
      perror("fclose");
      return 1;
    }
  }

  return 0;
}
