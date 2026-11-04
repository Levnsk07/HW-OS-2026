#include "MyList.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// typedef struct Node {
//     char *data;
//     struct Node *next;
// } Node;
//
// typedef struct List {
//     Node *head;
//     Node *tail;
// } List;


List *newList() {
    List *list;
    list = (List *) malloc(sizeof(List));
    list->head = NULL;
    list->tail = list->head;
    return list;
}

Node *newNode(char *data, int size) {
    Node *node;
    node = (Node *) malloc(sizeof(Node));
    node->data = malloc((size + 1) * sizeof(char));

    strcpy(node->data, data);

    node->next = NULL;
    return node;
}

List *addNode(List *list, char *data, int size) {
    if (list->tail == NULL) {
        list->head = newNode(data, size);
        list->tail = list->head;
        return list;
    }
    list->tail->next = newNode(data, size);
    list->tail = list->tail->next;
    return list;
}

void printList(List *list) {
    if (list == NULL) return;
    if (list->head == NULL) return;
    Node *current = list->head;
    do {
        printf("%s", current->data);
        // printf("\\x%02X", current->data);
        // printf("\n", current->data);
        current = current->next;
    } while (current != NULL);
}
