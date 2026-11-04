#include <stdio.h>
#include <unistd.h>
int main() {
    printf("RUID: %d\n", getuid());
    printf("EUID: %d\n", geteuid());
    FILE *file = fopen("file", "r");
    if (file == NULL) {
        perror("Error opening file\n");
    }
    setuid(geteuid());
    return 0;
}