#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <sys/types.h>
#include <sys/resource.h>

extern char **environ;

static void printHelp(const char *prog)
{
    fprintf(stderr,
            "Usage: %s [options]\n"
            "\t -i             Print real/effective UID and GID\n"
            "\t -s             Make process a process-group leader (setpgid)\n"
            "\t -p             Print PID, PPID and PGID\n"
            "\t -u             Print RLIMIT_NOFILE (ulimit)\n"
            "\t -Unew_ulimit   Change RLIMIT_NOFILE soft limit\n"
            "\t -c             Print core-file size limit\n"
            "\t -Csize         Change core-file size limit\n"
            "\t -d             Print current working directory\n"
            "\t -v             Print environment variables\n"
            "\t -Vname=value   Set or change an environment variable\n",
            prog);
}

static void print_rlim(const char *name, rlim_t value)
{
    if (value == RLIM_INFINITY)
        printf("%s: unlimited\n", name);
    else
        printf("%s: %llu\n", name, (unsigned long long)value);
}

static int parse_rlim(const char *s, rlim_t *value)
{
    char *end = NULL;
    long long v = strtoll(s, &end, 10);

    if (end == s || *end != '\0')
        return -1;

    if (v < 0)
        *value = RLIM_INFINITY;
    else
        *value = (rlim_t)v;

    return 0;
}

int main(int argc, char *argv[])
{
    const char *short_opts = "ispuU:cC:dV:v";

    struct option long_opts[] = {
        {"Unew_ulimit", required_argument, NULL, 'U'},
        {"Csize",       required_argument, NULL, 'C'},
        {"Vname",       required_argument, NULL, 'V'},
        {NULL, 0, NULL, 0}
    };

    int opt;
    int long_index = 0;
    struct rlimit lim;
    char cwd[4096];

    if (argc < 2) {
        printHelp(argv[0]);
        return 1;
    }

    while ((opt = getopt_long(argc, argv, short_opts, long_opts, &long_index)) != -1) {
        switch (opt) {
        case 'i':
            printf("Real UID: %ld\n", (long)getuid());
            printf("Effective UID: %ld\n", (long)geteuid());
            printf("Real GID: %ld\n", (long)getgid());
            printf("Effective GID: %ld\n", (long)getegid());
            break;

        case 's':
            if (setpgid(0, 0) == -1) {
                perror("setpgid");
            } else {
                printf("Process became a process-group leader\n");
            }
            break;

        case 'p':
            printf("PID: %ld\n", (long)getpid());
            printf("PPID: %ld\n", (long)getppid());
            printf("PGID: %ld\n", (long)getpgrp());
            break;

        case 'u': {
            long max_procs;
            max_procs = sysconf(_SC_CHILD_MAX);
            printf("RLIMIT: %ld\n", max_procs);
            break;
        }

        case 'U': {
            rlim_t new_limit;

            if (parse_rlim(optarg, &new_limit) == -1) {
                fprintf(stderr, "Invalid limit: %s\n", optarg);
                break;
            }

            if (getrlimit(RLIMIT_NOFILE, &lim) == -1) {
                perror("getrlimit(RLIMIT_NOFILE)");
                break;
            }

            lim.rlim_cur = new_limit;

            if (setrlimit(RLIMIT_NOFILE, &lim) == -1) {
                perror("setrlimit(RLIMIT_NOFILE)");
                break;
            }

            print_rlim("RLIMIT_NOFILE soft", lim.rlim_cur);
            print_rlim("RLIMIT_NOFILE hard", lim.rlim_max);
            break;
        }

        case 'c':
            if (getrlimit(RLIMIT_CORE, &lim) == -1) {
                perror("getrlimit(RLIMIT_CORE)");
                break;
            }

            print_rlim("RLIMIT_CORE soft", lim.rlim_cur);
            print_rlim("RLIMIT_CORE hard", lim.rlim_max);
            break;

        case 'C': {
            rlim_t new_size;

            if (parse_rlim(optarg, &new_size) == -1) {
                fprintf(stderr, "Invalid core size: %s\n", optarg);
                break;
            }

            if (getrlimit(RLIMIT_CORE, &lim) == -1) {
                perror("getrlimit(RLIMIT_CORE)");
                break;
            }

            lim.rlim_cur = new_size;
            lim.rlim_max = new_size;

            if (setrlimit(RLIMIT_CORE, &lim) == -1) {
                perror("setrlimit(RLIMIT_CORE)");
                break;
            }

            if (getrlimit(RLIMIT_CORE, &lim) == -1) {
                perror("getrlimit(RLIMIT_CORE)");
                break;
            }

            print_rlim("RLIMIT_CORE soft", lim.rlim_cur);
            print_rlim("RLIMIT_CORE hard", lim.rlim_max);
            break;
        }

        case 'd':
            if (getcwd(cwd, sizeof(cwd)) == NULL) {
                perror("getcwd");
            } else {
                printf("%s\n", cwd);
            }
            break;

        case 'v': {
            char **ep;

            for (ep = environ; *ep != NULL; ++ep) {
                printf("%s\n", *ep);
            }
            break;
        }

        case 'V': {
            char *eq = strchr(optarg, '=');

            if (eq != NULL) {
                *eq = '\0';

                if (setenv(optarg, eq + 1, 1) == -1) {
                    perror("setenv");
                } else {
                    printf("Set %s=%s\n", optarg, eq + 1);
                }
            } else if (optind < argc) {
                const char *name = optarg;
                const char *value = argv[optind++];

                if (setenv(name, value, 1) == -1) {
                    perror("setenv");
                } else {
                    printf("Set %s=%s\n", name, value);
                }
            } else {
                fprintf(stderr, "-V requires name=value or name value\n");
            }
            break;
        }

        case '?':
        default:
            fprintf(stderr, "Invalid option: -%c\n", optopt);
            printHelp(argv[0]);
            return 1;
        }
    }

    return 0;
}