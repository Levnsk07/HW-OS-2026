#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
     char *filename;
    
    if (argc > 1) {
        filename = argv[1];
    }
    else {
        fprintf(stderr, "Error: no file name\n");
        exit(1);
    }

    printf("RUID = %d\n", getuid());
    printf("EUID = %d\n", geteuid());
    
    FILE *file = fopen(filename, "r");
    if (file != NULL) {
        printf("File '%s' has been successfully opened\n", filename);
        fclose(file);
    }
    else {
        perror("Failed to open file");
    }

    if (setuid(getuid()) == -1) {
        perror("Setuid error");
        return 1;
    }

    // one more 
    printf("RUID = %d\n", getuid());
    printf("EUID = %d\n", geteuid());
    
    file = fopen(filename, "r");
    if (file != NULL) {
        printf("File '%s' has been successfully opened\n", filename);
        fclose(file);
    }
    else {
        perror("Failed to open file");
    }

    return 0;
}
