#include <stdio.h>
#include <unistd.h>

int main() {
    printf("Real UID: %d\n", getuid());
    printf("Effective UID: %d\n", geteuid());

    FILE *file = fopen("file", "r");
    if (file == NULL) {
        perror("Error opening file\n");
    } else {
        fclose(file);
    }

    setuid(getuid());

    printf("Real UID: %d\n", getuid());
    printf("Effective UID: %d\n", geteuid());

    file = fopen("file", "r");
    if (file == NULL) {
        perror("Error opening file\n");
    } else {
        fclose(file);
    }
    return 0;
}