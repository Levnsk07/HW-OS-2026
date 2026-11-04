#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <sys/resource.h>

extern char **environ;

#define MAX_OPTS 100

struct opt_entry {
    int opt;
    char *arg;
};

struct opt_entry opts[MAX_OPTS];
int n_opts = 0;

void do_i(void)
{
    printf("uid=%d, euid=%d, gid=%d, egid=%d\n",
           (int)getuid(), (int)geteuid(),
           (int)getgid(), (int)getegid());
}

void do_s(void)
{
    if (setpgid(0, 0) == -1) {
        perror("setpgid");
    } else {
        printf("%d\n", (int)getpgrp());
    }
}

void do_p(void)
{
    printf("pid = %d, ppid = %d, pgid = %d\n",
           (int)getpid(), (int)getppid(), (int)getpgrp());
}

void do_d(void)
{
    char buf[PATH_MAX];

    if (getcwd(buf, sizeof(buf)) == NULL) {
        perror("getcwd");
    } else {
        printf("cwd=%s\n", buf);
    }
}

void do_u(void)
{
    struct rlimit rl;
    if (getrlimit(RLIMIT_NOFILE, &rl) == -1) {
        perror("getrlimit");
    } else {
        printf("current limit = %ld, max limit = %ld\n",
               (long)rl.rlim_cur, (long)rl.rlim_max);
    }
}

void do_U(char *arg)
{
    struct rlimit rl;
    char *end;
    long val;

    errno = 0;
    val = strtol(arg, &end, 10);

    if (errno != 0 || *end != '\0' || end == arg || val < 0) {
        fprintf(stderr, "Invalid value: %s\n", arg);
        return;
    }

    if (getrlimit(RLIMIT_NOFILE, &rl) == -1) {
        perror("getrlimit");
        return;
    }

    rl.rlim_cur = (rlim_t)val;

    if (setrlimit(RLIMIT_NOFILE, &rl) == -1) {
        perror("setrlimit");
    } else {
        printf("ulimit set to %ld\n", val);
    }
}

void do_c(void)
{
    struct rlimit rl;
    if (getrlimit(RLIMIT_CORE, &rl) == -1) {
        perror("getrlimit");
    } else {
        printf("core size: soft = %ld, hard = %ld\n",
               (long)rl.rlim_cur, (long)rl.rlim_max);
    }
}

void do_C(char *arg)
{
    struct rlimit rl;
    long val = atol(arg);
    if (val < 0) {
        fprintf(stderr, "Invalid value: %s\n", arg);
        return;
    }

    if (getrlimit(RLIMIT_CORE, &rl) == -1) {
        perror("getrlimit");
        return;
    }

    rl.rlim_cur = (rlim_t)val;

    if (setrlimit(RLIMIT_CORE, &rl) == -1) {
        perror("setrlimit");
    } else {
        printf("core size set to %ld\n", val);
    }
}

void do_v(void)
{
    for (char **e = environ; *e != NULL; e++) {
        printf("%s\n", *e);
    }
}

void do_V(char *arg)
{
    if (strchr(arg, '=') == NULL) {
        fprintf(stderr, "-V: expected NAME=value\n");
        return;
    }
    if (putenv(arg) != 0) {
        perror("putenv");
    } else {
        printf("set: %s\n", arg);
    }
}

int main(int argc, char *argv[])
{
    char *options = "ispuU:cC:dvV:";
    int c;

    while ((c = getopt(argc, argv, options)) != -1) {
        if (c == '?') {
            fprintf(stderr, "Invalid option\n");
            exit(1);
        }
        opts[n_opts].opt = c;
        opts[n_opts].arg = optarg;
        n_opts++;
    }

    for (int i = n_opts - 1; i >= 0; i--) {
        switch (opts[i].opt) {
            case 'i': do_i(); break;
            case 's': do_s(); break;
            case 'p': do_p(); break;
            case 'u': do_u(); break;
            case 'U': do_U(opts[i].arg); break;
            case 'c': do_c(); break;
            case 'C': do_C(opts[i].arg); break;
            case 'd': do_d(); break;
            case 'v': do_v(); break;
            case 'V': do_V(opts[i].arg); break;
        }
    }

    return 0;
}
