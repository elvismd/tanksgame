#include "asset_storage.h"

#include "file_io.h"
#include "logger.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

Texture blank_texture;
Shader blank_shader;

#define BlankShaderContents ""\
                "#ifdef VERTEX_SHADER \n"\
                "layout (location = 0) in vec3 position; \n"\
                "layout (location = 1) in vec2 texCoords;\n"\
                "uniform mat4 model;                     \n"\
                "uniform mat4 vp;                        \n"\
                "void main() \n"\
                "{ \n"\
                "gl_Position = vp * model * vec4(position, 1.0f); \n"\
                "} \n"\
                "#endif \n"\
                "#ifdef FRAGMENT_SHADER \n"\
                "out vec4 frag_color; \n"\
                "void main() \n"\
                "{ \n"\
                "frag_color = vec4(1, 1, 1, 1); \n"\
                "} \n"\
                "#endif \n"

    
#define SHADER_CORE "#version 330 core\n"
Shader load_shader(const char* file_path, Memory_Arena* arena)
{
    File_Result shader_file = load_file(file_path, arena);
    Shader result = {};

    snprintf(result.file_path, array_count(result.file_path), file_path);
    snprintf(result.vertex_src, array_count(result.vertex_src), "%s%s%s", SHADER_CORE, "#define VERTEX_SHADER\n", shader_file.contents);
    snprintf(result.fragment_src, array_count(result.fragment_src), "%s%s%s", SHADER_CORE, "#define FRAGMENT_SHADER\n", shader_file.contents);

    return result;
}

Shader get_blank_shader()
{
    Shader result = {};

    snprintf(result.file_path, array_count(result.file_path), "blank_shader");
    snprintf(result.vertex_src, array_count(result.vertex_src), "%s%s%s", SHADER_CORE, "#define VERTEX_SHADER\n", BlankShaderContents);
    snprintf(result.fragment_src, array_count(result.fragment_src), "%s%s%s", SHADER_CORE, "#define FRAGMENT_SHADER\n", BlankShaderContents);

    return result;
}

Texture load_texture(const char* file_path)
{
    Texture result = {};

    int width, height, nr_components;
    unsigned char* texture_data = stbi_load(file_path, &width, &height, &nr_components, 0);

    snprintf(result.file_path, array_count(result.file_path), file_path);

    if(texture_data)
    {
        result.width = width;
        result.height = height;
        result.nr_components = nr_components;
        result.contents = texture_data;

        log_info("Loaded file: %s", file_path);
    }
    else 
        log_warning("Unable to read file: %s", file_path);

    return result;
}

Texture get_blank_texture()
{
    Texture result = {};

    snprintf(result.file_path, array_count(result.file_path), "blank_texture");

    result.width = 0;
    result.height = 0;
    result.nr_components = 4;
    result.contents = 0;

    return result;
}

void load_game_textures(char** paths, int size, Asset_Storage* asset_storage)
{
    Assert(size < array_size(asset_storage->textures));

    blank_texture = get_blank_texture();

    for(int i = 0; i < size; i++)
    {   
        char* path = paths[i];
        array_add(asset_storage->textures, load_texture(path));
    }
}

void load_game_shaders(char** paths, int size, Asset_Storage* asset_storage, Memory_Arena* arena)
{
    Assert(size < array_size(asset_storage->shaders));

    blank_shader = get_blank_shader();

    for(int i = 0; i < size; i++)
    {   
        char* path = paths[i];
        array_add(asset_storage->shaders, load_shader(path, arena));
    }
}

Texture* get_texture(const char* path, Asset_Storage* asset_storage, b32 point_filter)
{
    for(int i = 0; i < asset_storage->textures.length; i++)
    {
        Texture* texture = &array_item(asset_storage->textures, i);
        if(str_equal_to(texture->file_path, path))
        {
            texture->point_filter = point_filter;
            return texture;
        }
    }

    log_warning("Could not find texture on storage: %s", path);
    return &blank_texture;
}

Shader* get_shader(const char* path, Asset_Storage* asset_storage)
{
    for(int i = 0; i < asset_storage->shaders.length; i++)
    {
        Shader* shader = &array_item(asset_storage->shaders, i);
        if(str_equal_to(shader->file_path, path))
            return shader;
    }

    log_warning("Could not find shader on storage: %s", path);
    return &blank_shader;
}

vec4 get_texture_rect_from_index(u32 index, u32 tile_width, u32 tile_height, Texture* texture)
{
    u32 w = texture->width / tile_width;
    u32 h = texture->height / tile_height;

    r32 x = index % w;
    r32 y = floor((r32)index / (r32)w);

    vec4 src = vec4(x * tile_width, texture->height - (y * tile_height) - tile_height, tile_width, tile_height);
    src.x /= (r32)texture->width;
    src.y /= (r32)texture->height;
    src.z /= (r32)texture->width;
    src.w /= (r32)texture->height;

    return src;
}