#pragma once

#include "agnostic.h"
#include "math.h"
#include "game.h"
#include "array.h"
#include "memory_arena.h"
#include "opengl.h"

struct Renderer_State
{
    Mesh* quad_vao;
    u32 line_vao, line_vbo;

    u32 current_program;
    mat4 current_projection;

    Shader* current_shader;
};

Mesh* make_custom_mesh(vec3* vertices, vec2* uvs, vec4* colors, u32 buffer_size, Memory_Arena* arena);
Mesh* make_quad(r32 w, r32 h, Memory_Arena* arena);
void update_mesh(Mesh* mesh);
void set_mesh_rect(Mesh* mesh, Texture* texture, int index, int frame_width, int frame_height);

void begin_game_view(u32 window_width, u32 window_height);
void end_game_view();

void set_projection(mat4 projection);

void draw_line(Shader* shader, vec3 from, vec3 to, vec4 color, float scale = 8.0f);
void draw_line(Shader* shader, vec2 from, vec2 to, vec4 color, float scale = 8.0f);
void draw_quad(Shader* shader, vec2 position, float width, float height, vec4 color);

void draw_entity_mesh(Shader* shader, Entity* entity);