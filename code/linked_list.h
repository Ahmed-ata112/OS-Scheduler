#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

struct lkl_node {
    int data;
    int key;
    struct lkl_node *next;
};


//display the list
void lkl_printList(struct lkl_node *head) {
    struct lkl_node *ptr = head;
    printf("\n[ ");

    //start from the beginning
    while (ptr != NULL) {
        printf("(%d,%d) ", ptr->key, ptr->data);
        ptr = ptr->next;
    }

    printf(" ]");
}

//insert link at the first location
void lkl_insertFirst(struct lkl_node *head, int key, int data) {
    //create a link
    struct lkl_node *link = (struct lkl_node *) malloc(sizeof(struct lkl_node));

    link->key = key;
    link->data = data;

    //point it to old first lkl_node
    link->next = head;

    //point first to new first lkl_node
    head = link;
}

//delete_by_key first item
struct lkl_node *lkl_deleteFirst(struct lkl_node *head) {

    //save reference to first link
    struct lkl_node *tempLink = head;

    //mark next to first link as first
    head = head->next;

    //return the deleted link
    return tempLink;
}

//is list empty
bool lkl_isEmpty(struct lkl_node *head) {
    return head == NULL;
}

int lkl_length(struct lkl_node *head) {
    int length = 0;
    struct lkl_node *current;

    for (current = head; current != NULL; current = current->next) {
        length++;
    }

    return length;
}

//find_by_key a link with given key
struct lkl_node *find_by_key(struct lkl_node *head, int key) {

    //start from the first link
    struct lkl_node *current = head;

    //if list is empty
    if (head == NULL) {
        return NULL;
    }

    //navigate through list
    while (current->key != key) {

        //if it is last lkl_node
        if (current->next == NULL) {
            return NULL;
        } else {
            //go to next link
            current = current->next;
        }
    }

    //if data found, return the current Link
    return current;
}

//delete_by_key a link with given key
struct lkl_node *delete_by_key(struct lkl_node *head, int key) {

    //start from the first link
    struct lkl_node *current = head;
    struct lkl_node *previous = NULL;

    //if list is empty
    if (head == NULL) {
        return NULL;
    }

    //navigate through list
    while (current->key != key) {

        //if it is last lkl_node
        if (current->next == NULL) {
            return NULL;
        } else {
            //store reference to current link
            previous = current;
            //move to next link
            current = current->next;
        }
    }

    //found a match, update the link
    if (current == head) {
        //change first to point to next link
        head = head->next;
    } else {
        //bypass the current link
        previous->next = current->next;
    }

    return current;
}


bool free_linked_list(struct lkl_node *head) {
    //if empty
    if (!head)return true;
    while (head) {
        struct lkl_node *temp = head->next;
        free(head);
        head = temp;
    }


}
