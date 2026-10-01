#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>


int main(int argc, char *argv[]) {
    const struct termios oldSetting;
    tcgetattr(STDIN_FILENO, &oldSetting);

    struct termios newSetting = oldSetting;


    // newSetting.c_cc[VMIN] = 1;
    // newSetting.c_cc[VTIME] = 0;

    // newSetting.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
    newSetting.c_lflag &= ~(ECHO | ICANON); // without command
    newSetting.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

    // printf("%b %b\n",ECHO , ICANON);
    // printf("%b \n",ECHO | ICANON);
    // printf("%b \n", ~(ECHO | ICANON));

    tcsetattr(STDIN_FILENO, TCSANOW, &newSetting);

    printf("continue? (y/n) ");

    char result;
    scanf("%c", &result);

    if (result != 'y') printf("Stop\n");

    tcsetattr(STDIN_FILENO, TCSANOW, &oldSetting);
    printf("\n");
    return 0;
}
