#include <stdio.h>
#include <unistd.h>

void print_ids(void)
{
    printf("real uid = %ld, effective uid = %ld\n",
           (long)getuid(), (long)geteuid());
}

void try_open(const char *filename)
{
    FILE *file = fopen(filename, "r+");

    if (file == NULL) {
        perror("fopen");
    } else {
        printf("fopen: success\n");
        fclose(file);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s file\n", argv[0]);
        return 1;
    }

    /* Шаги 1-2 */
    printf("Before setuid:\n");
    print_ids();
    try_open(argv[1]);

    /* Шаг 3 */
    if (setuid(getuid()) == -1) {
        perror("setuid");
        return 1;
    }

    /* Шаг 4 */
    printf("\nAfter setuid:\n");
    print_ids();
    try_open(argv[1]);

    return 0;
}
