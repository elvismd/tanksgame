#pragma once 

#include "agnostic.h"
#include "entities.h"
#include "memory_arena.h"

void init_tilemap(Tile_Map* tile_map, Memory_Arena* arena);
void add_tile(u32 flags, vec2 position, vec4 source_rect, Tile_Map* tile_map);
void add_tile(u32 flags, vec2 position, u32 tile_index, Tile_Map* tile_map);
void remove_tile(vec2 position, Tile_Map* tile_map);