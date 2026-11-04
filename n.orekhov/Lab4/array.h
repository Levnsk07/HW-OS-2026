#ifndef NSULABOS_ARRAY_H
#define NSULABOS_ARRAY_H
struct Node {
  char *data;
  struct Node *next;
};

struct Node *create_node(char *data);
struct Node *append_node(struct Node *head, char *data);
struct Node *free_list(struct Node *head);

#endif //NSULABOS_ARRAY_H
