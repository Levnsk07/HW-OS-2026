#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>

struct line_entry {
    off_t offset;
    size_t length;
};

static char *g_map = NULL;
static off_t g_file_size = 0;

static void on_alarm(int sig)
{
    (void)sig;
    if (g_map != NULL && g_file_size > 0) {
        fwrite(g_map, 1, (size_t)g_file_size, stdout);
        fflush(stdout);
    }
    _exit(0);
}

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

    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("fstat");
        close(fd);
        return 1;
    }

    if (st.st_size == 0) {
        fprintf(stderr, "File is empty\n");
        close(fd);
        return 1;
    }

    g_file_size = st.st_size;

    g_map = mmap(NULL, (size_t)st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (g_map == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }

    close(fd);

    struct line_entry *table = NULL;
    size_t table_size = 0;
    size_t table_cap = 0;
    off_t line_start = 0;
    off_t i;

    for (i = 0; i < st.st_size; i++) {
        if (g_map[i] == '\n') {
            size_t len = (size_t)(i - line_start) + 1;

            if (table_size == table_cap) {
                size_t new_cap = (table_cap == 0) ? 16 : table_cap * 2;
                struct line_entry *tmp = realloc(table, new_cap * sizeof(*tmp));
                if (tmp == NULL) {
                    perror("realloc");
                    free(table);
                    munmap(g_map, g_file_size);
                    return 1;
                }
                table = tmp;
                table_cap = new_cap;
            }
            table[table_size].offset = line_start;
            table[table_size].length = len;
            table_size++;
            line_start = i + 1;
        }
    }

    printf("Table of lines (%zu lines):\n", table_size);
    for (size_t j = 0; j < table_size; j++) {
        printf("  line %2zu: offset=%ld, length=%zu\n",
               j + 1, (long)table[j].offset, table[j].length);
    }
    printf("\n");

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = on_alarm;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGALRM, &sa, NULL) == -1) {
        perror("sigaction");
        free(table);
        munmap(g_map, g_file_size);
        return 1;
    }

    char inbuf[64];
    int first = 1;

    for (;;) {
        if (first) {
            printf("Enter line number (0 to quit, 5 sec timeout): ");
        } else {
            printf("Enter line number (0 to quit): ");
        }
        fflush(stdout);

        if (first) {
            alarm(5);
        }

        if (fgets(inbuf, sizeof(inbuf), stdin) == NULL) {
            if (first) {
                alarm(0);
            }
            break;
        }

        if (first) {
            alarm(0);
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

        first = 0;

        struct line_entry *e = &table[num - 1];

        fwrite(g_map + e->offset, 1, e->length, stdout);
        if (e->length == 0 || g_map[e->offset + e->length - 1] != '\n') {
            printf("\n");
        }
        fflush(stdout);
    }

    free(table);
    munmap(g_map, g_file_size);
    return 0;
}
