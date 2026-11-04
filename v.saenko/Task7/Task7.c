#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <sys/select.h>


typedef struct {
    long offset;
    int length;
} Info;


int main() {

    int fd = open("file.txt", O_RDONLY);


    struct stat sb;
    if (fstat(fd, &sb) == -1) {
        perror("fstat");
        close(fd);
        return 1;
    }

    char *mapped = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (mapped == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }

    int capacity = 10;
    
    Info *table = malloc(capacity * sizeof(Info));

    table[0].offset = 0;


    int counter = 0;
    int pos = 0;
    int count = 0;
    for (off_t i = 0; i < sb.st_size; i++) {
        counter++;
        char ch = mapped[i];

        if (ch == '\n') {
            if (pos >= capacity - 1) {
                capacity *= 2;
                table = realloc(table, capacity * sizeof(Info));
            }

            table[pos].length = counter;
            pos++;


            table[pos].offset = i + 1;

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
                fwrite(mapped + table[i].offset, 1, table[i].length, stdout);
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
        printf("Строка %d: ", num);
        fwrite(mapped + table[realnum].offset, 1, table[realnum].length, stdout);
        if (mapped[table[realnum].offset + table[realnum].length - 1] != '\n') {
            putchar('\n');
        }




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
        printf("Строка %d: ", num);
        fwrite(mapped + table[realnum].offset, 1, table[realnum].length, stdout);
        if (mapped[table[realnum].offset + table[realnum].length - 1] != '\n') {
            putchar('\n');
        }
    }


    munmap(mapped, sb.st_size);
    free(table);
    close(fd);
    return 0;
}
