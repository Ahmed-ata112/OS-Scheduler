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
 * deallocate(start_id ,size) -> and checks merging inside
 * You can identify each block by its starting Point
 * 0 1 2 3 4 5  6 7    if id is odd multiple of size we don't check merge
 * if even then check if you can merge
 * @tip: we Can map each allocated mem id to the size so we only send the id to the De-allocation function
 * add to Arr
 *
 *
 * each element in the arr is a pair of start_ind -> finish_ind
 *
*/

#define MAX_SIZE 1024


void buddy_init() {

}




























