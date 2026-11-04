#define _XOPEN_SOURCE 600

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <signal.h>
#include <setjmp.h>

static sigjmp_buf timeout_jb;
static volatile sig_atomic_t can_jump;

static void on_alarm(int sig) {
    (void)sig;

    if (can_jump) {
        siglongjmp(timeout_jb, 1);
    }
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
    int fd;
    struct stat st;
    char *base;
    size_t size;
    size_t i;
    off_t start;
    off_t len;
    off_t *offs = NULL;
    off_t *lens = NULL;
    size_t count = 0;
    size_t cap = 0;
    int num;
    int rc;
    int ch;
    size_t idx;
    struct sigaction sa;

    if (argc != 2) {
        return 1;
    }

    fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        return 1;
    }

    if (fstat(fd, &st) == -1) {
        close(fd);
        return 1;
    }

    size = (size_t)st.st_size;
    base = NULL;

    if (size > 0) {
        base = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
        if (base == MAP_FAILED) {
            close(fd);
            return 1;
        }
    }

    start = 0;

    for (i = 0; i < size; i++) {
        if (base[i] == '\n') {
            len = (off_t)i - start;

            if (add_line(&offs, &lens, &count, &cap, start, len) != 0) {
                free(offs);
                free(lens);
                if (base != NULL) {
                    munmap(base, size);
                }
                close(fd);
                return 1;
            }

            start = (off_t)i + 1;
        }
    }

    if ((off_t)size > start) {
        len = (off_t)size - start;

        if (add_line(&offs, &lens, &count, &cap, start, len) != 0) {
            free(offs);
            free(lens);
            if (base != NULL) {
                munmap(base, size);
            }
            close(fd);
            return 1;
        }
    }

    for (idx = 0; idx < count; idx++) {
        printf("%lu %ld %ld\n",
               (unsigned long)(idx + 1),
               (long)offs[idx],
               (long)lens[idx]);
    }

    fflush(stdout);

    sa.sa_handler = on_alarm;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    if (sigaction(SIGALRM, &sa, NULL) == -1) {
        free(offs);
        free(lens);
        if (base != NULL) {
            munmap(base, size);
        }
        close(fd);
        return 1;
    }

    for (;;) {
        printf("line: ");
        fflush(stdout);

        if (sigsetjmp(timeout_jb, 1) != 0) {
            alarm(0);
            can_jump = 0;

            if (size > 0) {
                fwrite(base, 1, size, stdout);
            }

            fflush(stdout);
            _exit(0);
        }

        can_jump = 1;
        alarm(5);

        rc = scanf("%d", &num);

        if (rc != 1) {
            while ((ch = getchar()) != '\n' && ch != EOF) {
            }

            alarm(0);
            can_jump = 0;

            if (feof(stdin)) {
                break;
            }

            printf("invalid input\n");
            continue;
        }

        alarm(0);
        can_jump = 0;

        if (num == 0) {
            break;
        }

        if (num < 1 || (size_t)num > count) {
            printf("invalid line\n");
            continue;
        }

        idx = (size_t)num - 1;

        printf("%.*s\n", (int)lens[idx], base + (size_t)offs[idx]);
    }

    free(offs);
    free(lens);

    if (base != NULL) {
        munmap(base, size);
    }

    close(fd);

    return 0;
}