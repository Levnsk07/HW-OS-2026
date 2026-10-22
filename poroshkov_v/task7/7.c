#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <stdint.h>
#include <inttypes.h>
#include <errno.h>
#include <ctype.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/mman.h>

typedef struct {
    off_t offset;
    off_t length;
} Line;

static int append_line(Line **lines, size_t *count, off_t offset, off_t length) {
    if (*count >= SIZE_MAX / sizeof(**lines) - 1) {
        errno = ENOMEM;
        return -1;
    }
    Line *grown = realloc(*lines, (*count + 1) * sizeof(**lines));
    if (grown == NULL)
        return -1;
    *lines = grown;
    (*lines)[*count].offset = offset;
    (*lines)[(*count)++].length = length;
    return 0;
}

static int parse_index(const char *text, size_t *index) {
    while (isspace((unsigned char)*text))
        ++text;
    if (*text == '-')
        return -1;
    char *end;
    errno = 0;
    uintmax_t value = strtoumax(text, &end, 10);
    if (end == text || errno == ERANGE || value > SIZE_MAX)
        return -1;
    while (isspace((unsigned char)*end))
        ++end;
    if (*end != '\0')
        return -1;
    *index = (size_t)value;
    return 0;
}

static volatile sig_atomic_t timed_out = 0;

static void alarm_handler(int sig) {
    (void)sig;
    timed_out = 1;
}

static int read_request(char *buffer, size_t size, int first_input) {
    sigset_t blocked, previous, wait_mask;
    sigemptyset(&blocked);
    sigaddset(&blocked, SIGALRM);
    if (first_input) {
        if (sigprocmask(SIG_BLOCK, &blocked, &previous) == -1)
            return -1;
        wait_mask = previous;
        sigdelset(&wait_mask, SIGALRM);
        timed_out = 0;
        alarm(5);
    }
    size_t used = 0;
    int result = 1, too_long = 0;
    for (;;) {
        if (first_input) {
            fd_set input;
            FD_ZERO(&input);
            FD_SET(STDIN_FILENO, &input);
            int ready = pselect(STDIN_FILENO + 1, &input, NULL, NULL, NULL, &wait_mask);
            if (timed_out) {
                result = 2;
                break;
            }
            if (ready == -1) {
                if (errno == EINTR)
                    continue;
                result = -1;
                break;
            }
        }
        int c = fgetc(stdin);
        if (c == EOF) {
            result = ferror(stdin) ? -1 : (used != 0 || too_long);
            break;
        }
        if (c == '\n')
            break;
        if (used + 1 < size)
            buffer[used++] = (char)c;
        else
            too_long = 1;
    }
    if (first_input) {
        alarm(0);
        if (sigprocmask(SIG_SETMASK, &previous, NULL) == -1)
            return -1;
    }
    buffer[used] = '\0';
    return result == 1 && too_long ? 3 : result;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Использование: %s файл\n", argv[0]);
        return EXIT_FAILURE;
    }
    struct sigaction sa;
    sa.sa_handler = alarm_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGALRM, &sa, NULL) == -1) {
        perror("sigaction");
        return EXIT_FAILURE;
    }
    if (setvbuf(stdin, NULL, _IONBF, 0) != 0) {
        fputs("Не удалось отключить буферизацию ввода\n", stderr);
        return EXIT_FAILURE;
    }
    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror("open");
        return EXIT_FAILURE;
    }
    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("fstat");
        close(fd);
        return EXIT_FAILURE;
    }
    if (!S_ISREG(st.st_mode) || st.st_size < 0 || (uintmax_t)st.st_size > SIZE_MAX) {
        fputs("Нужен обычный файл, помещающийся в адресное пространство\n", stderr);
        close(fd);
        return EXIT_FAILURE;
    }
    if (st.st_size == 0) {
        puts("Файл пуст");
        return close(fd) == -1 ? EXIT_FAILURE : EXIT_SUCCESS;
    }
    size_t size = (size_t)st.st_size;
    const char *file = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (file == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return EXIT_FAILURE;
    }
    int status = EXIT_SUCCESS;
    if (close(fd) == -1) {
        perror("close");
        munmap((void *)file, size);
        return EXIT_FAILURE;
    }
    Line *lines = NULL;
    size_t count = 0, start = 0;
    for (size_t i = 0; i < size; ++i) {
        if (file[i] == '\n') {
            if (append_line(&lines, &count, (off_t)start,
                            (off_t)(i + 1 - start)) == -1) {
                perror("realloc");
                status = EXIT_FAILURE;
                goto cleanup;
            }
            start = i + 1;
        }
    }
    if (start < size && append_line(&lines, &count, (off_t)start,
                                    (off_t)(size - start)) == -1) {
        perror("realloc");
        status = EXIT_FAILURE;
        goto cleanup;
    }
    int first_input = 1;
    for (;;) {
        printf("Введите номер строки (0 — выход): ");
        fflush(stdout);
        char input[128];
        int result = read_request(input, sizeof(input), first_input);
        first_input = 0;
        if (result == 0)
            break;
        if (result == -1) {
            perror("Ввод");
            status = EXIT_FAILURE;
            break;
        }
        if (result == 2) {
            puts("\nВремя вышло. Содержимое файла:");
            if (fwrite(file, 1, size, stdout) != size)
                status = EXIT_FAILURE;
            break;
        }
        size_t index;
        if (result == 3 || parse_index(input, &index) == -1) {
            fputs("Введите целое неотрицательное число\n", stderr);
            continue;
        }
        if (index == 0)
            break;
        if (index > count) {
            fputs("Нет такой строки\n", stderr);
            continue;
        }
        Line line = lines[index - 1];
        if (fwrite(file + line.offset, 1, (size_t)line.length, stdout) != (size_t)line.length) {
            perror("Вывод строки");
            status = EXIT_FAILURE;
            break;
        }
    }
cleanup:
    free(lines);
    if (munmap((void *)file, size) == -1) {
        perror("munmap");
        status = EXIT_FAILURE;
    }
    if (fflush(stdout) == EOF)
        status = EXIT_FAILURE;
    return status;
}
