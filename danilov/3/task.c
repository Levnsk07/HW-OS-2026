#include <stdio.h>
#include <unistd.h>

static void ids(void) {
    printf("real UID=%ld eff UID=%ld | real GID=%ld eff GID=%ld\n",
        (long)getuid(), (long)geteuid(), (long)getgid(), (long)getegid());
}

int main(){

    ids();
    
    FILE *file = fopen("a.txt", "r");
    if (file == NULL){
        perror("file not found");
    } else {
        fclose(file);
    }

    setuid(getuid());

    ids();

    FILE *file2 = fopen("a.txt", "r");
    if (file2 == NULL){
        perror("file not found");
    } else {
        fclose(file2);
    }

    return 0;
}
