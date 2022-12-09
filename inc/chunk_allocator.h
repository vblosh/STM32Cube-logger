#ifndef INC_LOGGER_ALLOCATOR_H_
#define INC_LOGGER_ALLOCATOR_H_

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void* alloc_handle;

alloc_handle alloc_init(void* buffer, size_t buffer_size, size_t chunk_size, size_t alignment);
void alloc_deinit(alloc_handle alloc);
size_t chunk_size(alloc_handle alloc);

void* allocate(alloc_handle alloc);
void deallocate(alloc_handle alloc, void* ptr);

#ifdef __cplusplus
}
#endif

#endif /* INC_LOGGER_ALLOCATOR_H_ */
