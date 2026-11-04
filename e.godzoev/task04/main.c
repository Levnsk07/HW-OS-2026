#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void check_arrows(char *str) {
    char temp[1024];
    int i = 0;
    int j = 0;
    int len = strlen(str);
    while (i < len) {
        if (str[i] == '\033' && (i + 1) < len && str[i + 1] == '[') {
            const char *marker = "{COM}";
            int marker_len = strlen(marker);
            for (int m = 0; m < marker_len; m++) {
                temp[j++] = marker[m];
            }
            i += 2;
            while (i < len && (str[i] < 0x40 || str[i] > 0x7E)) {
                i++;
            }
            if (i < len) {
                i++;
            }
        } else {
            temp[j++] = str[i++];
        }
    }
    temp[j] = '\0';
    strcpy(str, temp);
}

typedef struct Node{
    int length;
    char *a;
    struct Node *next;
} Node;

typedef struct{
    Node *head;
    Node *tail;
} List;

Node *makeNode(int len, char *str){
    Node *node = malloc(sizeof(Node));
    node->a = malloc(sizeof(char) * (len + 1));
    memcpy(node->a, str, len);
    node->a[len] = '\0';
    node->length = len;
    node->next = NULL;
    return node;
}

void addToList(List *list, int len, char *str){
    Node *new = makeNode(len, str);
    if (list->head == NULL){
        list->head = new;
        list->tail = new;
    }
    else{
        list->tail->next = new;
        list->tail = new;
        new->next = NULL;
    }
}

int main(void){
    char str[1024];

    List list;
    list.head = NULL;
    list.tail = NULL;

    while(fgets(str, 1024, stdin) && str[0] != '.'){
        str[strcspn(str, "\n")] = '\0';
        check_arrows(str);
        int len = strlen(str);
        addToList(&list, len, str);
    }

    while(list.head != NULL){
        Node *cur = list.head;
        printf("%s\n", cur->a);
        list.head = cur->next;
        free(cur->a);
        free(cur);
    }

    return 0;
}