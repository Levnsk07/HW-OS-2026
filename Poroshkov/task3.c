#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>

int main(int argc, char *argv[])
{
    FILE *fp;
    const char *filename = "secret.txt";

    if (argc > 1) filename = argv[1];

    printf("Real UID:      %d\n", getuid());
    printf("Effective UID: %d\n", geteuid());
    printf("Real GID:      %d\n", getgid());
    printf("Effective GID: %d\n", getegid());

    printf("\nпытаюсь открыть '%s'...\n", filename);
    fp = fopen(filename, "r");
    if (fp == NULL) perror("fopen");
    else 
    {
        printf("файл открыт.\n");
        fclose(fp);
    }

   
    if (setuid(getuid()) == -1) perror("setuid");

    printf("Real UID:      %d\n", getuid());
    printf("Effective UID: %d\n", geteuid());

    printf("\nпытаюсь октрыть '%s' опять...\n", filename);
    fp = fopen(filename, "r");
    if (fp == NULL) perror("fopen");
    else 
    {
        printf("файл открыт.\n");
        fclose(fp);
    }

    return 0;
}

