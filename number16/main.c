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

    newSetting.c_lflag &= ~(ECHO | ICANON | ISIG | IEXTEN);
    newSetting.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);

    tcsetattr(STDIN_FILENO, TCSANOW, &newSetting);


    printf("continue (y/n) ");

    char result;
    scanf("%c", &result);

    if (result == '\n') printf("n");

    printf("\n %c \n", result);
    // cc_t
    // VMIN

    tcsetattr(STDIN_FILENO, TCSANOW, &oldSetting);

    return 0;
}
