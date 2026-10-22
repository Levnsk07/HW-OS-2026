#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#define MAX_LINES 1024

struct line_info {
    off_t offset;
    size_t length;
};

void clear_input(void)
{
    int c;
    while ((c = getchar()) != '\n' && c != EOF)
        ;
}

int main(int argc, char *argv[])
{
    int fd;
    char buf[4096];
    ssize_t n;
    off_t pos = 0;
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

    while ((n = read(fd, buf, sizeof(buf))) > 0)
    {
        for (ssize_t i = 0; i < n; ++i)
        {
            if (buf[i] == '\n')
            {
                if (line_count < MAX_LINES)
                {
                    lines[line_count].offset = line_start;
                    lines[line_count].length = pos + i + 1 - line_start;
                    ++line_count;
                }
                line_start = pos + i + 1;
            }
        }
        pos += n;
    }

    if (line_start < pos && line_count < MAX_LINES)
    {
        lines[line_count].offset = line_start;
        lines[line_count].length = pos - line_start;
        ++line_count;
    }

    printf("Table of lines:\n");
    for (int i = 0; i < line_count; ++i)
    {
        printf("  line %d: offset=%ld length=%zu\n", i + 1, (long)lines[i].offset, lines[i].length);
    }

    while (1)
    {
        int num;
        char linebuf[4096];

        printf("\nEnter line number (0 to end): ");
        fflush(stdout);

        int ret = scanf("%d", &num);
        if (ret == EOF)
        {
            break;
        }
        if (ret != 1)
        {
            clear_input();
            printf("Invalid input. Please enter a number.\n");
            continue;
        }

        clear_input();

        if (num == 0) break;
        if (num < 1 || num > line_count)
        {
            printf("Invalid line number (1..%d)\n", line_count);
            continue;
        }

        int idx = num - 1;

        if (lseek(fd, lines[idx].offset, SEEK_SET) == -1)
        {
            perror("lseek");
            continue;
        }

        ssize_t r = read(fd, linebuf, lines[idx].length);
        if (r == -1)
        {
            perror("read");
            continue;
        }

        write(STDOUT_FILENO, linebuf, r);
        if (r == 0 || linebuf[r - 1] != '\n')
        {
            write(STDOUT_FILENO, "\n", 1);
        }
    }

    close(fd);
    return 0;
}
