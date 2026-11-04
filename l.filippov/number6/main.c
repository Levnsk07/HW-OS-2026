#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

volatile sig_atomic_t flag = 0;

void stop(int signum) {
    flag = 1;
}

int main() {
    struct sigaction sa;
    sa.sa_handler = stop;
    sigemptyset(&sa.sa_mask);

    // ВАЖНО: sa_flags равен 0. Мы НЕ ставим SA_RESTART.
    // Это заставит scanf прерваться при получении сигнала.
    sa.sa_flags = 0;

    sigaction(SIGALRM, &sa, NULL);

    char *some = calloc(0, sizeof(char) * 1024);
    alarm(5);
    scanf("%s", some);
    pause();

    if (some[0] != 0) {
        printf("well well well\n");
    } else if (flag) {
        printf("Time Over\n");
    }

    free(some);
    return 0;
}
