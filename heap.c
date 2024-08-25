#include <stdio.h>
#include <sys/mman.h>

# define HEAP_SIZE 65536
# define BLOCK_SIZE 8
# define MAX_BLOCKS 8196

typedef struct {
    size_t size;
    int free;
    void *start;
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

    freed.list[freed.size++] = ptr;
}

void main() {
    init_heap();

    void *ptr = shaunalloc(20);

    void *ptr1 =  shaunalloc(30);

    void *ptr2 = shaunalloc(45);

    void *ptr3 = shaunalloc(60);

    for (int i = 0; i < alloced.size; i++) {
        printf("%p\n", alloced.list[i]);
    }

    free(ptr1);
    free(ptr2);

    printf("\n");
    for (int i = 0; i < freed.size; i++) {
        printf("%p\n", freed.list[i]);
    }
}