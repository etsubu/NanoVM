#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif


struct NanoHeap {
    unsigned char *heap_start;
    unsigned char *next_free;
    uint64_t heap_size;
};

typedef struct NanoHeap NanoHeap;

int NanoHeapAllocate(NanoHeap *heap, uint64_t size, uint64_t *outOffset);
int NanoHeapFree(NanoHeap *heap, uint64_t address);

void NanoHeapInit(NanoHeap *heap, unsigned char* heap_start, uint64_t heap_size);


#ifdef __cplusplus
}
#endif
