#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>

#include "MyList.h"

#define LINE_SIZE 4096

void replace_escape_sequences(char *str) {
    char temp[LINE_SIZE];
    int i = 0;
    int j = 0;
    int len = strlen(str);
    while (i < len) {
        if (str[i] == '\033' && (i + 1) < len && str[i + 1] == '[') {
            const char *marker = "{COM}";
            int marker_len = strlen(marker);
            for (int m = 0; m < marker_len; m++) {
                temp[j++] = marker[m];
            }
            i += 2;
            while (i < len && (str[i] < 0x40 || str[i] > 0x7E)) {
                i++;
            }
            if (i < len) {
                i++;
            }
        } else {
            temp[j++] = str[i++];
        }
    }
    temp[j] = '\0';
    strcpy(str, temp);
}


int main(int argc, char *argv[]) {
    printf("write some lines, for exit write {.} in start: \n");

    List *list = newList();
    char *buffer = malloc(sizeof(char) * LINE_SIZE);


    fgets(buffer, LINE_SIZE, stdin);
    buffer[1023] = '\0';
    replace_escape_sequences(buffer);

    while (buffer[0] != '.') {
        int size = strlen(buffer);
        list = addNode(list, buffer, size);

        fgets(buffer, LINE_SIZE, stdin);
        buffer[LINE_SIZE - 1] = '\0';
        replace_escape_sequences(buffer);
    }

    free(buffer);
    fclose(stdin);

    printf("\n ========== \t OUTPUT \t ========== \n");
    printList(list);

    return 0;
}
