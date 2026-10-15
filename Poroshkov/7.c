#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

volatile sig_atomic_t timed_out = 0;

void alarm_handler(int sig)
{
    timed_out = 1;
}

int main(int argc, char *argv[])
{
    struct sigaction sa;

    sa.sa_handler = alarm_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGALRM, &sa, NULL);

    off_t offsets[1000];
    int lengths[1000];

    if (argc < 2) {
        printf("вот так используй: %s <название_файла>\n", argv[0]);
        return 1;
    }
    int fd = open(argv[1], O_RDONLY);

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

    off_t file_size = st.st_size;

    if (file_size == 0) {
        printf("File is empty\n");
        close(fd);
        return 0;
    }


    char *file = mmap(
        NULL,
        file_size,
        PROT_READ,
        MAP_PRIVATE,
        fd,
        0
    );

    if (file == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }


    close(fd);

    offsets[0] = 0;

    int line = 0;

    for (off_t i = 0; i < file_size; i++) {
        if (file[i] == '\n') {
            off_t current_pos = i + 1;

            lengths[line] = current_pos - offsets[line];

            line++;
            offsets[line] = current_pos;
        }
    }

    if (offsets[line] < file_size) {
        lengths[line] = file_size - offsets[line];
        line++;
    }

    int index;

    while (1) {
        printf("введи номер строки (0 выход): ");
        fflush(stdout);

        timed_out = 0;
        alarm(5);

        scanf("%d", &index);

        alarm(0);


        if (timed_out) {
            printf("\nвремя вышло. что было в файле:\n");

            printf("%.*s", (int)file_size, file);

            munmap(file, file_size);
            return 0;
        }

        if (index == 0)
            break;

        index--;

        if (index < 0 || index >= line) {
            printf("нет такой строки\n");
            continue;
        }


        printf(
            "%.*s",
            lengths[index],
            file + offsets[index]
        );
    }

    munmap(file, file_size);

    return 0;
}
