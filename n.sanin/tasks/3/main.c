#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>

int main(int argc, char **argv) {
    const char *file = (argc > 1) ? argv[1] : "4.txt";

    printf("before: real uid=%ld, effective uid=%ld\n",
           (long)getuid(), (long)geteuid());

    FILE *f = fopen(file, "r");
    if (f != NULL) {
        printf("fopen: OK\n");
        fclose(f);
    } else {
        perror("fopen");
    }

    if (setuid(getuid()) == -1)
        perror("setuid");

    printf("after:  real uid=%ld, effective uid=%ld\n",
           (long)getuid(), (long)geteuid());

    f = fopen(file, "r");
    if (f != NULL) {
        printf("fopen: OK\n");
        fclose(f);
    } else {
        perror("fopen");
    }

    return 0;
}
