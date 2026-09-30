#include <stdio.h>
#include <unistd.h>

int main(int argc, char **argv) {
    printf("Реальный UID (RUID): %d\n", getuid());
    printf("Эффективный UID (EUID): %d\n", geteuid());


    FILE *file = fopen("file", "r");
    if (file == NULL) {
        perror("Error opening file\n");
    } else {
        printf("Success opening file\n");
        fclose(file);
    }

    setuid(geteuid());
    printf("Реальный UID (RUID): %d\n", getuid());
    printf("Эффективный UID (EUID): %d\n", geteuid());
}
