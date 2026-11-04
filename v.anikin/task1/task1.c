#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>
#include <errno.h>

struct opt_input {
    char opt;
    char *arg;
};

extern char **environ;

int main(int argc, char *argv[]) {
    char options[ ] = "ispuU:cC:dvV:";
    int c = 0;

    struct opt_input input[100];
    int n = 0;

    struct rlimit rlp;

    while((c = getopt(argc, argv, options)) != EOF) {
        if (n < 100){
            input[n].opt = c;
            input[n].arg = optarg;
            n++;
        }
    }

    for (int i = n-1; i >= 0; i--) {
        switch(input[i].opt) {

            case 'i':
                printf("RUID: %d\n", getuid());
                printf("EUID: %d\n", geteuid());
                printf("RGID: %d\n", getgid());
                printf("EGID: %d\n", getegid());
                break;

            case 's':
                setpgid(0,0);
                printf("The process is now the leader\n");
                break;
            
            case 'p':
                printf("PID = %d\n", getpid());
                printf("PPID = %d\n", getppid());
                printf("PGID = %d\n", getpgid(0));
                break;
            
            case 'u':
                getrlimit(RLIMIT_FSIZE, &rlp);
                printf("Ulimit size = %ld\n", rlp.rlim_cur);
                break;

            case 'U':
                if (input[i].arg == NULL) {
                    fprintf(stderr, "Error: option -U requires an argument\n");
                    break;
                }

                char *endptr_u;
                errno = 0;
                
                long new_limit = strtol(input[i].arg, &endptr_u, 10);
                
                if (*endptr_u != '\0' || input[i].arg == endptr_u) {
                    fprintf(stderr, "Error: invalid value for -U\n");
                    break;
                }
                if (errno == ERANGE) {
                    fprintf(stderr, "Error: value for -U is out of range\n");
                    break;
                }
                if (new_limit < 0) {
                    fprintf(stderr, "Error: value for -U cannot be negative\n");
                    break;
                }
                
                getrlimit(RLIMIT_FSIZE, &rlp);
                rlp.rlim_cur = new_limit;
                setrlimit(RLIMIT_FSIZE, &rlp);
                printf("Ulimit updated: ulimit = %ld\n", rlp.rlim_cur);
                break;

            case 'c':
                getrlimit(RLIMIT_CORE, &rlp);
                printf("Core size = %ld\n", rlp.rlim_cur);
                break;
            
            case 'C':
                if (input[i].arg == NULL) {
                    fprintf(stderr, "Error: option -С requires an argument\n");
                    break;
                }

                char *endptr_c;
                errno = 0;
                
                long new_size = strtol(input[i].arg, &endptr_c, 10);
                
                if (*endptr_c != '\0' || input[i].arg == endptr_c) {
                    fprintf(stderr, "Error: invalid value for -C\n");
                    break;
                }
                if (errno == ERANGE) {
                    fprintf(stderr, "Error: value for -C is out of range\n");
                    break;
                }
                if (new_size < 0) {
                    fprintf(stderr, "Error: value for -C cannot be negative\n");
                    break;
                }

                getrlimit(RLIMIT_CORE, &rlp);
                rlp.rlim_cur = new_size;
                setrlimit(RLIMIT_CORE, &rlp);
                printf("Core size updated: size = %ld\n", rlp.rlim_cur);
                break;

            case 'd':
                printf("Current working directory: %s\n", getcwd(NULL, 100));
                break;

            case 'v': {
                char **p;
                printf("Environment variables are:\n");
                for (p = environ; *p; p++) {
                    printf("%s\n", *p);
                }
                break;
            }

            case 'V':
                putenv(input[i].arg);
                printf("Environment updated\n");
                break;
            
            case '?':
                fprintf(stderr, "Unknown option or missing argument\n");
                break;
        }
            
    }

    return 0;
}
