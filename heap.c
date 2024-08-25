#include <stdio.h>
#include <sys/mman.h>
#include <stdlib.h>

# define HEAP_SIZE 65536
# define CHUNK_SIZE 8

typedef struct {
    size_t size;
    int free;
    struct block *next;
} block;

size_t alloced_size = 0;
void *heap = NULL;

void init_heap() {
    void *mem = mmap(NULL, HEAP_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    heap = mem;
}

void* salloc(size_t size) {
    size_t padded_size = 8 - (size % 8) + size;
    void *alloced_mem = heap + alloced_size;

    block *newBlock = (block*) alloced_mem;

    newBlock->size = padded_size;
    newBlock->free = 0;
    newBlock->next = NULL;

    void* mem = (void *) alloced_mem;

    alloced_size += padded_size + sizeof(block);

    return alloced_mem + 24;
}

void main() {
    init_heap();

    void *ptr = salloc(20);

    
}