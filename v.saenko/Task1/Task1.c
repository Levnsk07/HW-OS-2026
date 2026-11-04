#define _POSIX_C_SOURCE 200112L
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/resource.h>
extern char **environ;


int main(int argc, char *argv[]) {
    struct rlimit limit;


    int flag;
    while ((flag = getopt(argc, argv, "ispuU:cC:dvV:")) != -1) {
        switch (flag)
        {
        case 'i': {
                uid_t ruid = getuid();
                uid_t euid = geteuid();
                gid_t rgid = getgid();
                gid_t egid = getegid();
                printf("RUser: %d, EUser: %d, RGroup: %d, EGroup: %d\n", ruid, euid, rgid, egid);
            break;
        }
        case 's': {
                setpgid(0, 0);
                printf("Процесс стал лидером группы");
            break;
        }
        case 'p': {
                pid_t pid = getpid();
                pid_t ppid = getppid();
                pid_t pgid = getpgrp();
                printf("Текущий процесс: %d, Родительский процесс: %d, Группа процессов: %d\n", pid, ppid, pgid);
            break;
        }
        case 'u': {
                struct rlimit rl;
                if (getrlimit(RLIMIT_NOFILE, &rl) == 0) {
                    printf("ulimit: \n");
                    printf("S: %lu\n", (unsigned long)rl.rlim_cur);
                    printf("H: %lu\n", (unsigned long)rl.rlim_max);
                } 
            break;
        }
        case 'U': {
                char *endptr;
                errno = 0;
                long new_val = strtol(optarg, &endptr, 10);
                getrlimit(RLIMIT_CORE, &limit);


                if (errno != 0 || endptr == optarg) {
                    fprintf(stderr, "Ошибка: Некорректное числовое значение для ulimit: %s\n", optarg);
                    break;
                }

                limit.rlim_cur = (rlim_t)new_val;
                if (setrlimit(RLIMIT_FSIZE, &limit) == 0) {
                    printf("Мягкий лимит изменен на %ld байт\n", new_val);
                }

            break;
        }
        case 'c': {
                    if (getrlimit(RLIMIT_CORE, &limit) == 0) {
                        printf("Лимиты: \n");

                        if (limit.rlim_cur == RLIM_INFINITY) {
                            printf("Мягкий - без ограничений\n");
                        } else {
                            printf("Мягкий - %lu байт\n", (unsigned long)limit.rlim_cur);
                        } 

                        if (limit.rlim_max == RLIM_INFINITY) {
                            printf("Жесткий - без ограничений\n");
                        } else {
                            printf("Жесткий - %lu байт\n", (unsigned long)limit.rlim_max);
                        }
                    } 
            break;
        }
        case 'C': {
                limit.rlim_cur = (rlim_t)atol(optarg);
                if (setrlimit(RLIMIT_CORE, &limit) == 0) {
                    printf("Лимит изменен\n");
                }
            break;
        }
        case 'd': {
                char cwd[1024];
                getcwd(cwd, sizeof(cwd));
                printf("Директория: %s\n", cwd);
            break;
        }
        case 'v': {
                for (char **env = environ; *env != NULL; env++) {
                    printf("%s\n", *env);
                }
            break;
        }
        case 'V': {
                char *eq = strchr(optarg, '=');
                *eq = '\0';
                char *name = optarg;
                char *value = eq + 1;

                if(setenv(name, value, 1) == 0) {
                    printf("Переменная обновлена: %s = %s\n", name, getenv(name));
                } 
            break;
        }
        default:
            break;
        }
    } 

    return 0;
}
