#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 1024

struct node {
    char *str;
    struct node *next;
};

static void sanitize(char *s)
{
    size_t i = 0, j = 0;

    while (s[i] != '\0') {
        unsigned char c = (unsigned char)s[i];

        if (c == 0x1b) {
            i++;
            if (s[i] == '[') {
                i++;
                while (s[i] != '\0' && !(s[i] >= '@' && s[i] <= '~')) {
                    i++;
                }
                if (s[i] != '\0') i++;
            } else if (s[i] == 'O') {
                i++;
                if (s[i] != '\0') i++;
            } else if (s[i] != '\0') {
                i++;
            }
            continue;
        }

        if (c == '\n' || (c >= 0x20 && c <= 0x7e)) {
            s[j++] = (char)c;
        }
        i++;
    }
    s[j] = '\0';
}

static struct node *list_add(struct node *head, const char *s)
{
    struct node *n = malloc(sizeof(struct node));
    if (n == NULL) {
        perror("malloc");
        return head;
    }
    n->str = malloc(strlen(s) + 1);
    if (n->str == NULL) {
        perror("malloc");
        free(n);
        return head;
    }
    strcpy(n->str, s);
    n->next = head;
    return n;
}

static void list_print(const struct node *head)
{
    const struct node *p;
    for (p = head; p != NULL; p = p->next) {
        printf("%s", p->str);
    }
}

static void list_free(struct node *head)
{
    while (head != NULL) {
        struct node *next = head->next;
        free(head->str);
        free(head);
        head = next;
    }
}

int main(void)
{
    char buf[MAX_LINE];
    struct node *head = NULL;

    while (fgets(buf, sizeof(buf), stdin) != NULL) {
        sanitize(buf);
        if (buf[0] == '.') {
            break;
        }
        head = list_add(head, buf);
    }

    list_print(head);
    list_free(head);

    return 0;
}

