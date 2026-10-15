#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>

int main(int argc, char *argv[])
{
    FILE *fp;
    const char *filename = "data.txt";

    if (argc > 1) filename = argv[1];

    printf("Real UID:      %d\n", getuid());
    printf("Effective UID: %d\n", geteuid());
    printf("Real GID:      %d\n", getgid());
    printf("Effective GID: %d\n", getegid());

    printf("\nTrying to open '%s'...\n", filename);
    fp = fopen(filename, "r");
    if (fp == NULL) perror("fopen");
    else 
    {
        printf("File opened successfully.\n");
        fclose(fp);
    }

    printf("\nCalling setuid(getuid())...\n");
    if (setuid(getuid()) == -1) perror("setuid");

    printf("Real UID:      %d\n", getuid());
    printf("Effective UID: %d\n", geteuid());

    printf("\nTrying to open '%s' again...\n", filename);
    fp = fopen(filename, "r");
    if (fp == NULL) perror("fopen");
    else 
    {
        printf("File opened successfully.\n");
        fclose(fp);
    }

    return 0;
}
