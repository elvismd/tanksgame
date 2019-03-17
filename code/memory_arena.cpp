#include "memory_arena.h"
#include "logger.h"

void initialize_arena(Memory_Arena* arena, memory_index size, u8* base)
{
    arena->size = size;
    arena->base = base;
    arena->used = 0;
} 

void* push_memory(Memory_Arena* arena, memory_index size)
{
    Assert((arena->used + size) <= arena->size);

    void* result = arena->base + arena->used;
    arena->used += size;
	
    return result;
}

void log_arena(Memory_Arena* arena)
{
    r64 used_mb = (r64)arena->used / 1000000.0;
    log_info("Memory used on arena: %f megabytes", used_mb);
}