#include "linkedlist.h"
#include "math.h"
#include <stdio.h>
#include <stdbool.h>
/*
we have 1024 MB memory then it will divide into powers of two
1,2,4,8,16,32,64,128,256,512,1024 so we need to allocate array of (linkedlists of pairs) of size 11 such that
every linked list refers to a certain size depends on its index in the array
*/

struct Node** AllSizes = NULL;
int NumberOfLinkedList;

void initialize(int size)
{
	//allocate needed linked lists 
	NumberOfLinkedList = ceil(log(size) / log(2)) + 1;
	AllSizes = malloc(sizeof(struct Node*) * NumberOfLinkedList);
	for (int i = 0; i < NumberOfLinkedList; i++)
	{
		AllSizes[i] = NULL;
	}
	//initialize the whole size to the last linked list
	pair temp = { .StartIndex = 0,.EndIndex = size - 1 };
	insertAtBeginning(&AllSizes[NumberOfLinkedList - 1], temp);
}

void finalize()
{
	free(AllSizes);
}

bool allocate(int size, pair* Indicies)
{
	int index = ceil(log(size) / log(2));
	if (!isEmpty(&AllSizes[index]))
	{
		Indicies->StartIndex = AllSizes[index]->data.StartIndex;
		Indicies->EndIndex = AllSizes[index]->data.EndIndex;

		// Remove block from free list
		deleteFirstNode(&AllSizes[index]);
		printf("Memory allocated from %d to %d \n", Indicies->StartIndex, Indicies->EndIndex);
		return true;
	}
	else
	{
		int i;
		for (i = index + 1; i < NumberOfLinkedList; i++)
		{

			// Find block size greater than request
			if (!isEmpty(&AllSizes[i]))
				break;
		}

		// If no such block is found
		// i.e., no memory block available
		if (i == NumberOfLinkedList)
		{
			printf("Sorry, failed to allocate memory \n");
			return false;
		}

		// If found
		else
		{
			Indicies->StartIndex = AllSizes[i]->data.StartIndex;
			Indicies->EndIndex = AllSizes[i]->data.EndIndex;

			// Remove first block to split it into halves
			deleteFirstNode(&AllSizes[i]);
			i--;

			for (; i >= index; i--)
			{

				// Divide block into two halves
				pair pair1 = { .StartIndex = Indicies->StartIndex,.EndIndex = Indicies->StartIndex + (Indicies->EndIndex - Indicies->StartIndex) / 2 };
				pair pair2 = { .StartIndex = Indicies->StartIndex + (Indicies->EndIndex - Indicies->StartIndex + 1) / 2,.EndIndex = Indicies->EndIndex };
				insertAtBeginning(&AllSizes[i], pair2);
				insertAtBeginning(&AllSizes[i], pair1);

				Indicies->StartIndex = AllSizes[i]->data.StartIndex;
				Indicies->EndIndex = AllSizes[i]->data.EndIndex;

				// Remove first free block to
				// further split
				deleteFirstNode(&AllSizes[i]);
			}
			printf("Memory allocated from %d to %d \n", Indicies->StartIndex, Indicies->EndIndex);
			//ins
			return true;
		}
	}
}

bool deallocate(int start_index, int end_index)
{
	//size of the block to be searched
	int size = end_index - start_index + 1;
	int Index = ceil(log(size) / log(2));
	//free the block
	pair Deallocation_block = { .StartIndex = start_index ,.EndIndex = end_index };
	insertAtBeginning(&AllSizes[Index], Deallocation_block);
	printf("Memory from %d to %d freed\n", start_index, end_index);
	int i, buddyNumber = start_index / size, buddyAddress;
	if (buddyNumber % 2 != 0)
		buddyAddress = start_index - pow(2, Index);
	else
		buddyAddress = start_index + pow(2, Index);

	for (i = 0; i < length(AllSizes[Index]); i++)
	{

		// If buddy found and is also free
		if (AllSizes[Index]->data.StartIndex == buddyAddress)
		{

			// Now merge the buddies to make
			// them one large free memory block
			if (buddyNumber % 2 == 0)
			{
				pair temp = { .StartIndex = start_index ,.EndIndex = start_index + 2 * (pow(2, Index) - 1) };
				insertAtBeginning(&AllSizes[Index + 1], temp);
				printf("Coalescing of blocks starting at %d and %d was done\n", start_index, buddyAddress);
			}
			else
			{
				pair temp = { .StartIndex = buddyAddress ,.EndIndex = buddyAddress + 2 * pow(2, Index) };
				insertAtBeginning(&AllSizes[Index + 1], temp);
				printf("Coalescing of blocks starting at %d and %d was done\n", buddyAddress, start_index);

			}
			deleteFirstNode(&AllSizes[Index]);
			deleteLastNode(&AllSizes[Index]);
			break;
		}
	}
	return true;
}

int main()
{
	pair temp1 = { .StartIndex = 0,.EndIndex = 0 };
	pair temp2 = { .StartIndex = 0,.EndIndex = 0 };
	pair temp3 = { .StartIndex = 0,.EndIndex = 0 };
	pair temp4 = { .StartIndex = 125,.EndIndex = 200 };
	initialize(128);
	allocate(16, &temp1);
	allocate(16, &temp2);
	allocate(16, &temp3);
	//allocate(16, & temp4);
	deallocate(temp1.StartIndex,temp1.EndIndex);
	deallocate(temp2.StartIndex,temp2.EndIndex);
	deallocate(temp3.StartIndex,temp3.EndIndex);
	deallocate(temp4.StartIndex,temp4.EndIndex);

	return 0;

}
