#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXLINE 1024

struct node {
    char *str;
    struct node *next;
};

void strip_escapes(char *buf)
{
    char *src = buf;
    char *dst = buf;

    while (*src)
    {
        if (*src == '\033')
        {
            ++src;
            if (*src == '[')
            {
                ++src;
                while (*src && (*src < '@' || *src > '~'))
                    ++src;
                if (*src) ++src;
            }
        }
        else *dst++ = *src++;
    }
    *dst = '\0';
}

int main(void)
{
    char buf[MAXLINE];
    struct node *head = NULL;
    struct node *tail = NULL;

    printf("Enter strings (. at start to finish):\n");

    while (1)
    {
        if (fgets(buf, MAXLINE, stdin) == NULL) break;

        strip_escapes(buf);

        if (buf[0] == '.') break;

        size_t len = strlen(buf);
        if (len > 0 && buf[len-1] == '\n')
        {
            buf[len-1] = '\0';
            --len;
        }

        if (len == 0) continue;

        char *s = malloc(len + 1);
        if (s == NULL)
        {
            perror("malloc");
            break;
        }
        strcpy(s, buf);

        struct node *n = malloc(sizeof(struct node));
        if (n == NULL)
        {
            perror("malloc");
            free(s);
            break;
        }
        n->str = s;
        n->next = NULL;

        if (head == NULL)
        {
            head = n;
            tail = n;
        }
        else
        {
            tail->next = n;
            tail = n;
        }
    }

    printf("\nAll strings:\n");
    for (struct node *s = head; s != NULL; s = s->next)
    {
        printf("%s\n", s->str);
    }

    struct node *p = head;
    while (p != NULL)
    {
        struct node *next = p->next;
        free(p->str);
        free(p);
        p = next;
    }

    return 0;
}
