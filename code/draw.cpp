#include "draw.h"

#include <stdio.h>
#include <stddef.h>
#include "logger.h"

b32 initialized_vao = false;
Renderer_State renderer_state;

internal void compile_texture(Texture* texture)
{
	u32 id;
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	
	CHECK_GL();

	//GL_LINEAR
	if(texture->point_filter)
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	}
	else 
	{
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	}

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	if (texture->nr_components > 3) glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture->width, texture->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, texture->contents);
	else glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture->width, texture->height, 0, GL_RGB, GL_UNSIGNED_BYTE, texture->contents);

    glGenerateMipmap(GL_TEXTURE_2D);
	
	CHECK_GL();

	glBindTexture(GL_TEXTURE_2D, 0);

	texture->gl_id = id;
}

internal void compile_shader(Shader* shader)
{
	uint32 vs = glCreateShader(GL_VERTEX_SHADER);
	uint32 fs = glCreateShader(GL_FRAGMENT_SHADER);

	u32 id = glCreateProgram();

	char* vs_source = shader->vertex_src;
	char* fs_source = shader->fragment_src;

	glShaderSource(vs, 1, &vs_source, NULL);
	glShaderSource(fs, 1, &fs_source, NULL);

	glCompileShader(vs);
	glCompileShader(fs);

	CHECK_GL();

	int status;
	char log[1024];

	glGetShaderiv(vs, GL_COMPILE_STATUS, &status);
	if (!status)
	{
		glGetShaderInfoLog(vs, 1024, NULL, log);
		log_error("Shader %s - Vertex Shader error: \n %s", shader->file_path, log);
	}

	glGetShaderiv(fs, GL_COMPILE_STATUS, &status);
	if (!status)
	{
		glGetShaderInfoLog(fs, 1024, NULL, log);
		log_error("Shader %s - Fragment Shader error: \n %s", shader->file_path, log);
	}

	glAttachShader(id, vs);
	glAttachShader(id, fs);
	
	glLinkProgram(id);

	glGetProgramiv(id, GL_LINK_STATUS, &status);
	if (!status)
	{
		glGetProgramInfoLog(id, 1024, NULL, log);
		log_error("Shader %s - Program linking error: \n %s", shader->file_path, log);
	}

	glDeleteShader(vs);
	glDeleteShader(fs);

	CHECK_GL();

	shader->gl_id = id;
	// int nrAttributes, nrUniforms;
	// glGetProgramiv(shader->id, GL_ACTIVE_ATTRIBUTES, &nrAttributes);
	// glGetProgramiv(shader->id, GL_ACTIVE_UNIFORMS, &nrUniforms);
}	

Mesh* make_custom_mesh(vec3* vertices, vec2* uvs, vec4* colors, u32 buffer_size, Memory_Arena* arena)
{
	Mesh* mesh = push_struct(arena, Mesh);

	mesh->buffer_size = buffer_size;
	mesh->vertices = vertices;
	mesh->uvs = uvs;
	mesh->colors = colors;

	glGenVertexArrays(1, &mesh->gl_buffers.vao);
	glBindVertexArray(mesh->gl_buffers.vao);
	
	Assert(mesh->vertices != NULL);

	glGenBuffers(1, &mesh->gl_buffers.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->gl_buffers.vbo);
	glBufferData(GL_ARRAY_BUFFER, mesh->buffer_size * sizeof(vec3), &mesh->vertices[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(0);

	if(mesh->uvs != NULL)
	{
		glGenBuffers(1, &mesh->gl_buffers.ubo);
		glBindBuffer(GL_ARRAY_BUFFER, mesh->gl_buffers.ubo);
		glBufferData(GL_ARRAY_BUFFER, mesh->buffer_size * sizeof(vec2), &mesh->uvs[0], GL_STATIC_DRAW);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(1);
	}

	if(mesh->colors != NULL)
	{
		glGenBuffers(1, &mesh->gl_buffers.cbo);
		glBindBuffer(GL_ARRAY_BUFFER, mesh->gl_buffers.cbo);
		glBufferData(GL_ARRAY_BUFFER, mesh->buffer_size * sizeof(vec4), &mesh->colors[0], GL_STATIC_DRAW);
		glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, nullptr);
		glEnableVertexAttribArray(2);
	}

	return mesh;
}

Mesh* make_quad(r32 w, r32 h, Memory_Arena* arena)
{
	Mesh* mesh = push_struct(arena, Mesh);

	mesh->width = w;
	mesh->height = h;
	mesh->buffer_size = 4;

	mesh->vertices = (vec3*)push_memory(arena, sizeof(vec3) * mesh->buffer_size);
	mesh->uvs = (vec2*)push_memory(arena, sizeof(vec2) * mesh->buffer_size);

	mesh->bounds = vec4(-w * 0.5f, -h * 0.5f, w, h);

	w *= 0.5f;
	h *= 0.5f;

	mesh->vertices[0] = vec3(-w, h, 0.0f);
	mesh->vertices[1] = vec3(-w, -h, 0.0f);
	mesh->vertices[2] = vec3(w, h, 0.0f);
	mesh->vertices[3] = vec3(w, -h, 0.0f);

	mesh->uvs[0] = vec2(0.0f, 1.0f);
	mesh->uvs[1] = vec2(0.0f, 0.0f);
	mesh->uvs[2] = vec2(1.0f, 1.0f);
	mesh->uvs[3] = vec2(1.0f, 0.0f);

	glGenVertexArrays(1, &mesh->gl_buffers.vao);
	glBindVertexArray(mesh->gl_buffers.vao);

	glGenBuffers(1, &mesh->gl_buffers.vbo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->gl_buffers.vbo);
	glBufferData(GL_ARRAY_BUFFER, mesh->buffer_size * sizeof(vec3), &mesh->vertices[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &mesh->gl_buffers.ubo);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->gl_buffers.ubo);
	glBufferData(GL_ARRAY_BUFFER, mesh->buffer_size * sizeof(vec2), &mesh->uvs[0], GL_STATIC_DRAW);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(1);

	glBindVertexArray(0);

	return mesh;
}

void set_mesh_rect(Mesh* mesh, Texture* texture, int index, int frame_width, int frame_height)
{
	vec4 src = get_texture_rect_from_index(index, frame_width, frame_height, texture);

	int w = frame_width / 2;
	int h = frame_height / 2;

	mesh->vertices[0] = vec3(-w, h, 0.0f);
	mesh->vertices[1] = vec3(-w, -h, 0.0f);
	mesh->vertices[2] = vec3(w, h, 0.0f);
	mesh->vertices[3] = vec3(w, -h, 0.0f);

	mesh->uvs[0] = vec2(src.x, src.y + src.w);
	mesh->uvs[1] = vec2(src.x, src.y);
	mesh->uvs[2] = vec2(src.x + src.z, src.y + src.w);
	mesh->uvs[3] = vec2(src.x + src.z, src.y);
}

void update_mesh(Mesh* mesh)
{
	glBindBuffer(GL_ARRAY_BUFFER, mesh->gl_buffers.vbo);
	glBufferData(GL_ARRAY_BUFFER, mesh->buffer_size * sizeof(vec3), &mesh->vertices[0], GL_STATIC_DRAW);

	if(mesh->uvs != NULL)
	{
		glBindBuffer(GL_ARRAY_BUFFER, mesh->gl_buffers.ubo);
		glBufferData(GL_ARRAY_BUFFER, mesh->buffer_size * sizeof(vec2), &mesh->uvs[0], GL_STATIC_DRAW);
	}

	if(mesh->colors != NULL)
	{
		glBindBuffer(GL_ARRAY_BUFFER, mesh->gl_buffers.cbo);
		glBufferData(GL_ARRAY_BUFFER, mesh->buffer_size * sizeof(vec4), &mesh->colors[0], GL_STATIC_DRAW);
	}
}

void make_line(u32* out_vao, u32* out_vbo)
{
	u32 vao, vbo;
	glGenVertexArrays(1, &vao);
	glGenBuffers(1, &vbo);
	glBindVertexArray(vao);

	vec3 positions[1] = 
	{
		vec3(0, 0, 0)
	};

	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * 1, &positions[0], GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);

	*out_vao = vao;
	*out_vbo = vbo;

	CHECK_GL();
}

void begin_game_view(u32 window_width, u32 window_height)
{
	if(!initialized_vao)
	{
		glEnable(GL_LINE_SMOOTH);

		make_line(&renderer_state.line_vao, &renderer_state.line_vbo);

		initialized_vao = true;
	}

	glViewport(0, 0, window_width, window_height);

	glClearColor(1.0f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void end_game_view()
{
	
}

void set_projection(mat4 projection)
{
	renderer_state.current_projection = projection;
}

inline void set_current_program(Shader* shader)
{
	if(!shader->initialized)
	{
		compile_shader(shader);
		shader->initialized = true;
	}

	if(shader != renderer_state.current_shader)
	{
		glUseProgram(shader->gl_id);

		renderer_state.current_shader = shader;
		renderer_state.current_program = shader->gl_id;

		CHECK_GL();	
	}
}

internal vec3* mount_lines(vec3 from, vec3 to, float scale)
{ 
    vec3 dir = to - from;
	vec3 half_dir = dir * 0.5f;

    float angle = atan2f(dir.y, dir.x) + radians(90.0f);

    r32 w = scale, h = (magnitude_vector(half_dir));
    vec3 line_points[4] = 
    {
        vec3(-w, h, 0.0f),
		vec3(-w, -h, 0.0f),
		vec3(w, h, 0.0f),
		vec3(w, -h, 0.0f)
    };

    for(int i = 0; i < 4; i++)
    {
        line_points[i] = rotate_point(line_points[i], angle);
        line_points[i] = (from + half_dir) + line_points[i];
    }

    return line_points;
}

void draw_line(Shader* shader, vec2 from, vec2 to, vec4 color, float scale)
{
	draw_line(shader, vec3(from.x, from.y, 0.0f), vec3(to.x, to.y, 0.0f), color, scale);
}

void draw_line(Shader* shader, vec3 from, vec3 to, vec4 color, float scale)
{
	Static_Array(vec3, 4) points;

    auto lines = mount_lines(from, to, scale);
    for(int i = 0; i < 4; i++)
        array_add(points, lines[i]);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	set_current_program(shader);
	u32 shader_id = renderer_state.current_program;

	static mat4 model = mat4(1.0);
	model = translate(mat4(1.0), vec3(0.0f));
	
	u32 uniform_location = glGetUniformLocation(shader_id, "vp");
	glUniformMatrix4fv(uniform_location, 1, GL_FALSE, &renderer_state.current_projection[0][0]);

	uniform_location = glGetUniformLocation(shader_id, "model");
	glUniformMatrix4fv(uniform_location, 1, GL_FALSE, &model[0][0]);

	uniform_location = glGetUniformLocation(shader_id, "color");
	glUniform4fv(uniform_location, 1, &color[0]);

	CHECK_GL();
	
	glBindVertexArray(renderer_state.line_vao);
	
	glBindBuffer(GL_ARRAY_BUFFER, renderer_state.line_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * 4, &points.a[0], GL_STATIC_DRAW);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	CHECK_GL();
}

void draw_quad(Shader* shader, vec2 position, float width, float height, vec4 color)
{
	vec3 positions[4] = 
    {
        vec3( 0,  height, 0.0f),
		vec3( 0, 0, 0.0f),
		vec3( width,  height, 0.0f),
		vec3( width, 0, 0.0f)
    };

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	set_current_program(shader);
	u32 shader_id = renderer_state.current_program;

	static mat4 model = mat4(1.0);
	model = translate(mat4(1.0), vec3(position.x, position.y, 0.0f));
	
	u32 uniform_location = glGetUniformLocation(shader_id, "vp");
	glUniformMatrix4fv(uniform_location, 1, GL_FALSE, &renderer_state.current_projection[0][0]);

	uniform_location = glGetUniformLocation(shader_id, "model");
	glUniformMatrix4fv(uniform_location, 1, GL_FALSE, &model[0][0]);

	uniform_location = glGetUniformLocation(shader_id, "color");
	glUniform4fv(uniform_location, 1, &color[0]);

	CHECK_GL();
	
	glBindVertexArray(renderer_state.line_vao);
	
	glBindBuffer(GL_ARRAY_BUFFER, renderer_state.line_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * 4, &positions[0], GL_STATIC_DRAW);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

	CHECK_GL();
}

void draw_entity_mesh(Shader* shader, Entity* entity)
{
	if(entity->mesh == NULL) return;

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	Texture* texture = entity->texture;

	if(texture && !texture->initialized)
	{
		compile_texture(texture);
		texture->initialized = true;
	}

	set_current_program(shader);
	u32 shader_id = renderer_state.current_program;

	static mat4 entity_model = mat4(1.0);
	static vec3 entity_position = vec3(0.0f);
	static vec3 entity_scale = vec3(1.0f);

	entity_position.x = entity->position.x;
	entity_position.y = entity->position.y;

	entity_scale.x = entity->scale.x;
	entity_scale.y = entity->scale.y;

	entity_model = translate(mat4(1.0), entity_position);
	entity_model = glm::rotate(entity_model, glm::radians(entity->rotation), vec3(0.0f, 0.0f, 1.0f));
    entity_model = scale(entity_model, entity_scale);

	// Set uniforms
	u32 uniform_location = glGetUniformLocation(shader_id, "vp");
	glUniformMatrix4fv(uniform_location, 1, GL_FALSE, &renderer_state.current_projection[0][0]);

	uniform_location = glGetUniformLocation(shader_id, "model");
	glUniformMatrix4fv(uniform_location, 1, GL_FALSE, &entity_model[0][0]);

	uniform_location = glGetUniformLocation(shader_id, "color");

	glUniform4fv(uniform_location, 1, &entity->color[0]);

	if(texture)
	{
		uniform_location = glGetUniformLocation(shader_id, "image");
		glBindTexture(GL_TEXTURE_2D, texture->gl_id);
		glUniform1i(uniform_location, 0);
	}

	CHECK_GL();

	GLenum topology = GL_TRIANGLE_STRIP;
	if(entity->derived_type == TILEMAP_ENTITY)
		topology = GL_TRIANGLES;
	
	// Draw the VAO
	glBindVertexArray(entity->mesh->gl_buffers.vao);
	glDrawArrays(topology, 0, entity->mesh->buffer_size);

	glBindTexture(GL_TEXTURE_2D, 0);
	glBindVertexArray(0);

	CHECK_GL();
}