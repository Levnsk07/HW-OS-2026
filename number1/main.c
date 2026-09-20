#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#include <stdlib.h>
#endif


#include <stdio.h>
#include <unistd.h>
#include <sys/resource.h>
#include <getopt.h>
#include <string.h>

void printHelp_ru() {
    fprintf(stderr, "Invalid flag: \n"
            "\t -i  Печатает реальные и эффективные идентификаторы пользователя и группы.  \n"
            "\t -s  Процесс становится лидером группы. Подсказка: смотри setpgid(2). \n"
            "\t -p  Печатает идентификаторы процесса, процесса-родителя и группы процессов. \n"
            "\t -u  Печатает значение ulimit \n"
            "\t -Unew_ulimit  Изменяет значение ulimit. Подсказка: смотри atol(3C) на странице руководства strtol(3C) \n"
            "\t -c  Печатает размер в байтах core-файла, который может быть создан. \n"
            "\t -Csize  Изменяет размер core-файла \n"
            "\t -d  Печатает текущую рабочую директорию \n"
            "\t -v  Распечатывает переменные среды и их значения \n"
            "\t -Vname=value  Вносит новую переменную в среду или изменяет значение существующей переменной. \n"
    );
}

void printHelp() {
    fprintf(stderr, "Invalid flag: \n"
            "\t -i  Print PID user.  \n"
            "\t -s  Set leader of Process. \n"
            "\t -p  Print PID process. \n"
            "\t -u  print ulimit. \n"
            "\t -Unew_ulimit  change ulimit.\n"
            "\t -c  Print size of core file. \n"
            "\t -Csize  change size of core file \n"
            "\t -d  pwd \n"
            "\t -v  print PATH \n"
            "\t -Vname=value  change PATH statesman. \n"
    );
}

int main(int argc, char *argv[], char *envp[]) {
    if (argc < 2) {
        printHelp();
        return 1;
    }

    const char *short_options = "ispucdv";

    const struct option long_param[] = {
        {"Unew_ulimit", required_argument,NULL, 'U'},
        {"Csize", required_argument,NULL, 'C'},
        {"Vname", required_argument,NULL, 'V'}
    };

    // TIP лекция 2 - слайд 15
    int opt;
    int long_opt;
    struct rlimit lim;




    while ((opt = getopt_long(argc, argv, short_options, long_param, &long_opt)) != -1) {
        switch (opt) {
            case 'i':
                printf("Реальный UID (RUID): %d\n", getuid());
                printf("Эффективный UID (EUID): %d\n", geteuid());
                printf("Реальный GID (RGID): %d\n", getgid());
                printf("Эффективный GID (EGID): %d\n", getegid());
                return 0;
            case 's':
                if (setpgid(0, 0) == -1) {
                    fprintf(stderr, "Error in setpgid\n");
                    return 1;
                }
                printf("Sucsess \n");
                return 0;

            case 'p':
                printf("Инентифекатор процесса: %d \n", getpid());
                printf("Инентифекатор процесса-родитль: %d\n", getppid());
                printf("Инентифекатор группы процесса: %d\n", getpgrp());

                return 0;

            case 'u':
                if (getrlimit(RLIMIT_NOFILE, &lim) == 0) {
                    printf(" Soft limit: %lld \n", (long long) lim.rlim_cur);
                    printf(" Hard limit: %lld \n", (long long) lim.rlim_max);
                } else {
                    fprintf(stderr, "Error");
                }

                return 0;
            case 'U': // -Unew_ulimit

                lim.rlim_cur = atoi(optarg);

                setrlimit(RLIMIT_NOFILE, &lim);

                printf(" Soft limit: %lld \n", (long long) lim.rlim_cur);
                printf(" Hard limit: %lld \n", (long long) lim.rlim_max);
                return 0;
            case 'c':

                if (getrlimit(RLIMIT_CORE, &lim) == 0) {
                    if (lim.rlim_cur != RLIM_INFINITY) {
                        printf("Current Soft Limit: %lld bytes\n", (long long) lim.rlim_cur);
                    }else {
                        printf("Current Soft Limit: unlimit bytes\n");
                    }

                    if (lim.rlim_max != RLIM_INFINITY) {
                        printf("Current Hard Limit: %lld bytes\n", (long long) lim.rlim_max);
                    }else {
                        printf("Current Hard Limit: unlimit bytes\n");
                    }
                } else {
                    perror("Failed to get limit");
                    return 1;
                }
                return 0;

            case 'C': // -Csize
                ;

                int arg1= atoi(optarg);
                lim.rlim_cur = arg1;
                lim.rlim_max = arg1;

                if (setrlimit(RLIMIT_CORE, &lim) == -1) {
                    fprintf(stderr, "Error");
                    return 1;
                }


                getrlimit(RLIMIT_CORE, &lim);
                if (lim.rlim_cur == -1) {
                    printf("Current Soft Limit: unlimit bytes\n");
                } else {
                    printf("Current Soft Limit: %lld bytes\n", lim.rlim_cur);
                }

                if (lim.rlim_max == -1) {
                    printf("Current Hard Limit: unlimit bytes\n");
                } else {
                    printf("Current Hard Limit: %lld bytes\n", lim.rlim_max);
                }
                return 0;

            case 'd':
                ;
                char cwd[1024];

                if (getcwd(cwd, sizeof(cwd)) != NULL) {
                    printf("%s\n", cwd);
                    return 0;
                } else {
                    perror("Error pwd");
                    return 1;
                }

            case 'v':
                for (int i = 0; envp[i] != NULL; i++) {
                    printf("%s\n", envp[i]);
                }
                return 0;
            case 'V':
                ;
                char *path = getenv(optarg);
                if (path == NULL) {
                    fprintf(stderr, "Path does not exist \n");
                    return 1;
                }
                char *arg = argv[optind++];
                if (arg == NULL) {
                    fprintf(stderr, "Argument is NULL \n");
                    printf("test %s", arg);
                    return 1;
                }
                // path = arg;
                setenv(optarg, arg, 1);

                for (int i = 0; envp[i] != NULL; i++) {
                    printf("%s\n", envp[i]);
                }

                return 0;


            default:
                printHelp();
                return 1;
        }
    }

    return 0;
}
