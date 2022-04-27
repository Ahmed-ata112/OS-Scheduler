#include "linked_listt.h"
#include <math.h>
#include <stdio.h>

/**
 * Total size is 1024 bytes  10 places:: 2 4 8 16 32 64 128 256 512 1024
 * init
 * allocate(size) -> true | false
 * if  buddy[fit(size)] has a space -> allocate it directly
 * if not -> try searching upper sizes in array
 *              -> found a one with a free space in it :
 *                      You just split it into 2 sizes and take one of them and split it till u create needed size
 *              -> n exceeded size of array -> No free memory Sorry |> return false
 * delete_by_key the part from Arr
 *
 *
 *
 * deallocate(start_ind ,end_ind) -> and checks merging inside
 * You can identify each block by its starting Point
 * 0 1 2 3 4 5  6 7    if start_id is odd multiple of size we don't check merge
 * if even then check if you can merge
 * @tip: we Can map each allocated mem id to the size so we only send the id to the De-allocation function
 * add to Arr
 *
 *
 * each element in the sizes_arr is a pair of start_ind -> finish_ind
 *
*/

#define MAX_SIZE 1024
// because 2 ^ 10 = 1024
// and one for the 2 ^ 0
// 1 2  4 8  16 32 64 128 256 512 1024
#define MAX_PLACES 11
#define pair_s struct pair
node_s *sizes_arr[MAX_PLACES];

void buddy_init() {

    //init array and put the memory into it
    for (int i = 0; i < MAX_PLACES; ++i) {
        sizes_arr[i] = NULL; // init all array by 0
    }
    // a place in the 1024 sector that takes all the memory
    pair_s p = {.start_ind = 0, .end_ind = MAX_SIZE - 1};
    lkl_insertFirst(&sizes_arr[MAX_PLACES - 1], p);

}

bool buddy_allocate(int size, pair_s *returned) {
    if (size > MAX_SIZE) {
        printf("couldn't allocate e1");
        return false;
    }

    // the actual size to alloc is ceil(log2(size))
    int up_size = ceil(log2(size));
    if (!lkl_isEmpty(&sizes_arr[up_size])) {
        //there is a perfect place
        // TODO see what would u do with the returned
        node_s *del_node = lkl_deleteFirst(&sizes_arr[up_size]);
//        printf("\nallocated from %d to %d\n", del_node->data.start_ind, del_node->data.end_ind);
        *returned = (pair_s) {del_node->data.start_ind, del_node->data.end_ind};
        return true;
    }

    //split nearest larger place
    // search for a one with larger to split
    printf("\nup: %d\n", up_size);
    int higher_size = up_size + 1;
    for (; higher_size < MAX_PLACES; ++higher_size) {

        if (!lkl_isEmpty(&sizes_arr[higher_size])) {
            printf("%d\n", higher_size);
            break;

        }
    }
    if (higher_size == MAX_PLACES) {
        //Sorry : no upper sizes to split
        printf("\ncouldn't allocate: no space\n'");
        return false;
    }

    // split higher_size till u reach up_size

    while (higher_size != up_size) {
        node_s *extracted = lkl_deleteFirst(&sizes_arr[higher_size]);
        higher_size--;
        pair_s ss = extracted->data;
        //0 1 2 3  4 5 6 7
        int section_size = ss.end_ind - ss.start_ind;
        pair_s p1 = {.start_ind = ss.start_ind, ss.start_ind + section_size / 2};
        pair_s p2 = {.start_ind = ss.start_ind + section_size / 2 + 1, .end_ind= ss.end_ind};

        //printf("h: %d s : %d %d \n", higher_size, ss.start_ind, ss.end_ind);
        //printf("p1 : %d  \n", p1.start_ind - p1.end_ind - 1);
        //printf("p2 : %d \n", p2.start_ind - p2.end_ind - 1);
        lkl_insertFirst(&sizes_arr[higher_size], p2);

        lkl_insertFirst(&sizes_arr[higher_size], p1);
    }

    node_s *del_node = lkl_deleteFirst(&sizes_arr[up_size]);
    //printf("\nallocated from %d to %d\n", del_node->data.start_ind, del_node->data.end_ind);
    *returned = (pair_s) {del_node->data.start_ind, del_node->data.end_ind};

    return true;
}

void buddy_deallocate(int start_ind, int end_ind) {
    int size = end_ind - start_ind + 1;  //deallocated sector size
    pair_s p = {start_ind, end_ind};
    int up_size = ceil(log2(size));

    int a = start_ind / size;
    if (a % 2 == 1) {
        //don't Merge
        lkl_insertFirst(&sizes_arr[up_size], p);
        printf("\nDeallocate NO MERGE from %d to %d\n", start_ind, end_ind);
        return;
    } else {
        //Try to merge
        int s = up_size;
        while (s < MAX_PLACES) {
            node_s *head = sizes_arr[s];
            if (!head) {
                lkl_insertFirst(&sizes_arr[up_size], p);
                printf("\nDeallocate NO MERGE from %d to %d\n", start_ind, end_ind);
                return;
            }
            while (head) {

            }


        }
    }


}

int main() {
    buddy_init();
    pair_s pp;
    if (buddy_allocate(256, &pp))
        printf("\nin main allocated from %d to %d\n", pp.start_ind, pp.end_ind);
    if (buddy_allocate(256, &pp))
        printf("\nin main allocated from %d to %d\n", pp.start_ind, pp.end_ind);
    if (buddy_allocate(256, &pp))
        printf("\nin main allocated from %d to %d\n", pp.start_ind, pp.end_ind);

    if (buddy_allocate(2, &pp))
        printf("\nin main allocated from %d to %d\n", pp.start_ind, pp.end_ind);
    //deallocate last allocated mem
    //buddy_deallocate(pp.start_ind, pp.end_ind);
    if (buddy_allocate(2, &pp))
        printf("\nin main allocated from %d to %d\n", pp.start_ind, pp.end_ind);

}



















