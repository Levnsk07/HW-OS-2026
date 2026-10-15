#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>

volatile sig_atomic_t timed_out = 0;

void alarm_handler(int sig)
{
    timed_out = 1;
}
int main(int argc, char *argv[]) {

    struct sigaction sa;

    sa.sa_handler = alarm_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGALRM, &sa, NULL);

    char c, buffer[10001];
    off_t  offsets[1000];
    int lengths[1000];

    if (argc < 2) {
        printf("format: %s <filename>\n", argv[0]);
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);

    offsets[0] = 0;
    int line = 0;
    while (read(fd, &c, 1) == 1) {
        if (c == '\n') {
            off_t current_pos = lseek(fd, 0L, SEEK_CUR);
            lengths[line] = current_pos - offsets[line];
            offsets[++line] = current_pos;
        }
        
    }
    
    
    off_t end_pos = lseek(fd, 0L, SEEK_CUR);
        if (end_pos > offsets[line]) {
            lengths[line] = end_pos - offsets[line]  ;
            line++;
        }

    int index;
    while (1) {
        printf("срочно введи номер строки или 0 для выхода: ");
        timed_out = 0;
        alarm(5);
        scanf("%d" , &index);
    


        alarm(0);

        if (timed_out) {
            int n;
            lseek(fd, 0, SEEK_SET);
            while ((n = read(fd, buffer, sizeof(buffer)-1)) > 0) {
                buffer[n] = '\0';
                printf("%s", buffer);
            }
            return 0;
        }

        if (index == 0)
            break;
        
        index--;
        lseek(fd, offsets[index], SEEK_SET);
       
        read(fd, buffer, lengths[index]);
        buffer[lengths[index]] = '\0';
        printf("%s", buffer);
    }

  
}
