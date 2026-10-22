#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <string.h>
#include <errno.h>

extern char **environ;

struct opt_item {
    int opt;
    char *arg;
};

int main(int argc, char *argv[]) {
    int c;
    struct opt_item opts[100];
    int nopts = 0;

    /* Collect all options first */
    while ((c = getopt(argc, argv, "ispuU:cC:dvV:")) != -1) {
        opts[nopts].opt = c;
        opts[nopts].arg = optarg;
        nopts++;
    }

    /* If no options given, print message and exit */
    if (nopts == 0) {
        printf("No options given.\n");
        return 0;
    }

    /* Process options from right to left */
    for (int i = nopts - 1; i >= 0; i--) {
        switch (opts[i].opt) {

            case 'i':
                printf("Real UID: %d, Effective UID: %d\n",
                       getuid(), geteuid());
                printf("Real GID: %d, Effective GID: %d\n",
                       getgid(), getegid());
                break;

            case 's':
                if (setpgid(0, 0) == -1)
                    perror("setpgid");
                else
                    printf("Process became group leader\n");
                break;

            case 'p':
                printf("PID: %d, PPID: %d, PGID: %d\n",
                       getpid(), getppid(), getpgid(0));
                break;

            case 'u':
                {
                    struct rlimit rl;
                    if (getrlimit(RLIMIT_FSIZE, &rl) == -1) {
                        perror("getrlimit");
                        break;
                    }
                    printf("ulimit (file size): %ld\n", (long)rl.rlim_cur);
                }
                break;

            case 'U':
                {
                    char *endptr;
                    long val = strtol(opts[i].arg, &endptr, 10);

                    /* Check if argument is a valid number */
                    if (*endptr != '\0' || endptr == opts[i].arg) {
                        fprintf(stderr, "Invalid number for -U: %s\n",
                                opts[i].arg);
                        break;
                    }
                    /* Check if negative */
                    if (val < 0) {
                        fprintf(stderr, "-U value cannot be negative\n");
                        break;
                    }

                    struct rlimit rl;
                    if (getrlimit(RLIMIT_FSIZE, &rl) == -1) {
                        perror("getrlimit");
                        break;
                    }
                    if ((rlim_t)val > rl.rlim_max) {
                        fprintf(stderr, "-U value too large (max %ld)\n",
                                (long)rl.rlim_max);
                        break;
                    }
                    rl.rlim_cur = (rlim_t)val;
                    if (setrlimit(RLIMIT_FSIZE, &rl) == -1) {
                        perror("setrlimit");
                        break;
                    }
                    printf("ulimit changed to %ld\n", val);
                }
                break;

            case 'c':
                {
                    struct rlimit rl;
                    if (getrlimit(RLIMIT_CORE, &rl) == -1) {
                        perror("getrlimit");
                        break;
                    }
                    printf("Core file size: %ld bytes\n", (long)rl.rlim_cur);
                }
                break;

            case 'C':
                {
                    char *endptr;
                    long val = strtol(opts[i].arg, &endptr, 10);

                    if (*endptr != '\0' || endptr == opts[i].arg) {
                        fprintf(stderr, "Invalid number for -C: %s\n",
                                opts[i].arg);
                        break;
                    }
                    if (val < 0) {
                        fprintf(stderr, "-C value cannot be negative\n");
                        break;
                    }

                    struct rlimit rl;
                    if (getrlimit(RLIMIT_CORE, &rl) == -1) {
                        perror("getrlimit");
                        break;
                    }
                    if ((rlim_t)val > rl.rlim_max) {
                        fprintf(stderr, "-C value too large (max %ld)\n",
                                (long)rl.rlim_max);
                        break;
                    }
                    rl.rlim_cur = (rlim_t)val;
                    if (setrlimit(RLIMIT_CORE, &rl) == -1) {
                        perror("setrlimit");
                        break;
                    }
                    printf("Core file size changed to %ld\n", val);
                }
                break;

            case 'd':
                {
                    char cwd[1024];
                    if (getcwd(cwd, sizeof(cwd)) == NULL) {
                        perror("getcwd");
                        break;
                    }
                    printf("CWD: %s\n", cwd);
                }
                break;

            case 'v':
                {
                    printf("Environment variables:\n");
                    for (char **env = environ; *env; env++)
                        printf("  %s\n", *env);
                }
                break;

            case 'V':
                {
                    /* Check if argument contains '=' */
                    if (strchr(opts[i].arg, '=') == NULL) {
                        fprintf(stderr,
                                "Invalid format for -V: %s (need name=value)\n",
                                opts[i].arg);
                        break;
                    }
                    if (putenv(opts[i].arg) != 0) {
                        perror("putenv");
                        break;
                    }
                    printf("Environment variable set: %s\n", opts[i].arg);
                }
                break;

            case '?':
                fprintf(stderr, "Unknown option: %c\n", optopt);
                break;

            default:
                fprintf(stderr, "Unhandled option: %c\n", opts[i].opt);
                break;
        }
    }

    return 0;
}
