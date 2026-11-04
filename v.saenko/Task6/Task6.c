#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>


typedef struct {
    long offset;
    int length;
} Info;


int main() {

    int fd = open("file.txt", O_RDONLY);

    int capacity = 10;
    int count = 0;
    Info *table = malloc(capacity * sizeof(Info));
    table[0].offset = 0;

    int pos = 0;
    int counter = 0;
    char ch;
    while(read(fd, &ch, 1) > 0) {
        counter++;

        if (ch == '\n') {
            if (pos >= capacity - 1) {
                capacity *= 2;
                table = realloc(table, capacity * sizeof(Info));
            }

            table[pos].length = counter;
            pos++;


            table[pos].offset = lseek(fd, 0L, 1);

            counter = 0;
        }
    }

    count = pos;

    printf("table\n");

    printf("str offset length\n");
    for (int i = 0; i < count; i++) {
        printf("%d\t%ld\t%d\n", i + 1, table[i].offset, table[i].length);
    }

    printf("--------------------------\n");

   

    int num;




    printf("Input num:\n");

    fflush(stdout);

    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(STDIN_FILENO, &read_fds);

    struct timeval timeout;

    timeout.tv_sec = 5;
    timeout.tv_usec = 0;

    int retval = select(STDIN_FILENO + 1, &read_fds, NULL, NULL, &timeout);

    if (retval == -1) {
        perror("error");
        break;
    } else if (retval == 0) {
        printf("Timeout!\n");
        for (int i = 0; i < count; i++) {
            lseek(fd, table[i].offset, SEEK_SET);
            int len = table[i].length;
            char text[len + 1];
            read(fd, text, table[i].length);
            text[len] = '\0';
            printf("%s", text);
        }

        break;
    }
        


        if (scanf("%d", &num) != 1) {
            printf("Not num, try again:\n");
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            continue;
        }




        if (num == 0) {
            break;
        }

        if (num < 1 || num > count) {
            printf("not exists, try again:\n");
            continue;
        }

        int realnum = num-1;
        lseek(fd, table[realnum].offset, SEEK_SET);
        int len = table[realnum].length;
        char text[len + 1];
        read(fd, text, table[realnum].length);
        text[len] = '\0';
        printf("%s", text);









    while (1){
        printf("Input num:\n");

        fflush(stdout);

        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(STDIN_FILENO, &read_fds);


        int retval = select(STDIN_FILENO + 1, &read_fds, NULL, NULL, NULL);

        if (retval == -1) {
            perror("error");
            break;
        } 
        


        if (scanf("%d", &num) != 1) {
            printf("Not num, try again:\n");
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            continue;
        }




        if (num == 0) {
            break;
        }

        if (num < 1 || num > count) {
            printf("not exists, try again:\n");
            continue;
        }

        int realnum = num-1;
        lseek(fd, table[realnum].offset, SEEK_SET);
        int len = table[realnum].length;
        char text[len + 1];
        read(fd, text, table[realnum].length);
        text[len] = '\0';
        printf("%s", text);
    }



    free(table);exit
