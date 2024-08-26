#include <stdio.h>
#include "shaunalloc.h"

void main() {
    init_heap();
    
    int *x = (int *) shaunalloc(5*sizeof(int));

    for (int i = 0; i < 5; i++) {
        x[i] = i;
    }

    for (int i = 0; i < 5; i++) {
        printf("%d ", x[i]);
    }

    printf("\n");

    memory_inspect();
    int *y = (int *) realloc(x, 6*sizeof(int));

    y[5] = 10;

    for (int i = 0; i < 6; i++) {
        printf("%d ", y[i]);
    }
    printf("\n");

    memory_inspect();
}