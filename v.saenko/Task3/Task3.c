#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>

int main() {

    FILE *file;

    uid_t ruid = getuid();   
    uid_t euid = geteuid();  

    printf("Real UID: %d\n", ruid);
    printf("Effective UID: %d\n", euid);

    file = fopen("Example.txt", "r");

    if (file == NULL) {
        perror("Couldnt open file!\n");
        return 1;
    }

    fclose(file);


    if (setuid(ruid) == -1) {
        perror("Error calling setuid");
        return 1;
    }

    


    
    printf("Real UID: %d\n", getuid());
    printf("Effective UID: %d\n", geteuid());
    file = fopen("Example.txt", "r");

    if (file == NULL) {
        perror("Couldnt open file!");
        return 1;
    }

    fclose(file);

    return 0;
}
