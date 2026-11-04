#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/stat.h>

#define MAX_LEN 1024
#define MAX_LINES 100

static volatile sig_atomic_t timed_out = 0;

static void on_alarm(int sig) {
    (void)sig;
    timed_out = 1;
}

static void print_file_and_exit(char *map, off_t size) {
    fwrite(map, 1, size, stdout);
    exit(0);
}

int main() {
    int fd = open("text.txt", O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    struct stat st;
    if (fstat(fd, &st) == -1) {
        perror("fstat");
        close(fd);
        return 1;
    }
    off_t size = st.st_size;
    if (size == 0) {
        close(fd);
        return 0;
    }

    char *map = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (map == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }

    long offset[MAX_LINES];
    int len[MAX_LINES];
    int nlines = 0;

    long start = 0;
    int cur = 0;
    for (long i = 0; i < size && nlines < MAX_LINES; i++) {
        if (map[i] == '\n') {
            offset[nlines] = start;
            len[nlines] = cur;
            nlines++;
            start = i + 1;
            cur = 0;
        } else {
            cur++;
        }
    }

    if (cur > 0 && nlines < MAX_LINES) {
        offset[nlines] = start;
        len[nlines] = cur;
        nlines++;
    }

    printf("таблица: строка | отступ | длина\n");
    for (int i = 0; i < nlines; i++)
        printf("%d | %ld | %d\n", i + 1, offset[i], len[i]);

    int num;
    int first = 1;
    while (1) {
        printf("Введите номер строки:\n");

        int r;
        if (first) {
            timed_out = 0;
            signal(SIGALRM, on_alarm);
            alarm(5);
            r = scanf("%d", &num);
            alarm(0);
            if (timed_out)
                print_file_and_exit(map, size);
            first = 0;
        } else {
            r = scanf("%d", &num);
        }

        if (r != 1) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            if (c == EOF)
                break;
            printf("нужно число\n");
            continue;
        }

        int c = getchar();
        int junk = 0;
        while (c != '\n' && c != EOF) {
            junk = 1;
            c = getchar();
        }
        if (junk) {
            printf("нужно число\n");
            continue;
        }

        if (num == 0)
            break;
        if (num < 1 || num > nlines) {
            printf("нет строки с таким номером\n");
            continue;
        }

        char buf[MAX_LEN];
        int n = len[num - 1];
        if (n > MAX_LEN - 1)
            n = MAX_LEN - 1;
        memcpy(buf, map + offset[num - 1], n);
        buf[n] = '\0';
        printf("%s\n", buf);
    }

    munmap(map, size);
    close(fd);
    return 0;
}

