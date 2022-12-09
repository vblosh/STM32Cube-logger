#include <malloc.h>
#include "Logger/inc/chunk_allocator.h"

typedef struct
{
	size_t last;
	size_t chunk_size;
	char* stack[1];
} sAllocator ;

alloc_handle alloc_init(void* buffer, size_t buffer_size, size_t chunk_size, size_t alignment)
{
	// check alignment
	if(chunk_size % alignment) {
		chunk_size = (chunk_size / alignment + 1) * alignment;
	}

	size_t num_chunks = buffer_size / chunk_size;
	sAllocator* palloc = (sAllocator*)malloc(sizeof(sAllocator) + sizeof(void*) * (num_chunks - 1));
	if(palloc) {
		palloc->last = num_chunks;
		palloc->chunk_size = chunk_size;
		for (size_t i = 0; i < num_chunks; ++i) {
			palloc->stack[i] = buffer + i * chunk_size;
		}
	}
	return palloc;
}

inline size_t chunk_size(alloc_handle alloc)
{
	return ((sAllocator*)alloc)->chunk_size;
}

void alloc_deinit(alloc_handle alloc)
{
	free(alloc);
}

void* allocate(alloc_handle alloc)
{
	sAllocator* palloc = (sAllocator*)alloc;
	if(palloc->last == 0) return 0;

	return palloc->stack[--palloc->last];
}

void deallocate(alloc_handle alloc, void* ptr)
{
	sAllocator* palloc = (sAllocator*)alloc;

	palloc->stack[palloc->last++] = ptr;
}
