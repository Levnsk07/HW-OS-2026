#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Использование: %s команда [аргументы...]\n", argv[0]);
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
        execvp(argv[1], argv + 1);
        int error = errno;
        perror(argv[1]);
        _exit(error == ENOENT ? 127 : 126);
    }
    int status;
    while (waitpid(child, &status, 0) == -1) {
        if (errno == EINTR)
            continue;
        perror("waitpid");
        return EXIT_FAILURE;
    }
    int code;
    if (WIFEXITED(status)) {
        code = WEXITSTATUS(status);
        printf("Код завершения: %d\n", code);
    } else if (WIFSIGNALED(status)) {
        int sig = WTERMSIG(status);
        code = 128 + sig;
        printf("Процесс завершён сигналом %d\n", sig);
    } else {
        fputs("Неожиданное состояние дочернего процесса\n", stderr);
        return EXIT_FAILURE;
    }
    if (fflush(stdout) == EOF) {
        perror("fflush");
        return EXIT_FAILURE;
    }
    return code;
}
