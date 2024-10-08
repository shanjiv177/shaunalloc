#include <stdio.h>
// #include <string.h>
#include <sys/mman.h>

# define HEAP_SIZE 65536
# define BLOCK_SIZE 8
# define MAX_BLOCKS 8196

typedef struct {
    size_t size; // size of user data
    int free;
    void *start; // start of user data
} Block;

typedef struct {
    size_t size;
    void* list[MAX_BLOCKS];
} BlockList;

BlockList alloced = {0};
BlockList freed = {0};
void *heap = NULL;

void init_heap() {
    void *mem = mmap(NULL, HEAP_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    heap = mem;
}

size_t alloced_size  = 0;

void* shaunalloc(size_t size) {

    if (size == 0) {
        printf("Size must be positive\n");
        return NULL;
    }

    size_t padded_size = BLOCK_SIZE - (size % BLOCK_SIZE) + size; // aligning for chunks of size 8 bytes

    if (alloced_size + padded_size + sizeof(Block) > HEAP_SIZE) {
        printf("Insufficient storage on heap\n");
        return NULL;
    }

    for (int i = 0; i < freed.size; i++) {
        Block *block = (Block *) freed.list[i];
        // Getting header of the block
        block--;

        if (((int) (block->size - padded_size)) >= 24) {
            size_t curr_block_size = block->size;
            block->size = padded_size;
            block->free = 0;

            alloced.list[alloced.size++] = block->start;

            // creating a new block for the remaining space
            Block *newBlock = (Block *) (block->start + padded_size);
            newBlock->start = block->start + padded_size + sizeof(Block);
            newBlock->free = 1;
            newBlock->size = curr_block_size - padded_size - sizeof(Block);

            freed.list[i] = newBlock->start;
            
            return block->start;
        }

        if (block->size >= padded_size) {
            block->size = padded_size;
            block->free = 0;

            alloced.list[alloced.size++] = block->start;

            for (int j = i+1; i < freed.size-1; i++) {
                freed.list[j] = freed.list[j+1];
            }

            freed.size--;
            
            return block->start;
        }
    }
    void *alloced_mem = heap + alloced_size; // Pointer to the start of the block + header

    Block *newBlock = (Block*) alloced_mem;

    newBlock->size = padded_size; // size of the payload
    newBlock->free = 0;
    newBlock->start = (void *) alloced_mem + sizeof(Block);

    alloced_size += padded_size + sizeof(Block);

    alloced.list[alloced.size++] = newBlock->start;

    return newBlock->start; // returning the ptr to the start payload
}

void free(void* ptr) {

    int flag = 0;

    for (int i = 0; i < alloced.size; i++) {
        if (alloced.list[i] == ptr) flag = 1;
    }

    if (flag == 0) {
        printf("You are trying to free already freed memory\n");
        return;
    }

    // user will have access to only the pointer to the payload, so we point the header pointer to the header of the block
    Block* header = (Block *) ptr - 1;

    // Marking the block as freed
    header->free = 1;

    for (int i = 0; i < alloced.size; i++) {
        if (alloced.list[i] == ptr) {
            for (int j = i; j < alloced_size - 1; j++) {
                alloced.list[j] = alloced.list[j+1];
            }
            alloced.size--;
            break;
        }
    }

    freed.list[freed.size++] = ptr;

    for (int i = freed.size -1 ; i > 0; i--) {
        if (freed.list[i] - freed.list[i-1] < 0)  {
            void* ptr = freed.list[i];
            freed.list[i] = freed.list[i-1];
            freed.list[i-1] = ptr;
        }
    }

    if (freed.size < 1) return;

    for (int i = 0; i < freed.size - 1; i++) {
        // Go to the header of the current block
        Block *curr = ((Block *) freed.list[i]) - 1;

        // Go to header fo the next block
        Block *next = ((Block *) freed.list[i+1]) - 1;

        // if the difference between them is equal to size of curr block + sizeof(block)
        // then they are next to each other, and we can combine them

        if (((void *) next - (void *) curr) == (sizeof(Block) + curr->size)) {
            // Increase size of curr block
            curr->size += next->size + sizeof(Block);

            // remove "next" block's adress from freed.list
            for (int j = i+1; i < freed.size-1; i++) {
                freed.list[j] = freed.list[j+1];
            }

            freed.size--;
        }
    }
}

void* realloc(void* ptr, size_t size) {


    if (size == 0) {
        free(ptr);
        return;
    }

    int flag = 0;

    for (int i = 0; i < alloced.size; i++) {
        if (alloced.list[i] == ptr) flag = 1;
    }

    if (flag == 0) {
        printf("You are trying to realloc freed memory\n");
        return;
    }

    if (ptr == NULL) {
        printf("Invalid address\n");
        return;
    }

    void *mem = shaunalloc(size);
    char *cmem = (char *) mem;
    const char *cptr = (const char *) ptr;
    size_t padded_size = BLOCK_SIZE - (size % BLOCK_SIZE) + size;

    Block *block = (Block *) ptr;
    block--;
    size_t current_size = block->size;

    for (int i =0; i < current_size; i++) {
        cmem[i] = cptr[i];
    }

    // memcpy(mem, ptr, size);

    free(ptr);

    return mem;
}

size_t get_size(void *ptr) {
    Block *block = (Block *) ptr;
    block--;

    return block->size;
}

void memory_inspect() {

    // printf("Memory Allocated (in use) and their sizes: \n");
    // for (int i = 0; i < alloced.size; i++) {
    //     printf("\t%p\n", alloced.list[i]);
    // }

    // printf("Memory Freed (not in use) and their sizes in bytes: \n");
    // for (int i = 0; i < freed.size; i++) {
    //     printf("\t%p\n", freed.list[i]);
    // }

    // printf("\n");

    printf("Memory Allocated (in use) and their sizes in bytes: \n");
    for (int i = 0; i < alloced.size; i++) {
        printf("\t%p %ld\n", alloced.list[i], get_size(alloced.list[i]));
    }

    printf("Memory Freed (not in use) and their sizes in bytes: \n");
    for (int i = 0; i < freed.size; i++) {
        printf("\t%p %ld\n", freed.list[i], get_size(freed.list[i]));
    }

    printf("Size assigned till now: %ld\n\n", alloced_size);
}

// void main() {
//     init_heap();
//     void *ptr1 = shaunalloc(40);
//     memory_inspect();

//     void *ptr2 = shaunalloc(20);
//     memory_inspect(); 

//     int* x = ptr2;

//     *x = 10;
//     printf("\n%d\n", *x);

//     free(ptr1);
//     memory_inspect();

//     void *ptr3 = realloc(ptr2,40);
//     memory_inspect();

//     int *y = ptr3;
//     printf("\n%d\n", *y);
// }