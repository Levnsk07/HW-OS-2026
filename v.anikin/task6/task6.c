#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

char *filename;

void timeout_handler(int sig) {
    printf("\n\nTime is up!\n\n");
    fflush(stdout);

    int fd;

    if ((fd = open(filename, O_RDONLY)) == -1) {
        perror("Error reopening file");
        exit(1);
    }

    char buffer[200];
    int bytes_read;
    
    while ((bytes_read = read(fd, buffer, sizeof(buffer))) > 0) {
        write(1, buffer, bytes_read);
    }
    
    close(fd);
    exit(0);
}


int main(int argc, char *argv[]) {
    int table[200];
    int line_number, line_len[200];
    int i = 1, j = 0;
    int fd;
    char symb;
    char buf[200];

    if ((fd = open(argv[1], O_RDONLY)) == -1) {
        perror("Wrong file name");
        exit(1);
    }

    filename = argv[1];

    signal(SIGALRM, timeout_handler);

    table[1] = 0;
    while(read(fd, &symb, 1)) {
        if (symb == '\n') {
            j++;
            line_len[i++] = j;
            table[i] = lseek(fd, 0, SEEK_CUR);
            j = 0;
        }
        else
            j++;
    };

    printf("\nTable\n");
    printf("Line no. | Offset | Length\n");
    for (int k = 1; k < i; k++)
        printf("%-9d|%-8d|%-6d\n", k, table[k], line_len[k]);

    printf("\nEnter line number (you have 5 sec.): ");

    alarm(5);

    while(1) {
        
        if (scanf("%d", &line_number) != 1) {
            alarm(0);
            break;
        }

        alarm(0);

        if (line_number <= 0)
            break;

        if (line_number >= i) {
            fprintf(stderr, "Bad Line Number\n");
            printf("\nEnter line number: ");
            continue;
        }

        lseek(fd, table[line_number], SEEK_SET);

        if(read(fd, buf, line_len[line_number]))
            write(1, buf, line_len[line_number]);
        else
            fprintf(stderr, "Bad Line Number\n");
        
        printf("\nEnter line number: ");
    }

    close(fd);

    return 0;
}
