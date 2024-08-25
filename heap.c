#include <stdio.h>
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

    if (size == 0) return NULL;

    size_t padded_size = BLOCK_SIZE - (size % BLOCK_SIZE) + size; // aligning for chunks of size 8 bytes

    if (alloced_size + padded_size + sizeof(Block) > HEAP_SIZE) return NULL;

    for (int i = 0; i < freed.size; i++) {
        Block *block = (Block *) freed.list[i];

        if (block->size - padded_size >= 24) {
            // getting header of the block
            block--;
            block->size = padded_size;
            block->free = 0;

            alloced.list[alloced.size++] = block->start;

            // creating a new block for the remaining space
            Block *newBlock = (Block *) ((void* ) block + sizeof(block) + padded_size);
            newBlock->start = (void *) newBlock + sizeof(block);
            newBlock->free = 1;
            newBlock->size = block->size - padded_size - sizeof(Block);

            freed.list[i] = (void *) newBlock;
            
            return block->start;
        }

        if (block->size >= padded_size) {
            block--;
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

void main() {
    init_heap();

    void *ptr1 = shaunalloc(20);

    void *ptr2 = shaunalloc(30);

    void *ptr3 = shaunalloc(10);

    void *ptr4 = shaunalloc(15);

    for (int i = 0; i < alloced.size; i++) {
        printf("%p\n", alloced.list[i]);
    }

    free(ptr2);
    free(ptr3);

    void* ptr5 = shaunalloc(40);

    printf("\n");
    for (int i = 0; i < freed.size; i++) {
        printf("%p\n", freed.list[i]);
    }

    printf("\n");

    for (int i = 0; i < alloced.size; i++) {
        printf("%p\n", alloced.list[i]);
    }
}