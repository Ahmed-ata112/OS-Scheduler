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
 * each element in the sizes_arr is a pair of start_ind -> finish_ind
 *
 */
// because 2 ^ 10 = 1024
// and one for the 2 ^ 0
// 1 2  4 8  16 32 64 128 256 512 1024

#define MAX_SIZE 1024
#define MAX_PLACES 11
node_s *sizes_arr[MAX_PLACES];


/*!
 *
 * initialize the array
 * should be called before the array is initialized
 *
 */
void buddy_init() {

    // init array and put the memory into it
    for (int i = 0; i < MAX_PLACES; ++i) {
        sizes_arr[i] = NULL; // init all array by 0
    }
    // a place in the 1024 sector that takes all the memory
    pair_t p = {.start_ind = 0, .end_ind = MAX_SIZE - 1};
    lkl_insertFirst(&sizes_arr[MAX_PLACES - 1], p);
}

/*!
 *
 * @param size : the size You want to allocate
 * @param returned : passed by ref to get the start and end of the allocated memory (if allocated successfully) or NULL if failed
 * @return bool : true if allocated successfully and false if failed
 */
bool buddy_allocate(int size, pair_t *returned) {
    if (size > MAX_SIZE) {
        printf("couldn't allocate e1");
        return false;
    }

    // the actual size to alloc is ceil(log2(size))
    int up_size = ceil(log2(size));
    if (!lkl_isEmpty(&sizes_arr[up_size])) {
        // there is a perfect place
        //  TODO see what would u do with the returned
        node_s *del_node = lkl_deleteFirst(&sizes_arr[up_size]);
        //        printf("\nallocated from %d to %d\n", del_node->data.start_ind, del_node->data.end_ind);
        *returned = (pair_t) {del_node->data.start_ind, del_node->data.end_ind};
        printf("allocated from %d to %d\n", del_node->data.start_ind, del_node->data.end_ind);

        return true;
    }

    // split nearest larger place
    //  search for a one with larger to split
    int higher_size = up_size + 1;
    for (; higher_size < MAX_PLACES; ++higher_size) {

        if (!lkl_isEmpty(&sizes_arr[higher_size])) {
            printf("%d\n", higher_size);
            break;
        }
    }
    if (higher_size == MAX_PLACES) {
        // Sorry : no upper sizes to split
        printf("\ncouldn't allocate: no space\n'");
        return false;
    }

    // split higher_size till u reach up_size

    while (higher_size != up_size) {
        node_s *extracted = lkl_deleteFirst(&sizes_arr[higher_size]);
        higher_size--;
        pair_t ss = extracted->data;
        // 0 1 2 3  4 5 6 7
        int section_size = ss.end_ind - ss.start_ind;
        pair_t p1 = {.start_ind = ss.start_ind, ss.start_ind + section_size / 2};
        pair_t p2 = {.start_ind = ss.start_ind + section_size / 2 + 1, .end_ind = ss.end_ind};

        // printf("h: %d s : %d %d \n", higher_size, ss.start_ind, ss.end_ind);
        // printf("p1 : %d  \n", p1.start_ind - p1.end_ind - 1);
        // printf("p2 : %d \n", p2.start_ind - p2.end_ind - 1);
        lkl_insertFirst(&sizes_arr[higher_size], p2);

        lkl_insertFirst(&sizes_arr[higher_size], p1);
    }

    node_s *del_node = lkl_deleteFirst(&sizes_arr[up_size]);
    // printf("\nallocated from %d to %d\n", del_node->data.start_ind, del_node->data.end_ind);
    *returned = (pair_t) {del_node->data.start_ind, del_node->data.end_ind};
    printf("allocated from %d to %d\n", del_node->data.start_ind, del_node->data.end_ind);
    return true;
}


/*!
 *
 * @param start_ind start of memory to delete
 * @param end_ind end of the memory to delete
 * @note those 2 params should be given from the process table and the diff would be the size of the section (a power of 2)
 */
void buddy_deallocate(int start_ind, int end_ind) {
    int size = end_ind - start_ind + 1; // deallocated sector size
    pair_t p = {start_ind, end_ind};
    int up_size = ceil(log2(size));

    lkl_insertFirst(&sizes_arr[up_size], p); // inserts the fragment at the beginning of its size fragment
    printf("\ndeleted from %d to %d \n", start_ind, end_ind);

    while (1) {
        int buddy_order = start_ind / size;
        int buddy_to_search = -1;
        if (buddy_order % 2 == 1) {
            // the previous could be merged if found
            // 0 1   2 3       4 5 6 7
            buddy_to_search = start_ind - size;
        } else {
            buddy_to_search = start_ind + size;
        }
        struct lkl_node *ret = delete_by_start_id(&sizes_arr[up_size], buddy_to_search);

        if (ret == NULL) {
            // no more to merge
            return;
        }
        // delete it again to merge it
        struct lkl_node *the_other = lkl_deleteFirst(&sizes_arr[up_size]);
        int new_start, new_end;
        if (buddy_order % 2 == 1) {
            // the previous could be merged if found
            // 0 1   2 3       4 5 6 7
            new_start = ret->data.start_ind;
            new_end = the_other->data.end_ind;
        } else {
            new_start = the_other->data.start_ind;
            new_end = ret->data.end_ind;
        }
        printf("merged from %d to %d \n", new_start, new_end);
        pair_t pp;
        pp.start_ind = new_start;
        pp.end_ind = new_end;

        up_size++;
        lkl_insertFirst(&sizes_arr[up_size], pp);
        start_ind = new_start;
        size = new_end - new_start + 1;

        if (size == MAX_SIZE || up_size == MAX_PLACES - 1) {
            return;
        }
    }
}

int main() {
    buddy_init();
    pair_t p1;
    if (buddy_allocate(8, &p1))
        printf("\nin main allocated from %d to %d\n", p1.start_ind, p1.end_ind);
    pair_t p2;
    if (buddy_allocate(1, &p2))
        printf("\nin main allocated from %d to %d\n", p2.start_ind, p2.end_ind);

    // pair_t p3;
    // if (buddy_allocate(256, &p3))
    //     printf("\nin main allocated from %d to %d\n", p3.start_ind, p3.end_ind);
    // pair_t p4;
    // if (buddy_allocate(256, &p4))
    //     printf("\nin main allocated from %d to %d\n", p4.start_ind, p4.end_ind);

    buddy_deallocate(p1.start_ind, p1.end_ind);
    // buddy_deallocate(p2.start_ind, p2.end_ind);
    // buddy_deallocate(p3.start_ind, p3.end_ind);
    pair_t p5;
    if (buddy_allocate(800, &p5))
        printf("\nin main allocated from %d to %d\n", p5.start_ind, p5.end_ind);

    buddy_deallocate(p2.start_ind, p2.end_ind);
    if (buddy_allocate(500, &p5))
        printf("\nin main allocated from %d to %d\n", p5.start_ind, p5.end_ind);

    if (buddy_allocate(8, &p5))
        printf("\nin main allocated from %d to %d\n", p5.start_ind, p5.end_ind);

    buddy_deallocate(p5.start_ind, p5.end_ind);

    if (buddy_allocate(500, &p5))
        printf("\nin main allocated from %d to %d\n", p5.start_ind, p5.end_ind);
}
