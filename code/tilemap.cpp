#include "tilemap.h"
#include "draw.h"
#include "asset_storage.h"

void add_tile(u32 flags, vec2 position, u32 tile_index, Tile_Map* tile_map)
{
    add_tile(flags, position, get_texture_rect_from_index(tile_index, 32, 32, tile_map->entity->texture), tile_map);
}

void add_tile(u32 flags, vec2 position, vec4 source_rect, Tile_Map* tile_map)
{
    Mesh* mesh = tile_map->entity->mesh;
    vec3* vertices = mesh->vertices;
    vec2* uvs = mesh->uvs;
    vec4* colors = mesh->colors;

    vec2 tile_map_pos = tile_map->entity->position;

    // for a sprite batch, use just the float position on verts
    s32 index_x = (s32)(position.x);
    s32 index_y = (s32)(position.y);

    if(index_x < 0 || index_y < 0 || index_x >= TILEMAP_SIZE || index_y >= TILEMAP_SIZE) return;

    const r32 w = 1.0f;
    const r32 h = 1.0f;

    // TODO: Fix this, its redundant, the indexes are the positions itself
    tile_map->tile_positions[index_x][index_y] = position;

    uint32 tile_index = tile_map->tile_vertices_indexes[index_x][index_y];

    tile_map_pos = tile_map->entity->position;
    //if(flags == 0)
    {
        // Record the source rect and flags
        tile_map->tile_rects[index_x][index_y] = source_rect;
        tile_map->tile_types[index_x][index_y] = flags;

        r32 offset_x = index_x;
        r32 offset_y = index_y;

        vertices[tile_index]     = vec3(offset_x    , offset_y + h, 0.0f);
        vertices[tile_index + 1] = vec3(offset_x + w, offset_y  , 0.0f);
        vertices[tile_index + 2] = vec3(offset_x    , offset_y  , 0.0f);
        vertices[tile_index + 3] = vec3(offset_x    , offset_y + h, 0.0f);
        vertices[tile_index + 4] = vec3(offset_x + w, offset_y + h, 0.0f);
        vertices[tile_index + 5] = vec3(offset_x + w, offset_y  , 0.0f);

        uvs[tile_index]     = vec2(source_rect.x, source_rect.y + source_rect.w);
        uvs[tile_index + 1] = vec2(source_rect.x + source_rect.w, source_rect.y);
        uvs[tile_index + 2] = vec2(source_rect.x, source_rect.y);
        uvs[tile_index + 3] = vec2(source_rect.x, source_rect.y + source_rect.w);
        uvs[tile_index + 4] = vec2(source_rect.x + source_rect.w, source_rect.y + source_rect.w);
        uvs[tile_index + 5] = vec2(source_rect.x + source_rect.w, source_rect.y);

        colors[tile_index]     = vec4(1, 1, 1, 1);
        colors[tile_index + 1] = vec4(1, 1, 1, 1);
        colors[tile_index + 2] = vec4(1, 1, 1, 1);
        colors[tile_index + 3] = vec4(1, 1, 1, 1);
        colors[tile_index + 4] = vec4(1, 1, 1, 1);
        colors[tile_index + 5] = vec4(1, 1, 1, 1);
    }
}

void remove_tile(vec2 position, Tile_Map* tile_map)
{
    Mesh* mesh = tile_map->entity->mesh;
    vec3* vertices = mesh->vertices;

    // for a sprite batch, use just the float position on verts
    u32 index_x = (u32)(position.x + 0.5f);
    u32 index_y = (u32)(position.y + 0.5f);

    if(index_x < 0 || index_y < 0 || index_x >= TILEMAP_SIZE || index_y >= TILEMAP_SIZE) return;

    if(tile_map->tile_types[index_x][index_y] == -1) return;

    // TODO: Fix this, its redundant, the indexes are the positions itself
    tile_map->tile_positions[index_x][index_y] = position;

    uint32 tile_index = tile_map->tile_vertices_indexes[index_x][index_y];

    tile_map->tile_rects[index_x][index_y] = vec4(0.0f);
    tile_map->tile_types[index_x][index_y] = -1;

    vertices[tile_index]     = vec3(0.0f);
    vertices[tile_index + 1] = vec3(0.0f);
    vertices[tile_index + 2] = vec3(0.0f);
    vertices[tile_index + 3] = vec3(0.0f);
    vertices[tile_index + 4] = vec3(0.0f);
    vertices[tile_index + 5] = vec3(0.0f);
}

void init_tilemap(Tile_Map* tile_map, Memory_Arena* arena)
{
    u32 buffer_size = TILEMAP_SIZE * TILEMAP_SIZE * 6;

    vec3* vertices = (vec3*)push_memory(arena, sizeof(vec3) * buffer_size);
    vec2* uvs = (vec2*)push_memory(arena, sizeof(vec2) * buffer_size);
    vec4* colors = (vec4*)push_memory(arena, sizeof(vec4) * buffer_size);

    Entity* entity = tile_map->entity;

    entity->scale = vec2(50.0f);
    entity->bounds = vec2(TILEMAP_SIZE, TILEMAP_SIZE);

    int i = 0;
    for (int y = 0; y < TILEMAP_SIZE; y++)
    {
        for (int x = 0; x < TILEMAP_SIZE; x++)
        {
            tile_map->tile_positions[x][y] = vec2(x, y);
            tile_map->tile_rects[x][y] = vec4(0, 0, 0, 0);
            tile_map->tile_vertices_indexes[x][y] = i * 6;
            tile_map->tile_types[x][y] = -1;
            
            i++;
        }
    }

    entity->mesh = make_custom_mesh(vertices, uvs, colors, buffer_size, arena);

    entity->mesh->bounds = vec4(0, 0, TILEMAP_SIZE, TILEMAP_SIZE);

    update_mesh(entity->mesh);
}