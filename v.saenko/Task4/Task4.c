#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct Node {
    char *str;
    struct Node *next;
} Node;


int main() {

    char buffer[1024];
    Node *head = NULL;
    Node *tail = NULL;

    while  (fgets(buffer, sizeof(buffer), stdin) != NULL) {
        if (buffer[0] == '.') {
            break;
        }

        size_t len = strlen(buffer);
        if (len > 0 && buffer[len - 1] == '\n') {
            buffer[len - 1] = '\0';
            len--;
        }

        Node *new_node = (Node *)malloc(sizeof(Node));
        if (new_node == NULL) {
            fprintf(stderr, "Error\n");
            return 1;
        }


        new_node->str = (char *)malloc((len + 1) * sizeof(char));
        if (new_node->str == NULL) {
            fprintf(stderr, "Error\n");
            free(new_node); 
            return 1;
        }


        strcpy(new_node->str, buffer);
        new_node->next = NULL;

        if (head == NULL) {
            head = new_node;
            tail = new_node;
        } else {
            tail->next = new_node;
            tail = new_node;
        }

    }

    

    Node *current = head;
    while (current != NULL) {
	for (int i = 0; current->str[i] != '\0'; i++) {
		if ((unsigned char)current->str[i] >= 32 || current->str[i] == '\t') {
			putchar(current->str[i]);
		}
	}
        putchar('\n');
        current = current->next;
    }

    putchar('\n');

    current = head;
    while (current != NULL) {
        Node *temp = current;     
        current = current->next;  
        
        free(temp->str);          
        free(temp);   
    }            

    return 0;


}
