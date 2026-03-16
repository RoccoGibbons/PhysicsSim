#include <stdio.h>
#include <stdlib.h>

typedef struct Node {
    Object obj;
    struct Node *next;
} Node;


Node* initialiseLinkedList(Object obj) {
    struct Node *head = NULL;

    head = (struct Node*)malloc(sizeof(struct Node));

    if (!head) {
        printf("ERROR::LINKED_LIST::INITIALISATION::HEAD_MEMORY_ALLOCATION\n");
    }

    head -> obj = obj;
    head -> next = NULL;

    return head;
}

void appendLinkedList(Node* head, Object data) {
    struct Node *search = head;

    while (search->next != NULL) {
        search = search->next;
    }

    struct Node *newNode = NULL;
    newNode = (struct Node*)malloc(sizeof(struct Node));
    newNode->obj = data;
    newNode->next = NULL;

    search->next = newNode;
}

void printLinkedList(Node* head) {
    struct Node *search = head;

    while (search != NULL) {
        printf("%f, %f, %f\n", search->obj.position[0], search->obj.position[1], search->obj.position[2]);
        search = search->next;
    }
}


void freeLinkedList(Node* head) {
    if (head->next == NULL) {
        free(head);
        return;
    }
    freeLinkedList(head->next);
}