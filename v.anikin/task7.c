#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/mman.h>

char *filename;
char *file_map;
off_t file_size;

void timeout_handler(int sig) {
    printf("\n\nTime is up!\n\n");
    fflush(stdout);

    if (file_map != NULL) {
        write(1, file_map, file_size);
    }

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

    file_size = lseek(fd, 0, SEEK_END);

    file_map = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (file_map == MAP_FAILED) {
        perror("mmap failed");
        close(fd);
        exit(1);
    }

    signal(SIGALRM, timeout_handler);

    table[1] = 0;
    int current_line = 1;
    
    for (int k = 0; k < file_size; k++) {
        if (file_map[k] == '\n') {
            j++;
            line_len[current_line] = j; 
            current_line++;

            if (current_line < 200) 
                table[current_line] = k + 1;
            
            j = 0;
        }
        else 
            j++;
    }
    i = current_line;

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

        write(1, file_map + table[line_number], line_len[line_number]);
        
        printf("\nEnter line number: ");
    }

    munmap(file_map, file_size);
    close(fd);

    return 0;
}
