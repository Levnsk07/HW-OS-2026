#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>

typedef struct{
    off_t offset;
    int length;
} Table;

int main(void){
    int fd = open("text.txt", O_RDONLY);
    Table table[50];

    char buf[1024];
    off_t l_start = 0;
    int l_len = 0;
    int l_num = 0;
    ssize_t n;
    while ((n = read(fd, buf, sizeof(buf))) > 0) {
        for (ssize_t i = 0; i < n; i++) {
            if (buf[i] == '\n') {
                // конец строки
                table[l_num].length = l_len;
                table[l_num].offset = l_start;
                l_num++;
                l_len = 0;
                l_start = lseek(fd, 0L, SEEK_CUR) - (n - i - 1);
            } else {
                // обычный байт
                l_len++;
            }
        }
    }
    if (l_len > 0) {
        table[l_num].offset = l_start;
        table[l_num].length = l_len;
        l_num++;
    }

    int N;
    scanf("%d", &N);
    while (N != 0){
        lseek(fd, table[N - 1].offset, SEEK_SET);

        char *line = malloc(table[N - 1].length + 1);
        ssize_t got = read(fd, line, table[N - 1].length);
        line[got] = '\0';
        
        printf("%s\n", line);

        free(line);
        scanf("%d", &N);
    }

    return 0;
}