#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Node {
    struct Node *next;
    char *str;
} Node;

int main(int argc, char *argv[]) {
    Node *head = NULL;
    Node *tail = NULL; 

    char buffer[1024];

    printf("Enter strings. Enter '.' as string to exit program.\n");

    while (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        if (buffer[0] == '.')
            break;
        
        size_t len = strlen(buffer);

        buffer[len - 1] = '\0';
        len--;

        Node *new_node = malloc(sizeof(Node));
        if (new_node == NULL) {
            fprintf(stderr, "Node memory allocate error\n");
            return 1;
        }

        new_node->str = malloc(len + 1);
        if (new_node->str == NULL) {
            fprintf(stderr, "String memory allocate error\n");
            return 1;
        }
        
        strcpy(new_node->str, buffer);
        new_node->next = NULL;

        if (head == NULL){
            head = tail = new_node;
        }
        else {
            tail->next = new_node;
            tail = new_node;
        }
    }

    printf("OUTPUT\n");

        Node *current = head;
        while (current != NULL) {
            printf("%s\n", current->str);
            current = current->next;
        }
        
        current = head;
        while (current != NULL) {
            Node *temp = current;
            current = current->next;
            free(temp->str); 
            free(temp);       
        }

    return 0;
}
