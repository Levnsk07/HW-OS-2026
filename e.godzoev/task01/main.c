#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/resource.h>

int main(int argc, char *argv[], char *envp[]){
    char short_options[] = "ispucdv";

    const struct option long_options[] = {
        { "Unew_ulimit", required_argument, NULL, 'U' },
		{ "Csize", required_argument, NULL, 'C' },
		{ "Vname=value", required_argument, NULL, 'V' },
		{ NULL, 0, NULL, 0}
    };
    
    int opt;
    int index = 0;
    struct rlimit limit;

    while ((opt = getopt_long(argc, argv, short_options, long_options, &index)) != -1)
    {
        switch(opt){
            case 'i':
                printf("Реальный UID: %d\n", getuid());
                printf("Эффективный UID: %d\n", geteuid());
                printf("Реальный GID: %d\n", getgid());
                printf("Эффективный GID: %d\n", getegid());
                
                break; 
            case 's':
                if (setpgid(0, 0) == -1) {
                    perror("Ошибка вызова setpgid");
                }
                else{
                    printf("Теперь процесс — лидер своей собственной группы процессов");
                }

                break;
            case 'p':
                printf("PID:  %d\n", getpid());
                printf("PPID: %d\n", getppid());
                printf("PGID: %d\n", getpgrp());

                break; 
            case 'u':
                if (getrlimit(RLIMIT_NOFILE, &limit) == 0) {
                    printf("Мягкий лимит: %lu\n", (unsigned long)limit.rlim_cur);
                    printf("Жесткий лимит: %lu\n", (unsigned long)limit.rlim_max);
                }
                else {
                    perror("Ошибка при вызове getrlimit");
                }

                break; 
            case 'c':
                if (getrlimit(RLIMIT_CORE, &limit) == 0) {
                    printf("Максимальный размер core-файла: %lu байт\n", (unsigned long)limit.rlim_cur);
                }
                else {
                    perror("Ошибка при вызове getrlimit");
                }

                break; 
            case 'd':
                ;char cwd[1024];
                if (getcwd(cwd, sizeof(cwd)) != NULL) {
                    printf("Текущая директория: %s\n", cwd);
                }
                else {
                    perror("Ошибка получения директории");
                }

                break; 
            case 'v':
                for (int i = 0; envp[i] != NULL; i++) {
                    printf("%s\n", envp[i]);
                }

                break; 
            case 'U':
                limit.rlim_cur = atol(optarg);
                limit.rlim_max = atol(optarg);

                setrlimit(RLIMIT_NOFILE, &limit);

                break; 
            case 'C':
                limit.rlim_cur = atoi(optarg);
                limit.rlim_max = atoi(optarg);

                setrlimit(RLIMIT_CORE, &limit);

                break; 
            case 'V':
                setenv(optarg, argv[optind++], 1);

                break;
        }
    }
    
    return 0;
}