#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/mman.h>

#define MAX_LINES 1024
#define TIME_OUT 5

struct line_info {
    off_t offset;
    size_t length;
};

static char *g_data = NULL;
static off_t g_size = 0;

void clear_input(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

void on_alarm(int sig)
{
    (void)sig;

    if (g_data != NULL && g_size > 0)
    {
        write(STDOUT_FILENO, g_data, g_size);
    }
    write(STDOUT_FILENO, "\nTime is up\n", 12);
    _exit(0);
}

int main(int argc, char *argv[])
{
    int fd;
    struct stat st;
    off_t size;
    char *data;
    off_t line_start = 0;
    int line_count = 0;
    struct line_info lines[MAX_LINES];

    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <file>\n", argv[0]);
        return 1;
    }

    fd = open(argv[1], O_RDONLY);
    if (fd == -1)
    {
        perror("open");
        return 1;
    }

    // file size
    if (fstat(fd, &st) == -1)
    {
        perror("fstat");
        close(fd);
        return 1;
    }
    size = st.st_size;

    if (size == 0)
    {
        fprintf(stderr, "File is empty\n");
        close(fd);
        return 1;
    }

    // map into memory
    data = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
    if (data == MAP_FAILED)
    {
        perror("mmap");
        close(fd);
        return 1;
    }

    // build table
    for (off_t i = 0; i < size; ++i)
    {
        if (data[i] == '\n') {
            if (line_count < MAX_LINES) {
                lines[line_count].offset = line_start;
                lines[line_count].length = i + 1 - line_start;
                ++line_count;
            }
            line_start = i + 1;
        }
    }
    if (line_start < size && line_count < MAX_LINES)
    {
        lines[line_count].offset = line_start;
        lines[line_count].length = size - line_start;
        ++line_count;
    }

    g_data = data;
    g_size = size;

    if (signal(SIGALRM, on_alarm) == SIG_ERR)
    {
        perror("signal");
        munmap(data, size);
        close(fd);
        return 1;
    }

    // print table
    printf("Table of lines:\n");
    for (int i = 0; i < line_count; ++i)
    {
        printf("  line %d: offset=%ld length=%zu\n",
               i + 1, (long)lines[i].offset, lines[i].length);
    }

    int first = 1;

    while (1)
    {
        int num;

        printf("\nEnter line number (0 to end): ");
        fflush(stdout);

        if (first)
            alarm(TIME_OUT);

        int ret = scanf("%d", &num);

        if (first)
        {
            first = 0;
            alarm(0);
        }

        if (ret == EOF)
        {
            alarm(0);
            break;
        }

        if (ret != 1)
        {
            alarm(0);
            clear_input();
            printf("Invalid input. Please enter a number.\n");
            continue;
        }

        alarm(0);
        clear_input();

        if (num == 0)
            break;

        if (num < 1 || num > line_count)
        {
            printf("Invalid line number (1..%d)\n", line_count);
            continue;
        }

        int idx = num - 1;

        // print line
        write(STDOUT_FILENO,
              data + lines[idx].offset,
              lines[idx].length);

        if (lines[idx].length == 0 ||
            data[lines[idx].offset + lines[idx].length - 1] != '\n')
        {
            write(STDOUT_FILENO, "\n", 1);
        }
    }

    munmap(data, size);
    close(fd);
    return 0;
}
