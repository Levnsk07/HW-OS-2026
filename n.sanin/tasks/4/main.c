#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct node {
    char *str;
    struct node *next;
};

int main(void) {
    char buf[4096];
    struct node *head = NULL;
    struct node *tail = NULL;

    while (fgets(buf, sizeof buf, stdin) != NULL) {
        if (buf[0] == '.')
            break;

        size_t len = strlen(buf);

        char *copy = malloc(len + 1);
        if (copy == NULL) {
            perror("malloc");
            return 1;
        }
        strcpy(copy, buf);

        struct node *n = malloc(sizeof *n);
        if (n == NULL) {
            perror("malloc");
            free(copy);
            return 1;
        }

        n->str = copy;
        n->next = NULL;

        if (head == NULL)
            head = n;
        else
            tail->next = n;
        tail = n;
    }

    for (struct node *p = head; p != NULL; p = p->next)
        fputs(p->str, stdout);

    while (head != NULL) {
        struct node *next = head->next;
        free(head->str);
        free(head);
        head = next;
    }

    return 0;
}
