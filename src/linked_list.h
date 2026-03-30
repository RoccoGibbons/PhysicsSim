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

void deleteNode(Node* head, int deleteID) {
    Node *search = head;
    Node *previous = head;

    while (search->obj.id != deleteID && search != NULL) {
        previous = search;
        search = search->next;
    }
    if (search == NULL) {
        return;
    } else {
        previous->next = search->next;
        free(search);
    }
}

void printLinkedList(Node* head) {
    struct Node *search = head;

    while (search != NULL) {
        // printf("%f, %f, %f\n", search->obj.position[0], search->obj.position[1], search->obj.position[2]);
        printf("%i\n", search->obj.id);
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