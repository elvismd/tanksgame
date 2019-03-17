#pragma once

#include "agnostic.h"
#include "array.h"
#include "memory_arena.h"
#include "asset_storage.h"

enum Enity_Flags 
{
	NONE = 0x1,
	STATIC = 0x2,
	PHYSICAL = 0x4,
	DRAWABLE = 0x8,
	VISIBLE = 0x10,
};

struct Entity
{
	u32 flags;

	char name[1024];

	vec2 position;
	vec2 scale;
	float rotation;

	vec2 bounds;
	vec2 velocity;
	b32 use_gravity;

	Mesh* mesh;
	Texture* texture;

	vec4 color;

	u32 derived_type;
	void* derived_ptr;
};

struct Entity_Manager
{
    Static_Array(Entity, 32) entities;
};

#define USING_ENTITY(type) static int get_entity_type() { return type; } Entity* entity;

Entity* get_next_entity(void* derived_ptr, u32 type, Entity_Manager* entity_manager);

Entity* get_entity_by_name(const char* name, Entity_Manager* entity_manager);

template <typename T>
T* new_entity(Entity_Manager* entity_manager, Memory_Arena* memory_arena)
{
    T* result = push_struct(memory_arena, T);
    result->entity = get_next_entity(result, T::get_entity_type(), entity_manager);

    return result;
}