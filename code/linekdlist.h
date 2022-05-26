#include <stdio.h>
#include <stdlib.h>
#include "stdbool.h"

//define a pair which contain start and end indicies of reserverd memory
typedef struct pair
{
	int StartIndex;
	int EndIndex;
} pair;

//define a linked list node ure
typedef struct Node
{
	pair data;
	struct Node* next;
} Node;

// Insert at the beginning
void insertAtBeginning(Node** head_ref, pair new_data) {
	// Allocate memory to a node
	Node* new_node = (Node*)malloc(sizeof(Node));

	// insert the data
	new_node->data = new_data;

	new_node->next = (*head_ref);

	// Move head to new node
	(*head_ref) = new_node;
}

// Insert a node after a node
void insertAfter(Node* prev_node, pair new_data) {
	if (prev_node == NULL) {
		printf("the given previous node cannot be NULL");
		return;
	}

	Node* new_node = (Node*)malloc(sizeof(Node));
	new_node->data = new_data;
	new_node->next = prev_node->next;
	prev_node->next = new_node;
}

// Insert the the end
void insertAtEnd(Node** head_ref, pair new_data) {
	Node* new_node = (Node*)malloc(sizeof(Node));
	Node* last = *head_ref; /* used in step 5*/

	new_node->data = new_data;
	new_node->next = NULL;

	if (*head_ref == NULL) {
		*head_ref = new_node;
		return;
	}

	while (last->next != NULL) last = last->next;

	last->next = new_node;
	return;
}

//is linked list is empty
bool isEmpty(Node** head_ref)
{
	return *head_ref == NULL;
}

int length(Node* node)
{
	int length = 0;
	while (node != NULL) {
		length++;
		node = node->next;
	}
	return length;
}

//Delete from start
bool deleteFirstNode(Node** head_ref) {
	if (head_ref == NULL)
		return false;

	Node* temp = *head_ref;
	*head_ref = (*head_ref)->next;
	free(temp);
	return true;
}

//Delete last node in the linked list
bool deleteLastNode(Node** head_ref)
{
	if (head_ref == NULL)
		return false;
	Node* temp = *head_ref;
	while (temp->next != NULL)
	{
		continue;
	}
	free(temp);
	return true;
}

// Delete a node using its start index as a key
bool deleteNode(struct Node** head_ref, int StartIndex) {
	struct Node* temp = *head_ref, * prev = NULL;

	if (temp != NULL && temp->data.StartIndex == StartIndex) {
		*head_ref = temp->next;
		free(temp);
		return true;
	}
	// Find the key to be deleted
	while (temp != NULL && temp->data.StartIndex != StartIndex) {
		prev = temp;
		temp = temp->next;
	}

	// If the key is not present
	if (temp == NULL) return false;

	// Remove the node
	prev->next = temp->next;

	free(temp);
	return true;
}

// Print the linked list
void printList(Node* node) {
	while (node != NULL) {
		printf("[%d,%d] --> ", node->data.StartIndex, node->data.EndIndex);
		node = node->next;
	}
	printf("NULL\n");
}
