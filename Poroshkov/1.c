#define _XOPEN_SOURCE 600
#ifdef __APPLE__
#define _DARWIN_C_SOURCE
#endif
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <string.h>
#include <errno.h>
#include <stdint.h>

extern char **environ;

typedef struct {
    int name;
    char *value;
} Option;

static long parse_number(const char *str) {
    char *end;
    errno = 0;
    long value = strtol(str, &end, 10);
    if (errno == ERANGE || end == str || *end != '\0' || value < 0) {
        fprintf(stderr, "Некорректное число: %s\n", str);
        return -1;
    }
    return value;
}

static int set_limit(int resource, long value) {
    struct rlimit limit;
    if (getrlimit(resource, &limit) == -1) {
        perror("getrlimit");
        return -1;
    }
    rlim_t requested = (rlim_t)value;
    if ((uintmax_t)requested != (uintmax_t)value || requested == RLIM_INFINITY ||
        (limit.rlim_max != RLIM_INFINITY && requested > limit.rlim_max)) {
        fprintf(stderr, "Значение превышает допустимый лимит\n");
        return -1;
    }
    limit.rlim_cur = requested;
    if (setrlimit(resource, &limit) == -1) {
        perror("setrlimit");
        return -1;
    }
    return 0;
}

static int show_process_limit(void) {
#ifdef RLIMIT_NPROC
    struct rlimit limit;
    if (getrlimit(RLIMIT_NPROC, &limit) == -1) {
        perror("getrlimit(RLIMIT_NPROC)");
        return -1;
    }
    if (limit.rlim_cur == RLIM_INFINITY)
        puts("max user processes              (-u) unlimited");
    else
        printf("max user processes              (-u) %ju\n", (uintmax_t)limit.rlim_cur);
#else
    errno = 0;
    long limit = sysconf(_SC_CHILD_MAX);
    if (limit == -1 && errno != 0) {
        perror("sysconf(_SC_CHILD_MAX)");
        return -1;
    }
    if (limit == -1)
        puts("max user processes              (-u) indeterminate");
    else
        printf("max user processes              (-u) %ld\n", limit);
#endif
    return 0;
}

static int print_directory(void) {
    size_t size = 128;
    for (;;) {
        char *cwd = malloc(size);
        if (cwd == NULL) {
            perror("malloc");
            return -1;
        }
        if (getcwd(cwd, size) != NULL) {
            puts(cwd);
            free(cwd);
            return 0;
        }
        int error = errno;
        free(cwd);
        if (error != ERANGE || size > SIZE_MAX / 2) {
            errno = error;
            perror("getcwd");
            return -1;
        }
        size *= 2;
    }
}

int main(int argc, char *argv[]) {
    int opt, status = EXIT_SUCCESS;
    size_t count = 0, capacity = 0;
    Option *options = NULL;
    while ((opt = getopt(argc, argv, ":ipsuU:cC:dvV:")) != -1) {
        if (opt == '?' || opt == ':') {
            fprintf(stderr, opt == '?' ? "Неизвестная опция: -%c\n" :
                    "Опция -%c требует аргумент\n", optopt);
            free(options);
            return EXIT_FAILURE;
        }
        if (count == capacity) {
            size_t next = capacity == 0 ? 16 : capacity * 2;
            if (next < capacity || next > SIZE_MAX / sizeof(*options)) {
                free(options);
                return EXIT_FAILURE;
            }
            Option *grown = realloc(options, next * sizeof(*options));
            if (grown == NULL) {
                perror("realloc");
                free(options);
                return EXIT_FAILURE;
            }
            options = grown;
            capacity = next;
        }
        options[count].name = opt;
        options[count++].value = optarg;
    }
    if (optind != argc) {
        fprintf(stderr, "Неожиданный аргумент: %s\n", argv[optind]);
        free(options);
        return EXIT_FAILURE;
    }
    while (count > 0) {
        Option option = options[--count];
        switch (option.name) {
        case 'i':
            printf("RUID: %ld\nEUID: %ld\nRGID: %ld\nEGID: %ld\n",
                   (long)getuid(), (long)geteuid(), (long)getgid(), (long)getegid());
            break;
        case 's':
            if (setpgid(0, 0) == -1) {
                perror("setpgid");
                status = EXIT_FAILURE;
            } else {
                puts("Процесс стал лидером группы");
            }
            break;
        case 'p':
            printf("PID: %ld\nPPID: %ld\nPGID: %ld\n",
                   (long)getpid(), (long)getppid(), (long)getpgrp());
            break;
        case 'u':
            if (show_process_limit() == -1)
                status = EXIT_FAILURE;
            break;
        case 'U': {
            long value = parse_number(option.value);
            if (value == -1) {
                status = EXIT_FAILURE;
                break;
            }
#ifdef RLIMIT_NPROC
            if (set_limit(RLIMIT_NPROC, value) == -1)
                status = EXIT_FAILURE;
            else
                printf("max user processes установлен в %ld\n", value);
#else
            fprintf(stderr, "-U: эта система не предоставляет RLIMIT_NPROC. "
                    "sysconf(_SC_CHILD_MAX) доступен только для чтения; "
                    "лимит числа процессов не изменён.\n");
            status = EXIT_FAILURE;
#endif
            break;
        }
        case 'c': {
            struct rlimit limit;
            if (getrlimit(RLIMIT_CORE, &limit) == -1) {
                perror("getrlimit(RLIMIT_CORE)");
                status = EXIT_FAILURE;
            } else if (limit.rlim_cur == RLIM_INFINITY) {
                puts("Core limit: unlimited");
            } else {
                printf("Core limit: %ju байт\n", (uintmax_t)limit.rlim_cur);
            }
            break;
        }
        case 'C': {
            long value = parse_number(option.value);
            if (value == -1 || set_limit(RLIMIT_CORE, value) == -1)
                status = EXIT_FAILURE;
            break;
        }
        case 'd':
            if (print_directory() == -1)
                status = EXIT_FAILURE;
            break;
        case 'v':
            for (char **entry = environ; *entry != NULL; ++entry)
                puts(*entry);
            break;
        case 'V': {
            char *equals = strchr(option.value, '=');
            if (equals == NULL || equals == option.value) {
                fprintf(stderr, "-V требует непустое name=value\n");
                status = EXIT_FAILURE;
            } else if (putenv(option.value) != 0) {
                perror("putenv");
                status = EXIT_FAILURE;
            }
            break;
        }
        }
    }
    free(options);
    return status;
}

