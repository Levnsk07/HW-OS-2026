#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LINE 4096

struct node {
        char *str;
        size_t len;
        struct node *next;
};

int main(void) {
        char buffer[MAX_LINE];
        struct node *head = NULL;
        struct node *tail = NULL;

        while (fgets(buffer, MAX_LINE, stdin) != NULL) {
                if (buffer[0] == '.') {
                        break;
                }

                size_t w = 0;
                char *read_ptr = buffer;

                while (*read_ptr != '\0') {
                        if (*read_ptr == '\x1b') {
                                read_ptr++;
                                if (*read_ptr == '[') { 
                                        read_ptr++;
                                        while (*read_ptr != '\0' && (*read_ptr < 'A' || *read_ptr > 'Z') && (*read_ptr < 'a' || *read_ptr > 'z') && *read_ptr != '~') {
                                                read_ptr++;
                                        }
                                        if (*read_ptr != '\0') {
                                                read_ptr++;
                                        }
                                }
                                continue;
                        }

                        if (*read_ptr != '\n' && *read_ptr != '\0') {
                                buffer[w++] = *read_ptr;
                        }
                        read_ptr++;
                }
                buffer[w] = '\0';

                size_t len = w;

                if (len > 0 && buffer[len - 1] == '\n') {
                        buffer[len - 1] = '\0';
                        len--;
                }

                struct node *node = malloc(sizeof(struct node));
                if (node == NULL) {
                        perror("malloc failed");
                        return 1;
                }

                node->str = malloc(len + 1);
                if (node->str == NULL) {
                        perror("malloc failed");
                        free(node);
                        return 1;
                }

                memcpy(node->str, buffer, len + 1);
                node->len = len;
                node->next = NULL;

                if (head == NULL) {
                        head = node;
                        tail = node;
                } else {
                        tail->next = node;
                        tail = node;
                }
        }

        for (struct node *cur = head; cur != NULL; cur = cur->next) {
                printf("%s\n", cur->str);
        }

        while (head != NULL) {
                struct node *next = head->next;
                free(head->str);
                free(head);
                head = next;
        }

        return 0;
}

