#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

static char *g_map = NULL;
static size_t g_size = 0;

static void on_alarm(int sig) {
    (void)sig;

    if (g_map != NULL && g_size > 0) {
        size_t off = 0;
        while (off < g_size) {
            ssize_t w = write(STDOUT_FILENO, g_map + off, g_size - off);
            if (w < 0)
                _exit(1);
            off += (size_t)w;
        }
    }

    _exit(0);
}

struct Line {
    size_t off;
    size_t len;
};

static void add_line(struct Line **lines, size_t *n, size_t *cap,
                     size_t off, size_t len) {
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

    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("fstat");
        close(fd);
        return 1;
    }

    size_t size = (size_t)st.st_size;
    char *data = NULL;

    if (size > 0) {
        data = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
        if (data == MAP_FAILED) {
            perror("mmap");
            close(fd);
            return 1;
        }
    }

    g_map = data;
    g_size = size;
    signal(SIGALRM, on_alarm);

    struct Line *lines = NULL;
    size_t n = 0;
    size_t cap = 0;

    for (size_t i = 0; i < size; ) {
        size_t start = i;

        while (i < size && data[i] != '\n')
            i++;

        if (i < size && data[i] == '\n') {
            add_line(&lines, &n, &cap, start, i - start + 1);
            i++;
        } else {
            if (i > start)
                add_line(&lines, &n, &cap, start, i - start);
        }
    }

    for (;;) {
        alarm(5);

        printf("line number (0 to exit): ");
        fflush(stdout);

        int num;
        if (scanf("%d", &num) != 1) {
            alarm(0);
            break;
        }

        alarm(0);

        if (num == 0)
            break;

        if (num < 1 || (size_t)num > n) {
            printf("No such line\n");
            continue;
        }

        size_t idx = (size_t)num - 1;

        fwrite(data + lines[idx].off, 1, lines[idx].len, stdout);

        if (lines[idx].len == 0 ||
            data[lines[idx].off + lines[idx].len - 1] != '\n')
            putchar('\n');
    }

    if (data != NULL)
        munmap(data, size);

    close(fd);
    free(lines);
    return 0;
}
