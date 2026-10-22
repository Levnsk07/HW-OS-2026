#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <limits.h>
#include <string.h>
#include <errno.h>

extern char **environ;

#ifndef ARG_MAX
        #define ARG_MAX sysconf(_SC_ARG_MAX)
#endif

struct opt_record {
        int opt;
        char *arg;
};

void print_usage(char *argv0) {
        printf("Usage: %s [-i] [-s] [-p] [-u] [-U <new_limit>] [-c] [-C <size>] [-d] [-v] [-V <name=value>]\n", argv0);
        printf("  -i: Print real and effective identification of user and group.\n");
        printf("  -s: Make process a group leader.\n");
        printf("  -p: Print identification of process, its parent and group.\n");
        printf("  -u: Print ulimit value.\n");
        printf("  -U <new_ulimit>: Change ulimit value.\n");
        printf("  -c: Print core file size (in bytes).\n");
        printf("  -C <size>: Change core file size.\n");
        printf("  -d: Print current working directory.\n");
        printf("  -v: Print environment variables and their values.\n");
        printf("  -V <name=value>: Create new environment variable or change already existing one.\n");
}

#ifdef __sun
#include <rctl.h>
#include <signal.h>

extern size_t rctlblk_size(void);

static int get_nproc(rlim_t *value) {
        long max_procs = sysconf(_SC_CHILD_MAX);

        if (max_procs == -1) {
                return -1;
        }

        *value = (rlim_t)max_procs;

        return 0;
}

static int set_nproc(rlim_t value) {
        rctlblk_t *rblk;

        if ((rblk = malloc(rctlblk_size())) == NULL) {
                return -1;
        }

        rctlblk_set_value(rblk, value);
        rctlblk_set_privilege(rblk, RCPRIV_PRIVILEGED);
        rctlblk_set_local_action(rblk, RCTL_LOCAL_SIGNAL, SIGXRES);

        int ret = setrctl("project.max-processes", NULL, rblk, RCTL_INSERT);

        free(rblk);

        return ret;
}
#endif

int main(int argc, char *argv[]) {
        const char *options = "ispuU:cC:dvV:";
        struct opt_record records[argc];
        int n = 0;
        int opt;
        struct rlimit limit;
        char *end;

        while ((opt = getopt(argc, argv, options)) != -1) {
                if (opt == '?') {
                        print_usage(argv[0]);
                        return 1;
                }
                records[n].opt = opt;
                records[n].arg = optarg;
                n++;
        }

        if (!n) {
                print_usage(argv[0]);
                return 1;
        }

        for (int i = n - 1; i >= 0; i--) {
                switch (records[i].opt) {
                        case 'i': {
                                uid_t ruid = getuid();
                                uid_t euid = geteuid();
                                gid_t rgid = getgid();
                                gid_t egid = getegid();

                                printf("\truid\t|\teuid\t|\trgid\t|\tegid\n");
                                printf("\t%lu\t|\t%lu\t|\t%lu\t|\t%lu\n",
                                    (unsigned long)ruid, (unsigned long)euid,
                                    (unsigned long)rgid, (unsigned long)egid);

                                break;
                        }
                        case 's': {
                                if (setpgid(0, 0) == 0) {

                                        perror("setpgid failed");
                                        return 1;
                                }

                                break;
                        }
                        case 'p': {
                                pid_t pid = getpid();
                                pid_t ppid = getppid();
                                pid_t pgid = getpgrp();

                                printf("\tpid\t|\tppid\t|\tpgid\n");
                                printf("\t%d\t|\t%d\t|\t%d\n",
                                    (int)pid, (int)ppid, (int)pgid);

                                break;
                        }
                        case 'u': {
                                #ifdef __sun
                                      rlim_t val;
                                      if (get_nproc(&val) == 0) {
                                              if (val == RLIM_INFINITY) {
                                                      printf("Ulimit value: unlimited\n");
                                              } else {
                                                      printf("Ulimit value: %ld\n", (long)val);
                                              }
                                      } else {
                                              perror("sysconf failed");
                                              return 1;
                                      }
                                #else
                                      if (getrlimit(RLIMIT_NPROC, &limit) == 0) {
                                              if (limit.rlim_cur == RLIM_INFINITY) {
                                                      printf("Ulimit value: unlimited\n");
                                              } else {
                                                      printf("Ulimit value: %ld\n", (long)limit.rlim_cur);
                                              }
                                      } else {
                                              perror("getrlimit failed");
                                              return 1;
                                      }
                                #endif
                                break;
                        }
                        case 'U': {
                                long new_limit;
                                end = NULL;

                                errno = 0;
                                new_limit = strtol(records[i].arg, &end, 10);
                                if (errno != 0 || end == records[i].arg || *end != '\0' || new_limit < 0) {
                                        fprintf(stderr, "Invalid value for -U: %s.\n", records[i].arg);
                                        return 1;
                                }

                                #ifdef __sun
                                        if (set_nproc((rlim_t)new_limit) != 0) {
                                                perror("setrctl failed");
                                                return 1;
                                        }
                                #else
                                        if (getrlimit(RLIMIT_NPROC, &limit) != 0) {
                                                perror("getrlimit failed");
                                                return 1;
                                        }

                                        if ((rlim_t)new_limit > limit.rlim_max) {
                                                fprintf(stderr, "new_limit (%ld) exceeds rlim_max (%ld).\n", new_limit, (long)limit.rlim_max);
                                                return 1;
                                        }

                                        limit.rlim_cur = (rlim_t)new_limit;

                                        if (setrlimit(RLIMIT_NPROC, &limit) != 0) {
                                                perror("setrlimit failed");
                                                return 1;
                                        }
                                #endif

                                break;
                        }
                        case 'c': {
                                if (getrlimit(RLIMIT_CORE, &limit) == 0) {
                                        if (limit.rlim_cur == RLIM_INFINITY) {
                                                printf("Core size: unlimited\n");
                                        } else {
                                                printf("Core size: %ld\n", (long)limit.rlim_cur);
                                        }
                                } else {
                                        perror("getrlimit failed");
                                        return 1;
                                }

                                break;
                        }
                        case 'C': {
                                long long size;
                                end = NULL;

                                errno = 0;
                                size = strtoll(records[i].arg, &end, 10);
                                if (errno != 0 || end == records[i].arg || *end != '\0' || size < 0) {
                                        fprintf(stderr, "Invalid value for -C: %s.\n", records[i].arg);
                                        return 1;
                                }

                                if (getrlimit(RLIMIT_CORE, &limit) != 0) {
                                        perror("getrlimit failed");
                                        return 1;
                                }

                                if ((rlim_t)size > limit.rlim_max) {
                                        fprintf(stderr, "size (%lld) exceeds rlim_max (%lld).\n", size, (long long)limit.rlim_max);
                                        return 1;
                                }

                                limit.rlim_cur = (rlim_t)size;

                                if (setrlimit(RLIMIT_CORE, &limit) != 0) {
                                        perror("setrlimit failed");
                                        return 1;
                                }

                                break;
                        }
                        case 'd': {
                                char cwd[PATH_MAX];
                                if (getcwd(cwd, sizeof(cwd)) != NULL) {
                                        printf("Current working directory: %s\n", cwd);
                                } else {
                                        perror("getcwd failure");
                                        return 1;
                                }

                                break;
                        }
                        case 'v': {
                                printf("Environment variables:\n");
                                for (char **env = environ; *env; ++env) {
                                        printf("%s\n", *env);
                                }

                                break;
                        }
                        case 'V': {
                                const char *equals;
                                size_t name_len, value_len;
                                char *name, *value;

                                if (!records[i].arg) {
                                        print_usage(argv[0]);
                                        return 1;
                                }

                                equals = strchr(records[i].arg, '=');
                                if (equals == NULL) {
                                        fprintf(stderr, "Invalid format for -V (expected name=value): %s.\n", records[i].arg);
                                        return 1;
                                }

                                name_len = equals - records[i].arg;
                                value_len = strlen(equals + 1);

                                if (name_len == 0) {
                                        fprintf(stderr, "Empty variable name in -V.\n");
                                        return 1;
                                }

                                name = malloc(name_len + 1);
                                value = malloc(value_len + 1);
                                if (!name || !value) {
                                        perror("malloc failed");
                                        free(name);
                                        free(value);
                                        return 1;
                                }

                                memcpy(name, records[i].arg, name_len);
                                name[name_len] = '\0';
                                memcpy(value, equals + 1, value_len);
                                value[value_len] = '\0';

                                if (setenv(name, value, 1) != 0) {
                                        perror("setenv failed");
                                        free(name);
                                        free(value);
                                        return 1;
                                }

                                free(name);
                                free(value);

                                break;
                        }
                        case '?':
                        default:
                                print_usage(argv[0]);
                                return 1;
                }
        }

        return 0;
}
