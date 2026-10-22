#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Использование: %s файл\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (fflush(NULL) == EOF) {
        perror("fflush");
        return EXIT_FAILURE;
    }
    pid_t child = fork();
    if (child == -1) {
        perror("fork");
        return EXIT_FAILURE;
    }
    if (child == 0) {
        int fd = open(argv[1], O_RDONLY);
        if (fd == -1) {
            perror(argv[1]);
            _exit(EXIT_FAILURE);
        }
        if (dup2(fd, STDIN_FILENO) == -1) {
            perror("dup2");
            _exit(EXIT_FAILURE);
        }
        if (fd != STDIN_FILENO)
            close(fd);
        execlp("cat", "cat", (char *)NULL);
        perror("exec cat");
        _exit(127);
    }
    printf("Родитель: PID=%ld, дочерний процесс=%ld\n", (long)getpid(), (long)child);
    fflush(stdout);
    int status;
    while (waitpid(child, &status, 0) == -1) {
        if (errno == EINTR)
            continue;
        perror("waitpid");
        return EXIT_FAILURE;
    }
    printf("\nРодитель: дочерний процесс завершён. Это последняя строка.\n");
    if (fflush(stdout) == EOF) {
        perror("fflush");
        return EXIT_FAILURE;
    }
    if (!WIFEXITED(status) || WEXITSTATUS(status) != 0)
        return EXIT_FAILURE;
    return EXIT_SUCCESS;
}
