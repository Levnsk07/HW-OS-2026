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

static int build_table(int fd, Line **lines, size_t *count) {
    off_t start = 0;
    char c;
    for (;;) {
        ssize_t n = read(fd, &c, 1);
        if (n == -1) {
            if (errno == EINTR)
                continue;
            return -1;
        }
        if (n == 0)
            break;
        if (c == '\n') {
            off_t end = lseek(fd, 0, SEEK_CUR);
            if (end == (off_t)-1 ||
                append_line(lines, count, start, end - start) == -1)
                return -1;
            start = end;
        }
    }
    off_t end = lseek(fd, 0, SEEK_CUR);
    if (end == (off_t)-1)
        return -1;
    if (end > start)
        return append_line(lines, count, start, end - start);
    return 0;
}

static int print_line(int fd, Line line) {
    if ((uintmax_t)line.length > SIZE_MAX - 1) {
        errno = ENOMEM;
        return -1;
    }
    size_t length = (size_t)line.length;
    char *buffer = malloc(length + 1);
    if (buffer == NULL)
        return -1;
    if (lseek(fd, line.offset, SEEK_SET) == (off_t)-1) {
        free(buffer);
        return -1;
    }
    size_t done = 0;
    while (done < length) {
        size_t chunk = length - done < 4096 ? length - done : 4096;
        ssize_t n = read(fd, buffer + done, chunk);
        if (n == -1 && errno == EINTR)
            continue;
        if (n <= 0) {
            if (n == 0)
                errno = EIO;
            free(buffer);
            return -1;
        }
        done += (size_t)n;
    }
    buffer[length] = '\0';
    int result = printf("%s", buffer) < 0 ? -1 : 0;
    free(buffer);
    return result;
}

static int print_file(int fd) {
    if (lseek(fd, 0, SEEK_SET) == (off_t)-1)
        return -1;
    char buffer[4096];
    for (;;) {
        ssize_t n = read(fd, buffer, sizeof(buffer));
        if (n == -1 && errno == EINTR)
            continue;
        if (n == -1)
            return -1;
        if (n == 0)
            return 0;
        if (fwrite(buffer, 1, (size_t)n, stdout) != (size_t)n)
            return -1;
    }
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
    Line *lines = NULL;
    size_t count = 0;
    int status = EXIT_SUCCESS;
    if (build_table(fd, &lines, &count) == -1) {
        perror("Построение таблицы");
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
            if (print_file(fd) == -1) {
                perror("Чтение файла");
                status = EXIT_FAILURE;
            }
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
        if (print_line(fd, lines[index - 1]) == -1) {
            perror("Чтение строки");
            status = EXIT_FAILURE;
            break;
        }
    }
cleanup:
    free(lines);
    if (close(fd) == -1) {
        perror("close");
        status = EXIT_FAILURE;
    }
    if (fflush(stdout) == EOF)
        status = EXIT_FAILURE;
    return status;
}
