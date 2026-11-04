#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

#define MAX_LEN 1024
#define MAX_LINES 100

static volatile sig_atomic_t timed_out = 0;

static void on_alarm(int sig) {
    (void)sig;
    timed_out = 1;
}

static void print_file_and_exit(int fd) {
    char buf[MAX_LEN];
    lseek(fd, 0L, SEEK_SET);
    int r;
    while ((r = read(fd, buf, sizeof buf)) > 0)
        write(1, buf, r);
    exit(0);
}

int main() {
    int fd = open("text.txt", O_RDONLY);
    if (fd == -1) {
        perror("open");
        return 1;
    }

    long offset[MAX_LINES];
    int len[MAX_LINES];
    int nlines = 0;

    char ch;
    long start = 0;
    int cur = 0;
    while (nlines < MAX_LINES && read(fd, &ch, 1) == 1) {
        if (ch == '\n') {
            offset[nlines] = start;
            len[nlines] = cur;
            nlines++;
            start = lseek(fd, 0L, SEEK_CUR);
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
                print_file_and_exit(fd);
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

        lseek(fd, offset[num - 1], SEEK_SET);
        char buf[MAX_LEN];
        int n = len[num - 1];
        if (n > MAX_LEN - 1)
            n = MAX_LEN - 1;
        int rr = read(fd, buf, n);
        if (rr < 0) {
            perror("read");
            break;
        }
        buf[rr] = '\0';
        printf("%s\n", buf);
    }

    close(fd);
    return 0;
}

