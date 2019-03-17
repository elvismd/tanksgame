#pragma once

#define GLFW_INCLUDE_NONE
#include <glfw/glfw3.h> 

#include <cmath>
#include <stdlib.h>
#include "agnostic.h"
#include "logger.h"
#include "array.h"
#include "input.h"

#include "file_io.h"
#include "asset_storage.h"
#include "memory_arena.h"
#include "entity_manager.h"
#include "entities.h"

#include <cute_filewatch.h>

inline Game_Controller_Input *get_controller(Game_Input* input, int index)
{
	Assert(index < array_count(input->controllers));
	Game_Controller_Input *result = &input->controllers[index];

	return result;
}

struct Game_Memory
{
	b32 is_initialized;

	u64 permanent_storage_size;
	void *permanent_storage;
	memory_index permanent_used_size;

	u64 transient_storage_size;
	void *transient_storage;
	memory_index transient_used_size;
};

struct Camera 
{
	vec2 position;
	float scale;

	int width, height;

	mat4 projection;
};

struct Game_State
{
	Memory_Arena main_arena;
	Asset_Storage asset_storage;
	Entity_Manager entity_manager;
	filewatch_t* filewatch;

	b32 editor_mode;

	Camera* camera;
	Shader* sprite_shader;
	Shader* line_shader;
};

void game_update_and_render(Game_Memory *memory, Game_Input *input, vec2 window_size);