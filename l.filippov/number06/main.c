#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

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
    char ch;
    char buf[1024];
    int count;
    int start = 0;
    int num, pos = 0;
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

    while (read(fd, &ch, 1) > 0) {
        pos++;
        if (ch == '\n') {
            if (addLine(&table, &n, &capacity, start, pos - start - 1)) {
                printf("ERR\n");
                close(fd);
                free(table);
                return 1;
            }
            start = pos;
        }
    }

    if (pos > start) {
        if (addLine(&table, &n, &capacity, start, pos - start)) {
            printf("ERR\n");
            close(fd);
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
        lseek(fd, 0, SEEK_SET);
        while ((count = read(fd, buf, sizeof(buf))) > 0) {
            // write(STDOUT_FILENO, buf, count);
            printf("%s\n", buf);
        }
        printf("\n");

        return 0;
    }

    alarm(0);

    while (1) {
        if (res != 1 || (num == 0) || (num < 48) || (num > 57))
            break;

        if (num < 1 || num > n) {
            printf("Dont Exist\n");
        } else {
            lseek(fd, table[num - 1].start, SEEK_SET);
            count = read(fd, buf, table[num - 1].len);
            buf[count] = '\0';
            printf("%s\n", buf);
        }

        printf("Print line number (0 - exit): ");
        res = scanf("%d", &num);
    }
    return 0;
}
