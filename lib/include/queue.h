typedef struct node {
    int val;
    struct node * next;
} node_t;


void enqueue(node_t ** head, int val);

void pushQ(node_t ** head, int val);

int isEmptyQ(node_t * head);

int dequeue(node_t ** head);

void print_queue(node_t ** head);