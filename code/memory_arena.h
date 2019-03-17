#pragma once 

#include "agnostic.h"

struct Memory_Arena 
{
	memory_index size;
	u8* base;
	memory_index used;
};

void initialize_arena(Memory_Arena* arena, memory_index size, u8* base);
void log_arena(Memory_Arena* arena);

void* push_memory(Memory_Arena* arena, memory_index size);
#define push_struct(arena, type) (type*)push_memory(arena, sizeof(type))