#include "NanoHeap.h"

int NanoHeapAllocate(NanoHeap *heap, uint64_t size, uint64_t *outOffset) {
    if (size == 0) {
        return 1;
    }
    uint64_t available_space = heap->heap_size - (heap->next_free - heap->heap_start);
    if (available_space >= size) {
        uint64_t ptr = (uint64_t) (heap->next_free - heap->heap_start);
        heap->next_free = heap->next_free + size;
        *outOffset = ptr;
        return 0;
    }
    return 1;
}

int NanoHeapFree(NanoHeap *heap, uint64_t address) {
    // No-op with bump allocator
    (void)heap;
    (void)address;
    return 0;
}

void NanoHeapInit(NanoHeap *heap, unsigned char* heap_start, uint64_t heap_size) {
    heap->heap_start = heap_start;
    heap->heap_size = heap_size;
    heap->next_free = heap_start;
}