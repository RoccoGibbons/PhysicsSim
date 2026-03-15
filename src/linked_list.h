#include <stdio.h>
#include <stdlib.h>
#include "simulation_calculations.h"


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

    head->obj = obj;
    head->next = NULL;

    return head;
}

void appendLinkedList(Node* head, Object obj) {
    struct Node *newNode = NULL;
    newNode = (struct Node*)malloc(sizeof(struct Node));
    newNode->obj = obj;
    newNode->next = NULL;

    struct Node *search = head;

    while (search->next != NULL) {
        search = search->next;
    }

    search->next = newNode;
}

