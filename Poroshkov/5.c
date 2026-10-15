#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {
    char c, buffer[10000];
    off_t  offsets[1000];
    int lengths[1000];

    if (argc < 2) {
        printf("вот так используй: %s <название_файла>\n", argv[0]);
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);
    offsets[0] = 0;
    int line = 0;
    while (read(fd, &c, 1) == 1) {
        if (c == '\n') {
            off_t current_pos = lseek(fd, 0L, SEEK_CUR);
            lengths[line] = current_pos - offsets[line];
            offsets[++line] = current_pos;
        }
        
    }
    
    off_t end_pos = lseek(fd, 0L, SEEK_CUR);
        if (end_pos > offsets[line]) {
            lengths[line] = end_pos - offsets[line]  ;
            line++;
        }

    int index;
    while (1) {
        scanf("%d" , &index);
         if (index == 0)
            break;
        index--;
        lseek(fd, offsets[index], SEEK_SET);
       
        read(fd, buffer, lengths[index]);
        buffer[lengths[index]] = '\0';
        printf("%s", buffer);
    }
    
}
