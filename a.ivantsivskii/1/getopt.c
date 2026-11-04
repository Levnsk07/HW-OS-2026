#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>
#include <string.h>
int main(int argc, char *argv[]){
    int c;
    extern char *optarg;
    extern int optind;
    while ((c = getopt(argc, argv, "ispuU:cC:dvV:")) != -1) {
        if(c=='U'||c=='C'||c=='V'){
            argv[optind-1]=argv[optind-2];
            argv[optind-2]=optarg;
        }
    }
    char **argv2=calloc(argc+1,sizeof(char*));
    optind=1;
    argv2[0]=argv[0];
    for(int i=1;i<argc;i++)argv2[i]=argv[argc-i];
    while ((c = getopt(argc, argv2, "ispuU:cC:dvV:")) != -1) {
        switch (c) {
            case 'i':{  
                printf("uid=%u euid=%u gid=%u egid=%u\n",getuid(),geteuid(),getgid(),getegid());
                break;
            }
            case 's':{
                setpgid(0, 0);
                break;
            }
            case 'p':{
                printf("pid=%d ppid=%d pgid=%d\n",getpid(),getppid(),getpgid(0));
                break;
            }
            case 'u':{
                long lim;
                lim = sysconf(_SC_CHILD_MAX);
                if(lim<0)printf("unlimited\n");
                else printf("%ld\n",lim);
            break;
            }
            case 'U': {
                long lim;
                lim = atol(optarg);
                (void)lim;
                fprintf(stderr, "setrlimit: operation not supported\n");
                break;
            }
            case 'c':{
                struct rlimit rl;
                if (getrlimit(RLIMIT_CORE, &rl) == 0) {
                    if((long)rl.rlim_cur<0)printf("unlimited\n");
                    else printf("%ld\n",(long)rl.rlim_cur);
                }
                break;
            }
            case 'C': {
                struct rlimit rl;
                if (getrlimit(RLIMIT_CORE, &rl) == 0) {
                    rl.rlim_cur = atoi(optarg);
                    if (setrlimit(RLIMIT_CORE, &rl) == -1) {
                        perror("setrlimit");
                    }
                }
                break;
            }
            case 'd':{
                char buf[4096];
                if (getcwd(buf, sizeof(buf)) != NULL) {
                    printf("%s\n", buf);
                }
                break;
            }
            case 'v':{
                extern char **environ;
                for (char **env = environ; *env != NULL; env++) {
                    printf("%s\n", *env);
                }
                break;
            }
            case 'V':{
                char *eq = strchr(optarg, '=');
                char *buf1=calloc(strlen(optarg),sizeof(char)),*buf2=calloc(strlen(optarg),sizeof(char));
                strncpy(buf1,optarg,eq-optarg);
                strcpy(buf2,optarg+(eq-optarg)+1);
                setenv(buf1, buf2, 1);
                break;
            }
        }
    }
}   