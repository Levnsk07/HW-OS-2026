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
    struct node *n;
    struct node *p;
    char *s;
    size_t len;

    while (fgets(buf, sizeof buf, stdin) != NULL) {
        len = strlen(buf);

        if (len > 0 && buf[len - 1] == '\n') {
            buf[len - 1] = '\0';
            --len;
        }

        if (buf[0] == '.') {
            break;
        }

        n = malloc(sizeof *n);
        s = malloc(len + 1);
        
        strcpy(s, buf);

        n->str = s;
        n->next = NULL;

        if (head == NULL) {
            head = n;
            tail = n;
        } else {
            tail->next = n;
            tail = n;
        }
    }

    for (p = head; p != NULL; p = p->next) {
        printf("%s\n", p->str);
    }

    while (head != NULL) {
    p = head;
    head = head->next;
    free(p->str);
    free(p);
    }

    return 0;
}