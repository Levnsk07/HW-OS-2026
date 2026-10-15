#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE_LEN 256

typedef struct Node {
    char *str;
    struct Node *next;
} Node;

int main(void) {
    char buffer[MAX_LINE_LEN];
    Node *head = NULL;
    Node *tail = NULL;

    while (fgets(buffer, sizeof(buffer), stdin)) {
        if (buffer[0] == '.')
            break;

        buffer[strcspn(buffer, "\n")] = '\0';

        Node *node = malloc(sizeof(Node));
        if (node == NULL)
            return 1;

        node->str = malloc(strlen(buffer) + 1);
        if (node->str == NULL)
            return 1;

        strcpy(node->str, buffer);
        node->next = NULL;

        if (head == NULL)
            head = node;
        else
            tail->next = node;

        tail = node;
    }

    Node *cur = head;

    while (cur != NULL) {
        printf("%s\n", cur->str);

        Node *next = cur->next;
        free(cur->str);
        free(cur);
        cur = next;
    }

    return 0;
}
