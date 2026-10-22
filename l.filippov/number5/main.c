#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

struct Line {
    int start;
    int len;
};

int addLine(struct Line **table, int *n, int *capacity,
            int start, int len)
{
    if (*n == *capacity) {
        *capacity *= 2;

        struct Line *temp = realloc(
            *table, *capacity * sizeof(struct Line)
        );

        if (temp == NULL)
            return 1;

        *table = temp;
    }

    (*table)[*n].start = start;
    (*table)[*n].len = len;
    (*n)++;

    return 0;
}

int main()
{
    char ch;
    char buf[1024];
    int count;


    int start = 0;
    int num, pos = 0;


    int n = 0, capacity = 10;
    struct Line *table= malloc(capacity * sizeof(struct Line));

    int fd = open("file", O_RDONLY);
    if (fd == -1) {
        printf("ERR\n");
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
        printf("num %d: start = %d, len = %d\n",
               i + 1, table[i].start, table[i].len);
    }

    while (1) {
        printf("Print line number (0 - exit): ");

        scanf("%d", &num);

        if (num == 0)
            break;

        if (num < 1 || num > n) {
            printf("Такой строки нет\n");
            continue;
        }

        lseek(fd, table[num - 1].start, SEEK_SET);

        count = read(fd, buf, table[num - 1].len);
        buf[count] = '\0';

        printf("%s\n", buf);
    }

    close(fd);
    return 0;
}
