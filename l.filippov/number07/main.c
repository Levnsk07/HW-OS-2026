#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>

struct Line {
    int start;
    int len;
};

volatile sig_atomic_t timeout_flag = 0;

void handle_alarm(int signum) {
    timeout_flag = 1;
}

int addLine(struct Line **table, int *n, int *capacity, int start, int len) {
    if (*n == *capacity) {
        *capacity *= 2;
        struct Line *temp = realloc(*table, *capacity * sizeof(struct Line));
        if (temp == NULL)
            return 1;
        *table = temp;
    }
    (*table)[*n].start = start;
    (*table)[*n].len = len;
    (*n)++;
    return 0;
}

int main() {
    int num;
    int n = 0, capacity = 10;
    struct Line *table = malloc(capacity * sizeof(struct Line));

    struct sigaction sa;
    sa.sa_handler = handle_alarm;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGALRM, &sa, NULL);

    int fd = open("file", O_RDONLY);
    if (fd == -1) {
        printf("ERR\n");
        free(table);
        return 1;
    }

    struct stat sb;
    fstat(fd, &sb);
    if (sb.st_size == 0) {
        printf("Err");
        return 1;
    }

    char *file_mem = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    close(fd);

    int start = 0;
    for (int pos = 0; pos < sb.st_size; pos++) {
        if (file_mem[pos] == '\n') {
            if (addLine(&table, &n, &capacity, start, pos - start)) {
                printf("ERR\n");
                munmap(file_mem, sb.st_size);
                free(table);
                return 1;
            }
            start = pos + 1;
        }
    }

    if (start < sb.st_size) {
        if (addLine(&table, &n, &capacity, start, sb.st_size - start)) {
            printf("ERR\n");
            munmap(file_mem, sb.st_size);
            free(table);
            return 1;
        }
    }

    printf("Table\n");
    for (int i = 0; i < n; i++) {
        printf("num %d: start = %d, len = %d\n", i + 1, table[i].start, table[i].len);
    }

    alarm(5);
    printf("Print line number (0 - exit): ");
    fflush(stdout);

    int res = scanf("%d", &num);

    if (timeout_flag) {
        printf("\ntime out:\n");
        printf("%.*s\n", (int) sb.st_size, file_mem);
        munmap(file_mem, sb.st_size);
        free(table);
        return 0;
    }

    alarm(0);

    while (1) {
        if (res != 1 || num == 0)
            break;

        if (num < 1 || num > n || (num == 0) || (num < 48) || (num > 57)) {
            printf("nope\n");
        } else {
            printf("%.*s\n", table[num - 1].len, &file_mem[table[num - 1].start]);
        }

        printf("Print line number (0 - exit): ");
        res = scanf("%d", &num);
    }

    munmap(file_mem, sb.st_size);
    free(table);
    return 0;
}
