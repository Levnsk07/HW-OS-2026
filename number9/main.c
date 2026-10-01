#include <sys/types.h>
#include <sys/wait.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char *argv[]) {
    pid_t pid = fork(); // Создаем новый процесс

    if (pid < 0) {
        perror("fork failed");
        exit(1);
    } else if (pid == 0) {
        printf("Это дочерний процесс. PID: %d\n", getpid());

        system("cat bigFile.txt");

        printf("Дочерний процесс завершает работу.\n");
        exit(0);
    } else {
        printf("Это родительский процесс. PID: %d, PID потомка: %d\n", getpid(), pid);


        wait(NULL); // результат работы дочернего процесса, null - любой
        // waitpid(pid, NULL, 0); // выбор определённого процесса, действие после выполнения процесса, способ остоновки
        // waitid(P_PID, getpid(), NULL,WEXITED); // Расширенный контроль, определённый Процесс, с определёным способом завершения, с подробным получением данных


        printf("Родительский процесс понял, что потомок завершился.\n");
    }

    return 0;
}
