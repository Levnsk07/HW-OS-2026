#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

// typedef struct line {
//     int startPlace;
//     int lineLength;
// }line;

int main(int argc, char *argv[]) {
    // узнаем какую чтроку надо получить, после идём до этой строки и выводим строку
    int fd = open("file", O_RDONLY);
    char buffer;

    int numLine;
    scanf("%d", &numLine);

    int counter = 1;
    while (read(fd, &buffer, 1) > 0) {
        if (counter != numLine) {
            while (buffer != '\n') {
                read(fd, &buffer, 1);
            }
            counter++;
        } else {
            while (buffer != '\n') {
                read(fd, &buffer, 1);
                printf("%c", buffer);
            }
            printf("\n");
            return 0;
        }
    }

    return 0;
}
