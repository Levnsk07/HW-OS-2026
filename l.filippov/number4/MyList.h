//
// Created by leo on 05.10.2026.
//

#ifndef OS_MYLIST_H
#define OS_MYLIST_H


typedef struct Node {
    char *data;
    struct Node *next;
} Node;

typedef struct List {
    Node *head;
    Node *tail;
} List;


List *newList();

Node *newNode(char *data, int size);

List *addNode(List *list, char *data, int size);

void printList(List *list);


#endif //OS_MYLIST_H
