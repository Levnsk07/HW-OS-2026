#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>

struct Line {
    off_t off;
    size_t len;
};

static void add_line(struct Line **lines, size_t *n, size_t *cap,
                     off_t off, size_t len) {
    if (*n == *cap) {
        size_t newcap = (*cap == 0) ? 16 : (*cap * 2);
        struct Line *p = realloc(*lines, newcap * sizeof **lines);
        if (p == NULL) {
            perror("realloc");
            exit(1);
        }
        *lines = p;
        *cap = newcap;
    }

    (*lines)[*n].off = off;
    (*lines)[*n].len = len;
    (*n)++;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s file\n", argv[0]);
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    struct Line *lines = NULL;
    size_t n = 0;
    size_t cap = 0;

    off_t start = 0;
    size_t len = 0;
    char ch;
    ssize_t r;

    while ((r = read(fd, &ch, 1)) == 1) {
        if (ch == '\n') {
            add_line(&lines, &n, &cap, start, len + 1);
            start = lseek(fd, 0, SEEK_CUR);
            len = 0;
        } else {
            len++;
        }
    }

    if (r < 0) {
        perror("read");
        close(fd);
        free(lines);
        return 1;
    }

    if (len > 0)
        add_line(&lines, &n, &cap, start, len);

    for (;;) {
        printf("line number (0 to exit): ");
        fflush(stdout);

        int num;
        if (scanf("%d", &num) != 1)
            break;

        if (num == 0)
            break;

        if (num < 1 || (size_t)num > n) {
            printf("No such line\n");
            continue;
        }

        size_t idx = (size_t)num - 1;

        if (lseek(fd, lines[idx].off, SEEK_SET) == (off_t)-1) {
            perror("lseek");
            continue;
        }

        char *buf = malloc(lines[idx].len + 1);
        if (buf == NULL) {
            perror("malloc");
            break;
        }

        ssize_t got = read(fd, buf, lines[idx].len);
        if (got < 0) {
            perror("read");
            free(buf);
            continue;
        }

        buf[got] = '\0';
        printf("%s", buf);
        if (got == 0 || buf[got - 1] != '\n')
            printf("\n");

        free(buf);
    }

    close(fd);
    free(lines);
    return 0;
}
