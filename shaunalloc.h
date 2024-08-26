#ifndef SHAUNALLOC_H
#define SHAUNALLOC_H

#include <stdio.h>

void init_heap();
void* shaunalloc(size_t size);
void free(void* ptr);
void* realloc(void* ptr, size_t size);
void memory_inspect();

#endif // SHAUNALLOC_H