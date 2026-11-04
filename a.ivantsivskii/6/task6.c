#define _XOPEN_SOURCE 600
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <signal.h>

static int fd;
static char dumpbuf[4096];

static void on_alarm(int sig) {
    ssize_t n;
    ssize_t w;
    size_t done;

    (void)sig;

    if (lseek(fd, 0L, SEEK_SET) == (off_t)-1) {
        _exit(1);
    }

    while ((n = read(fd, dumpbuf, sizeof dumpbuf)) > 0) {
        done = 0;
        while (done < (size_t)n) {
            w = write(STDOUT_FILENO, dumpbuf + done, (size_t)n - done);
            if (w <= 0) {
                _exit(1);
            }
            done += (size_t)w;
        }
    }

    if (n < 0) {
        _exit(1);
    }

    _exit(0);
}

static int add_line(off_t **offs, off_t **lens, size_t *count, size_t *cap, off_t off, off_t len) {
    size_t newcap;
    off_t *tmp;

    if (*count == *cap) {
        newcap = *cap ? *cap * 2 : 128;

        tmp = realloc(*offs, newcap * sizeof *tmp);
        if (tmp == NULL) {
            return -1;
        }
        *offs = tmp;

        tmp = realloc(*lens, newcap * sizeof *tmp);
        if (tmp == NULL) {
            return -1;
        }
        *lens = tmp;

        *cap = newcap;
    }

    (*offs)[*count] = off;
    (*lens)[*count] = len;
    (*count)++;

    return 0;
}

int main(int argc, char **argv) {
    char ch;
    ssize_t r;
    off_t start;
    off_t pos;
    off_t len;
    off_t *offs = NULL;
    off_t *lens = NULL;
    size_t count = 0;
    size_t cap = 0;
    int num;
    int rc;
    int c;
    size_t idx;
    char *line;
    size_t want;
    size_t total;
    struct sigaction sa;

    if (argc != 2) {
        return 1;
    }

    fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        return 1;
    }

    start = 0;

    while ((r = read(fd, &ch, 1)) == 1) {
        if (ch == '\n') {
            pos = lseek(fd, 0L, 1);
            if (pos == (off_t)-1) {
                free(offs);
                free(lens);
                close(fd);
                return 1;
            }

            len = pos - start - 1;

            if (add_line(&offs, &lens, &count, &cap, start, len) != 0) {
                free(offs);
                free(lens);
                close(fd);
                return 1;
            }

            start = pos;
        }
    }

    if (r == -1) {
        free(offs);
        free(lens);
        close(fd);
        return 1;
    }

    pos = lseek(fd, 0L, 1);
    if (pos == (off_t)-1) {
        free(offs);
        free(lens);
        close(fd);
        return 1;
    }

    if (pos > start) {
        len = pos - start;

        if (add_line(&offs, &lens, &count, &cap, start, len) != 0) {
            free(offs);
            free(lens);
            close(fd);
            return 1;
        }
    }

    sa.sa_handler = on_alarm;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGALRM, &sa, NULL) == -1) {
        free(offs);
        free(lens);
        close(fd);
        return 1;
    }

    for (;;) {
        printf("line: ");
        fflush(stdout);

        alarm(5);

        rc = scanf("%d", &num);

        if (rc != 1) {
            while ((c = getchar()) != '\n' && c != EOF) {
            }

            alarm(0);

            if (feof(stdin)) {
                break;
            }

            printf("invalid input\n");
            continue;
        }

        alarm(0);

        if (num == 0) {
            break;
        }

        if (num < 1 || (size_t)num > count) {
            printf("invalid line\n");
            continue;
        }

        idx = (size_t)num - 1;

        if (lseek(fd, offs[idx], 0) == (off_t)-1) {
            printf("error\n");
            continue;
        }

        want = (size_t)lens[idx];

        line = malloc(want + 1);
        if (line == NULL) {
            printf("error\n");
            continue;
        }

        total = 0;

        while (total < want) {
            r = read(fd, line + total, want - total);
            if (r <= 0) {
                break;
            }
            total += (size_t)r;
        }

        line[total] = '\0';

        printf("%s\n", line);

        free(line);
    }

    free(offs);
    free(lens);
    close(fd);

    return 0;
}