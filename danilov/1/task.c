#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/resource.h>

extern char **environ;

static void usage(char *prog){
	printf("использование: %s [-i] [-s] [-p] [-u] [-Uлимит] [-c] [-Cразмер]"
		" [-d] [-v] [-Vимя=значение]\n", prog);
}

static void show_limit(int res, char *label){
	struct rlimit rl;

	if (getrlimit(res, &rl) != 0) {
		perror("getrlimit");
		return;
	}
	if (rl.rlim_cur == RLIM_INFINITY)
		printf("%s: unlimited\n", label);
	else
		printf("%s: %ld байт\n", label, (long)rl.rlim_cur);
}

static void change_limit(int res, char *label, char *arg){
	struct rlimit rl;
	char *end;
	long v;

	errno = 0;
	v = strtol(arg, &end, 10);
	if (*arg == '\0' || *end != '\0' || errno == ERANGE) {
		fprintf(stderr, "ошибка: \"%s\" не является числом\n", arg);
		return;
	}
	if (getrlimit(res, &rl) != 0) {
		perror("getrlimit");
		return;
	}
	rl.rlim_cur = (rlim_t)v;
	if (setrlimit(res, &rl) != 0) {
		perror("setrlimit");
		return;
	}
	printf("%s теперь %ld байт\n", label, v);
}

int main(int argc, char *argv[]){
	char buf[PATH_MAX];
	char *name, *eq;
	int c;

	if (argc < 2) {
		usage(argv[0]);
		return 2;
	}

	while ((c = getopt(argc, argv, "ispuU:cC:dvV:")) != -1) {
		switch (c) {
		case 'i':
			printf("real uid=%ld gid=%ld\n", (long)getuid(), (long)getgid());
			printf("eff  uid=%ld gid=%ld\n", (long)geteuid(), (long)getegid());
			break;
		case 's':
			if (setpgid(0, 0) != 0)
				perror("setpgid");
			printf("pgid=%ld pid=%ld\n", (long)getpgrp(), (long)getpid());
			break;
		case 'p':
			printf("pid=%ld ppid=%ld pgid=%ld\n",
				(long)getpid(), (long)getppid(), (long)getpgrp());
			break;
		case 'u':
			show_limit(RLIMIT_FSIZE, "ulimit");
			break;
		case 'U':
			change_limit(RLIMIT_FSIZE, "ulimit", optarg);
			break;
		case 'c':
			show_limit(RLIMIT_CORE, "core");
			break;
		case 'C':
			change_limit(RLIMIT_CORE, "core", optarg);
			break;
		case 'd':
			if (getcwd(buf, sizeof buf) == NULL)
				perror("getcwd");
			else
				printf("%s\n", buf);
			break;
		case 'v':
			for (char **e = environ; *e != NULL; e++)
				printf("%s\n", *e);
			break;
		case 'V':
			eq = strchr(optarg, '=');
			if (eq == NULL || eq == optarg) {
				fprintf(stderr, "ошибка: нужно имя=значение\n");
				break;
			}
			name = malloc(eq - optarg + 1);
			strncpy(name, optarg, eq - optarg);
			name[eq - optarg] = '\0';
			if (setenv(name, eq + 1, 1) != 0)
				perror("setenv");
			free(name);
			break;
		default:
			usage(argv[0]);
			return 2;
		}
	}
	return 0;
}

