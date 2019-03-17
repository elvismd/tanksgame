#pragma once

#include "agnostic.h"
#include "array.h"
#include "memory_arena.h"

struct Texture 
{
	b32 initialized;

	char file_path[1024];
	u32 width, height;
	u32 nr_components;
	uchar* contents;
	b32 flip;
	b32 point_filter;

	u32 gl_id;
};

struct Shader 
{
	b32 initialized;

	char file_path[1024];
	char vertex_src[2048];
	char fragment_src[2048];

	u32 gl_id;
};

struct Mesh 
{
	float width;
	float height;

	vec3* vertices;
	vec2* uvs;
	vec4* colors;

	u32 buffer_size;

	vec4 bounds;

	struct 
	{
		u32 vao;
		u32 vbo;
		u32 ubo;
		u32 cbo;
	} gl_buffers;
};

struct Asset_Storage
{
	Static_Array(Texture, 32) textures;
	Static_Array(Shader, 4) shaders;
};

Shader load_shader(const char* file_path, Memory_Arena* arena);
Shader get_blank_shader();
Texture load_texture(const char* file_path);
Texture get_blank_texture();

void load_game_textures(char** paths, int size, Asset_Storage* asset_storage);
void load_game_shaders(char** paths, int size, Asset_Storage* asset_storage, Memory_Arena* arena);

Texture* get_texture(const char* path, Asset_Storage* asset_storage, b32 point_filter = false);
Shader* get_shader(const char* path, Asset_Storage* asset_storage);

vec4 get_texture_rect_from_index(u32 index, u32 tile_width, u32 tile_height, Texture* texture);