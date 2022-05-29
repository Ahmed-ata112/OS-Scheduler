#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define node_s struct lkl_node

typedef struct pair {
    int start_ind;
    int end_ind;

} pair_t;


struct lkl_node {
    struct pair data;
    struct lkl_node *next;
};


//display the list
void lkl_printList(node_s **head) {
    struct lkl_node *ptr = *head;
    printf("\n[ ");

    //start from the beginning
    while (ptr != NULL) {
        printf("\n ptr:(%d,%d) \n", ptr->data.start_ind, ptr->data.end_ind);
        ptr = ptr->next;
    }

    printf(" ]");
}

//insert link at the first location
void lkl_insertFirst(node_s **head, struct pair data) {
    //create a link
    struct lkl_node *link = (struct lkl_node *) malloc(sizeof(struct lkl_node));

    link->data = data;

    //point it to old first lkl_node
    link->next = *head;

    //point first to new first lkl_node
    *head = link;
}

//delete_by_key first item
struct lkl_node *lkl_deleteFirst(struct lkl_node **head) {

    //save reference to first link
    struct lkl_node *tempLink = *head;
    //mark next to first link as first
    *head = (*head)->next;
    //return the deleted link

    return tempLink;
}

//is list empty
bool lkl_isEmpty(struct lkl_node **head) {
    return *head == NULL;
}

int lkl_length(struct lkl_node **head) {
    int length = 0;
    struct lkl_node *current;

    for (current = *head; current != NULL; current = current->next) {
        length++;
    }

    return length;
}


bool free_linked_list(struct lkl_node **head) {
    //if empty
    if (!(*head))return true;
    while (*head) {
        struct lkl_node *temp = (*head)->next;
        free(*head);
        *head = temp;
    }
    return true;
}


//delete_by_key a link with given key
struct lkl_node *delete_by_start_id(struct lkl_node **head, int start_ind) {

    //start from the first link
    struct lkl_node *current = *head;
    struct lkl_node *previous = NULL;

    //if list is empty
    if (*head == NULL) {
        return NULL;
    }

    //navigate through list
    while (current->data.start_ind != start_ind) {

        //if it is last lkl_node
        if (current->next == NULL) {
            return NULL; // the last Node // NOT FOUND
        } else {
            //store reference to current link
            previous = current;
            //move to next link
            current = current->next;
        }
    }

    //found a match, update the link
    if (current == *head) {
        //change first to point to next link
        *head = (*head)->next;
    } else {
        //bypass the current link
        previous->next = current->next;
    }

    return current;
}
