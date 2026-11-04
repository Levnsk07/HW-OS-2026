#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

#define DATA_FILE "secret.txt"

static void print_ids(void)
{
    printf("RUID = %d\n", getuid());
    printf("EUID = %d\n", geteuid());
}

static void try_open(const char *label)
{
    FILE *f = fopen(DATA_FILE, "r");
    if (f == NULL) {
        printf("%s: cannot open %s: ", label, DATA_FILE);
        perror("");
    } else {
        printf("%s: opened %s successfully\n", label, DATA_FILE);
        fclose(f);
    }
}

int main(void)
{
    print_ids();
    try_open("First try");

    setuid(getuid());
    print_ids();
    try_open("Second try");

    return 0;
}
