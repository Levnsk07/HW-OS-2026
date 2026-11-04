#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>

typedef struct{
    off_t offset;
    int length;
} Table;

int fd;
char *data;
size_t file_size;

void on_alarm(int sig){
    (void)sig;
    fwrite(data, 1, file_size, stdout);
    fflush(stdout);
    munmap(data, file_size);
    close(fd);
    exit(0);
}

int main(void){
    fd = open("text.txt", O_RDONLY);

    struct stat st;
    fstat(fd, &st);
    file_size = st.st_size;

    data = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (data == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    Table table[50];
    int l_num = 0;
    off_t l_start = 0;
    int l_len = 0;

    for (size_t i = 0; i < file_size; i++) {
        if (data[i] == '\n') {
            table[l_num].offset = l_start;
            table[l_num].length = l_len;
            l_num++;
            l_len = 0;
            l_start = i + 1;
        } else {
            l_len++;
        }
    }
    if (l_len > 0) {
        table[l_num].offset = l_start;
        table[l_num].length = l_len;
        l_num++;
    }

    signal(SIGALRM, on_alarm);

    int N;
    alarm(5);
    scanf("%d", &N);
    alarm(0);
    while (N != 0){
        char *line = data + table[N - 1].offset;

        fwrite(line, 1, table[N - 1].length, stdout);
        putchar('\n');
        fflush(stdout);

        alarm(5);
        scanf("%d", &N);
        alarm(0);
    }

    munmap(data, file_size);
    close(fd);
    return 0;
}