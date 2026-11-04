#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LEN 4096

typedef struct Node {
    char *str;
    size_t len;
    struct Node *next;
} Node;

int main() {
    char buffer[MAX_LEN];
    Node *head = NULL, *tail = NULL;

    while (fgets(buffer, MAX_LEN, stdin) != NULL) {
        if (buffer[0] == '.')
            break;

        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n')
            buffer[--len] = '\0';

        Node *node = malloc(sizeof(Node));
        node->str = malloc(len + 1);
        strcpy(node->str, buffer);
        node->len = len;
        node->next = NULL;

        if (tail == NULL)
            head = tail = node;
        else {
            tail->next = node;
            tail = node;
        }
    }

    for (Node *n = head; n != NULL; n = n->next)
        printf("%s\n", n->str);

    while (head != NULL) {
        Node *tmp = head;
        head = head->next;
        free(tmp->str);
        free(tmp);
    }

    return 0;
}

