#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>

int main(void){
    printf("Реальный UID (RUID): %d\n", getuid());
    printf("Эффективный UID (EUID): %d\n", geteuid());
    
    FILE *f = fopen("file", "r");

    if (f == NULL) {
        perror("Ошибка при открытии файла");
        return 1;
    }

    setuid(getuid());

    printf("Реальный UID (RUID): %d\n", getuid());
    printf("Эффективный UID (EUID): %d\n", geteuid());
    
    fclose(f);
    f = NULL;

    f = fopen("file", "r");

    if (f == NULL) {
        perror("Ошибка при открытии файла");
        return 1;
    }

    fclose(f);
}