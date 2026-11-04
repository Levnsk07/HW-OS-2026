#define _XOPEN_SOURCE 700

#include <errno.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/resource.h>
#include <ulimit.h>
#include <unistd.h>

extern char **environ;

struct action {
    int option;
    char *argument;
};

int execute(int option, char *argument) {
    struct rlimit limit;
    char path[PATH_MAX];
    char *end;
    long value = 0;

    if (option == 'U' || option == 'C') {
        errno = 0;
        value = strtol(argument, &end, 10);
        if (errno != 0 || end == argument || *end != '\0' || value < 0) {
            fprintf(stderr, "Invalid value for -%c: %s\n", option, argument);
            return 1;
        }
    }

    switch (option) {
        case 'i':
            printf("UID: %lu, EUID: %lu\n", (unsigned long)getuid(),
                   (unsigned long)geteuid());
            printf("GID: %lu, EGID: %lu\n", (unsigned long)getgid(),
                   (unsigned long)getegid());
            break;
        case 's':
            if (setpgid(0, 0) == -1) {
                perror("setpgid");
                return 1;
            }
            break;
        case 'p':
            printf("PID: %ld, PPID: %ld, PGID: %ld\n", (long)getpid(),
                   (long)getppid(), (long)getpgrp());
            break;
        case 'u':
            value = ulimit(UL_GETFSIZE, 0L);
            if (value == -1) {
                perror("ulimit");
                return 1;
            }
            printf("ulimit: %ld (512-byte blocks)\n", value);
            break;
        case 'U':
            if (value > LONG_MAX / 512) {
                fprintf(stderr, "Value for -U is too large\n");
                return 1;
            }
            if (ulimit(UL_SETFSIZE, value) == -1) {
                perror("ulimit");
                return 1;
            }
            break;
        case 'c':
            if (getrlimit(RLIMIT_CORE, &limit) == -1) {
                perror("getrlimit");
                return 1;
            }
            if (limit.rlim_cur == RLIM_INFINITY) {
                puts("core: unlimited");
            } else {
                printf("core: %llu bytes\n", (unsigned long long)limit.rlim_cur);
            }
            break;
        case 'C':
            if (getrlimit(RLIMIT_CORE, &limit) == -1) {
                perror("getrlimit");
                return 1;
            }
            limit.rlim_cur = (rlim_t)value;
            if (setrlimit(RLIMIT_CORE, &limit) == -1) {
                perror("setrlimit");
                return 1;
            }
            break;
        case 'd':
            if (getcwd(path, sizeof(path)) == NULL) {
                perror("getcwd");
                return 1;
            }
            puts(path);
            break;
        case 'v':
            for (int i = 0; environ[i] != NULL; i++) {
                puts(environ[i]);
            }
            break;
        case 'V':
            end = strchr(argument, '=');
            if (end == NULL || end == argument) {
                fprintf(stderr, "Expected -Vname=value\n");
                return 1;
            }
            if (putenv(argument) != 0) {
                perror("putenv");
                return 1;
            }
            break;
    }

    return 0;
}

int main(int argc, char *argv[]) {
    if (argc == 1) {
        printf("Usage: %s [-i] [-s] [-p] [-u] [-Unumber] [-c] [-Csize] "
               "[-d] [-v] [-Vname=value]\n", argv[0]);
        return 0;
    }

    size_t capacity = 0;
    for (int i = 1; i < argc; i++) {
        capacity += strlen(argv[i]);
    }

    struct action *actions = malloc((capacity + 1) * sizeof(*actions));
    if (actions == NULL) {
        perror("malloc");
        return 1;
    }

    int count = 0;
    int option;
    opterr = 0;

    while ((option = getopt(argc, argv, ":ispuU:cC:dvV:")) != -1) {
        if (option == '?' || option == ':') {
            fprintf(stderr, "%s: -%c\n",
                    option == '?' ? "Unknown option" : "Missing argument",
                    optopt);
            free(actions);
            return 1;
        }
        actions[count].option = option;
        actions[count].argument = optarg;
        count++;
    }

    if (optind < argc) {
        fprintf(stderr, "Unexpected argument: %s\n", argv[optind]);
        free(actions);
        return 1;
    }

    int result = 0;
    for (int i = count - 1; i >= 0; i--) {
        if (execute(actions[i].option, actions[i].argument) != 0) {
            result = 1;
            break;
        }
    }

    free(actions);
    return result;
}
