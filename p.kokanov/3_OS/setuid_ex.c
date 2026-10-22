#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

void print_ids() {
    uid_t ruid = getuid();
    uid_t euid = geteuid();
    gid_t rgid = getgid();
    gid_t egid = getegid();

    printf("\truid\t|\teuid\t|\trgid\t|\tegid\n");
    printf("\t%lu\t|\t%lu\t|\t%lu\t|\t%lu\n",
        (unsigned long)ruid, (unsigned long)euid,
        (unsigned long)rgid, (unsigned long)egid);
}

int read_file() {
    FILE *file = fopen("file", "r");

	if (file == NULL) {
        return -1;
    } 

    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    printf("\nFile: \n");
    while ((read = getline(&line, &len, file)) != -1) {
        printf("%s", line);
    }

    free(line);
    fclose(file);

    return 0;
}

int set_euid() {
    uid_t ruid = getuid();

    return setuid(ruid);
}

int main() {
    print_ids();

    if(read_file() == -1) {
        perror("fopen failed");
    }

    if (set_euid() == -1) {
        perror("setuid failed");
        return EXIT_FAILURE;
    }

    print_ids();

    if(read_file() == -1) {
        perror("fopen failed");
    }

	return EXIT_SUCCESS;
}

