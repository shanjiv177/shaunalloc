# shaunalloc

Trying to write a memory allocator that when finished would hopefully do something similar to malloc.

## What have I done?

The program gets a 64KB chunk of memory through mmap.

Each block which would be assigned is padded to nearest 8 bytes.

The program is all about how the heap/memory from mmap is being assigned and managed.

There are four main functions
- shaunalloc
- free
- realloc
- memory_inspect

### shaunalloc

shaunalloc takes in size as a parameter, checks if there is any already freed memory that could be repurposed first and then if there is no freed memory that could be repurposed, it assigns a block of memory from the heap that we got from mmap.

### free

It sets the block passed into it as "freed". It could be repurposedd for further allocations by shaunalloc.

### realloc

realloc can be used to sort of expand the size of previously alloced blocks. It gives out a new pointer to the expanded location and copies old data to the new address.

### memory_inspect

memory_inspect can be used to show the end user details like how much meory has been freed, alloced or assigned.

I have used two lists to keep track of alloced and freed blocks. One more variable alloced_size is used to keep track of how many bytes have "actually" been assigned from the heap that was mmaped.


