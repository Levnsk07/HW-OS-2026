#include "array.h"

#include <stdlib.h>
#include <string.h>


struct Node* create_node(char* data) {
    struct Node* new_node = malloc(sizeof(struct Node));
    if (!new_node) return NULL;

    size_t n = strlen(data) + 1;
    new_node->data = malloc(n);
    if (!new_node->data) {
        free(new_node);
        return NULL;
    }
    strcpy(new_node->data, data);

    new_node->next = NULL;
    return new_node;
}

struct Node* append_node(struct Node* head, char* data) {
    struct Node* new_node = create_node(data);
    if (!new_node) {
        return NULL;
    }
    if (!head) {
        return new_node;
    }
    struct Node* current = head;
    while (current->next) {
        current = current->next;
    }
    current->next = new_node;
    return head;
}

struct Node *free_list(struct Node *head) {
    struct Node *current = head;
    while (current) {
        struct Node *next = current->next;
        free(current->data);
        free(current);
        current = next;
    }
    return NULL;
}
