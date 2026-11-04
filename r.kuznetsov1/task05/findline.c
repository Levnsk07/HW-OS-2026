#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

#define BUF_SIZE 4096

struct line_entry {
    off_t offset;
    size_t length;
};

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        perror(argv[1]);
        return 1;
    }

    struct line_entry *table = NULL;
    size_t table_size = 0;
    size_t table_cap = 0;

    off_t line_start = 0;
    char buf[BUF_SIZE];
    ssize_t n;

    while ((n = read(fd, buf, sizeof(buf))) > 0) {
        ssize_t i;
        for (i = 0; i < n; i++) {
            if (buf[i] == '\n') {
                off_t nl_offset = lseek(fd, 0L, SEEK_CUR) - (n - i);
                size_t len = (size_t)(nl_offset - line_start) + 1;

                if (table_size == table_cap) {
                    size_t new_cap = (table_cap == 0) ? 16 : table_cap * 2;
                    struct line_entry *tmp = realloc(table, new_cap * sizeof(*tmp));
                    if (tmp == NULL) {
                        perror("realloc");
                        free(table);
                        close(fd);
                        return 1;
                    }
                    table = tmp;
                    table_cap = new_cap;
                }
                table[table_size].offset = line_start;
                table[table_size].length = len;
                table_size++;

                line_start = nl_offset + 1;
            }
        }
    }
    if (n == -1) {
        perror("read");
        free(table);
        close(fd);
        return 1;
    }

    printf("Table of lines (%zu lines):\n", table_size);
    for (size_t i = 0; i < table_size; i++) {
        printf("  line %2zu: offset=%ld, length=%zu\n",
               i + 1, (long)table[i].offset, table[i].length);
    }
    printf("\n");

    char inbuf[64];
    for (;;) {
        printf("Enter line number (0 to quit): ");
        fflush(stdout);

        if (fgets(inbuf, sizeof(inbuf), stdin) == NULL) {
            break;
        }

        size_t l = strlen(inbuf);
        while (l > 0 && (inbuf[l - 1] == '\n' || inbuf[l - 1] == '\r' || inbuf[l - 1] == ' ')) {
            inbuf[--l] = '\0';
        }

        if (l == 0) {
            continue;
        }

        char *end = NULL;
        errno = 0;
        long num = strtol(inbuf, &end, 10);

        if (errno != 0 || end == inbuf || *end != '\0') {
            continue;
        }

        if (num == 0) {
            break;
        }

        if (num < 1 || (size_t)num > table_size) {
            continue;
        }

        struct line_entry *e = &table[num - 1];

        if (lseek(fd, e->offset, SEEK_SET) == (off_t)-1) {
            perror("lseek");
            continue;
        }

        char *line = malloc(e->length + 1);
        if (line == NULL) {
            perror("malloc");
            continue;
        }

        ssize_t got = read(fd, line, e->length);
        if (got == -1) {
            perror("read");
            free(line);
            continue;
        }
        line[got] = '\0';

        printf("%s", line);
        if (got == 0 || line[got - 1] != '\n') {
            printf("\n");
        }

        free(line);
    }

    free(table);
    close(fd);
    return 0;
}
