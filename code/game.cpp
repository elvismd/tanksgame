#include "game.h"

#include "draw.h"
#include "player_control.h"
#include "enemy_ai.h"
#include "map.h"
#include "tilemap.h"

#include "utils.h"

#define GRAVITY 100

#define TILE_GROUND 0
#define TILE_OBSTACLE 1

vec2 old_window_size;
Player* player_entity;
Tile_Map* main_tile_map;
Entity* last_selected_entity;
vec2 diff_on_selection;

#define CUTE_FILEWATCH_FREE(ptr, ctx)
#define ASSETSYS_FREE( ctx, ptr )

#define CUTE_FILEWATCH_MALLOC(size, ctx) alloc_custom_memory(size, ctx)
#define ASSETSYS_MALLOC( ctx, size ) ( alloc_custom_memory( size, ctx ) )
internal void* alloc_custom_memory(size_t size, void* ctx)
{
    Memory_Arena* arena = (Memory_Arena*)ctx;
    void* result = push_memory(arena, size);

    return result;
}

#define ASSETSYS_IMPLEMENTATION
#define STRPOOL_IMPLEMENTATION
#include <assetsys.h>

#define CUTE_FILEWATCH_IMPLEMENTATION
#define STRPOOL_EMBEDDED_IMPLEMENTATION
#include <cute_filewatch.h>

void watch_callback(filewatch_update_t change, const char* virtual_path, void* udata);

void get_game_variables(const char* path, Game_State* game_state)
{
    File_Result file_result = load_temp_file(path);
    Text_File_Handler text_handler = get_text_file_handler(&file_result);

    char temp[2048];
    memset(temp, 0, 2048);

    #define attempt_to_record_variable(str, var, func)\
        if(word_is(&text_handler, str))\
        {\
            consume_word(&text_handler);\
            snprintf(temp, 2048, "%.*s", text_handler.word_length, text_handler.word);\
            var = func(temp);\
        }\

    while(can_consume_lines(&text_handler))
    {     
        consume_word(&text_handler);

        attempt_to_record_variable("movement_speed", player_entity->movement_speed, atof);
        attempt_to_record_variable("camera_scale", game_state->camera->scale, atof);
    }

    #undef attempt_to_record_variable

    free(file_result.contents);
}

void load_level(const char* path, Tile_Map* tile_map, Game_State* game_state)
{
    File_Result file_result = load_temp_file(path);
    Text_File_Handler text_handler = get_text_file_handler(&file_result);

    int x = 0;
    int y = 0;

    eat_all_white_space(&text_handler);

    int counter = 0;
    bool was_eol = false;

    while(counter < text_handler.length)
    {
        char flag = text_handler.at[0];
        if(is_end_of_line(flag))
        {
            was_eol = true;
        }
        else 
        {
            if(was_eol)
            {
                x = 0;
                y++;
                was_eol = false;

                //printf("\n");
            }

            if(is_number(flag))
            {
                //printf("%c", flag);
                int tile_flag = flag - '0';
                int tile_index = 64;

                if(tile_flag == TILE_OBSTACLE) tile_index = 68; 

                add_tile(tile_flag, vec2(x, y), tile_index, tile_map);
            }
            x++;
        }

        counter++;
        ++text_handler.at;
    }

    printf("\n");

    free(file_result.contents);
}

internal void setup_camera(Camera* camera, vec2 window_size)
{
    camera->projection = ortho(0.0f, window_size.x, 0.0f, window_size.y, -1.0f, 1.0f);
    glm::vec3 translate(-camera->position.x + window_size.x / 2, -camera->position.y + window_size.y / 2, 0.0f);
    camera->projection = glm::translate(camera->projection, translate);
    vec3 scale_vec(camera->scale, camera->scale, 0.0f);
    camera->projection = scale(glm::mat4(1.0f), scale_vec) * camera->projection;
}

internal b32 rect_collide_with_another_entity(vec4 rect, u32 ignore_index, Entity_Manager* entity_manager)
{
    for(u32 j = 0; j < entity_manager->entities.length; j++)
    {
        if(j == ignore_index) continue;

        Entity* entity1 = &(array_item(entity_manager->entities, j));

        if(!(entity1->flags & PHYSICAL)) continue;

        vec4 r1 = vec4(entity1->position.x, entity1->position.y, entity1->bounds.x * entity1->scale.x, entity1->bounds.y * entity1->scale.y);
        r1.x -= r1.z * 0.5f;
        r1.y -= r1.w * 0.5f;

        if(rect_overlap_rect(rect, r1))
            return true;
    }

    return false;
}

internal b32 rect_collide_with_tilemap(vec4 rect, Tile_Map* tile_map)
{
    rect.x -= tile_map->entity->position.x;
    rect.y -= tile_map->entity->position.y;

    u32 px = (int)ceil(rect.x / tile_map->entity->scale.x);
    u32 py = (int)ceil(rect.y / tile_map->entity->scale.y);

    u32 ox = 0;
    u32 oy = 0;

    s32 accuracy = 4;

     for (s32 y = -accuracy; y <= accuracy; y++)
     {
         for (s32 x = -accuracy; x <= accuracy; x++)
         {
            ox = px + x;
            oy = py + y;

            if(ox >= 0 && oy >= 0 && ox < TILEMAP_SIZE && oy < TILEMAP_SIZE)
            {
                int type = tile_map->tile_types[ox][oy];
                if (type == TILE_OBSTACLE)
                {
                    vec2 pos = tile_map->tile_positions[ox][oy];

                    pos.x *= tile_map->entity->scale.x;
                    pos.y *= tile_map->entity->scale.y;

                    vec4 bounds = { pos.x, pos.y, tile_map->entity->scale.x , tile_map->entity->scale.y };
                    // bounds.x -= bounds.z * 0.5f;
                    // bounds.y -= bounds.w * 0.5f;

                    if (rect_overlap_rect(bounds, rect)) return true;
                }
            }
         }
     }

     return false;
}

inline vec2 screen_to_world_coord(Camera* cam, vec2 screen_pos)
{
    screen_pos.y = cam->height - screen_pos.y;
    screen_pos -= vec2(cam->width / 2, cam->height / 2);
    
    screen_pos /= cam->scale;
    screen_pos += cam->position;
    
    return screen_pos;
}

void game_update_and_render(Game_Memory *memory, Game_Input *input, vec2 window_size)
{
     Game_State* game_state = NULL;

    if(!memory->is_initialized)
    {
        Assert(sizeof(Game_State) <= memory->permanent_storage_size);

        game_state = (Game_State*)memory->permanent_storage;

        initialize_arena(&game_state->main_arena,     
                        memory->permanent_storage_size - sizeof(Game_State),   
                        (u8*)memory->permanent_storage + sizeof(Game_State));

        assetsys_t* assetsys = assetsys_create(&game_state->main_arena);
        filewatch_t* filewatch = filewatch_create(assetsys, &game_state->main_arena);

        filewatch_mount(filewatch, "./data", "/data");
        filewatch_start_watching(filewatch, "/data", watch_callback, game_state);

        char* game_textures[4] = 
        {
            "data/tank_spritesheet.png",
            "data/train.png",
            "data/Kgw6C.png",
            "data/dk.png"
        };
        load_game_textures(game_textures, array_count(game_textures), &game_state->asset_storage);

        char* game_shaders[2] = 
        {
            "data/sprite.glsl",
            "data/line.glsl"
        };
        load_game_shaders(game_shaders, array_count(game_shaders), &game_state->asset_storage, &game_state->main_arena);

        Camera* camera = game_state->camera;
        camera = push_struct(&game_state->main_arena, Camera);
        camera->scale = 0.3f;
        camera->width = window_size.x;
        camera->height = window_size.y;

        setup_camera(camera, window_size);
        
        game_state->camera = camera;

        game_state->sprite_shader = get_shader("data/sprite.glsl", &game_state->asset_storage);
        game_state->line_shader = get_shader("data/line.glsl", &game_state->asset_storage);

         // === TileMap Setup
        Tile_Map* tile_map = new_entity<Tile_Map>(&game_state->entity_manager, &game_state->main_arena);
        Entity* entity = tile_map->entity;
        entity->texture = get_texture("data/Kgw6C.png", &game_state->asset_storage, true);
        snprintf(entity->name, 1024, "main_tile_map");

        main_tile_map = tile_map;

        init_tilemap(tile_map, &game_state->main_arena);

        load_level("data/map.csv", tile_map, game_state);

        entity->position -= vec2(entity->scale.x * ((r32)70 / 2));

        update_mesh(entity->mesh);

        // === Player Setup
        Player* player = new_entity<Player>(&game_state->entity_manager, &game_state->main_arena);
        entity = player->entity;
        player_entity = player;

        player->movement_speed = 74;

        entity->texture = get_texture("data/tank_spritesheet.png", &game_state->asset_storage);
        entity->mesh = make_quad(entity->texture->width, entity->texture->height, &game_state->main_arena);
        entity->position = vec2(0, 0);
        entity->scale = vec2(0.5f);
        entity->flags |= PHYSICAL;
        entity->bounds = vec2(entity->texture->width / 2, entity->texture->height * 0.9f);

        set_mesh_rect(entity->mesh, entity->texture, 0, entity->texture->width / 2, entity->texture->height);
        update_mesh(entity->mesh);

        // === Cannon Setup
        Cannon* cannon = new_entity<Cannon>(&game_state->entity_manager, &game_state->main_arena);
        entity = cannon->entity;

        player->cannon_entity = entity;

        entity->position = player->entity->position;
        entity->texture = get_texture("data/tank_spritesheet.png", &game_state->asset_storage);
        entity->mesh = make_quad(entity->texture->width, entity->texture->height, &game_state->main_arena);
        entity->scale = vec2(0.5f);
        entity->bounds = vec2(entity->texture->width, entity->texture->height);

        set_mesh_rect(entity->mesh, entity->texture, 1, entity->texture->width / 2, entity->texture->height);
        update_mesh(entity->mesh);

        // === Projectiles Setup
        for(int i = 0; i < 10; i++)
        {
            Projectile* projectile = new_entity<Projectile>(&game_state->entity_manager, &game_state->main_arena);
            entity = projectile->entity;

            entity->position = player->entity->position;
            player->projectiles[i] = projectile;

            entity->texture = get_texture("data/dk.png", &game_state->asset_storage);
            entity->mesh = make_quad(entity->texture->width, entity->texture->height, &game_state->main_arena);

            entity->scale = vec2(0.4f);
            entity->bounds = vec2(entity->texture->width, entity->texture->height);

            entity->flags = DRAWABLE;
        }

        // === Enemies Setup
        {
            Enemy* enemy = new_entity<Enemy>(&game_state->entity_manager, &game_state->main_arena);
            entity = enemy->entity;
            enemy->target = player->entity;

            entity->texture = get_texture("data/tank_spritesheet.png", &game_state->asset_storage);
            entity->mesh = make_quad(entity->texture->width, entity->texture->height, &game_state->main_arena);

            set_mesh_rect(entity->mesh, entity->texture, 0, entity->texture->width / 2, entity->texture->height);
            update_mesh(entity->mesh);
            entity->position = vec2(200, 0);
            entity->scale = vec2(0.5f);
            entity->flags |= PHYSICAL;
            entity->bounds = vec2(entity->texture->width / 2, entity->texture->height * 0.9f);
            entity->color = vec4(1, 0, 0, 1);

            Cannon* enemy_cannon = new_entity<Cannon>(&game_state->entity_manager, &game_state->main_arena);
            entity = enemy_cannon->entity;

            enemy->cannon_entity = entity;

            entity = enemy_cannon->entity;
            entity->position = enemy->entity->position;
            entity->texture = get_texture("data/tank_spritesheet.png", &game_state->asset_storage);
            entity->mesh = make_quad(entity->texture->width, entity->texture->height, &game_state->main_arena);
            entity->scale = vec2(0.5f);
            entity->bounds = vec2(entity->texture->width, entity->texture->height);
            entity->color = vec4(1, 0, 0, 1);

            set_mesh_rect(entity->mesh, entity->texture, 1, entity->texture->width / 2, entity->texture->height);
            update_mesh(entity->mesh);
        }

        get_game_variables("data/game.vars", game_state);
        game_state->filewatch = filewatch;
        game_state->editor_mode = false;

        r64 game_size = sizeof(Game_State);
        r64 mb_game_size = (game_size) / 1000000.0;
        log_info("Game has %f bytes | %f megabytes", game_size, mb_game_size);
        log_arena(&game_state->main_arena);

        old_window_size = window_size;
        memory->is_initialized = true;
    }
    else game_state = (Game_State*)memory->permanent_storage;

    filewatch_update(game_state->filewatch);
    filewatch_notify(game_state->filewatch);

    auto controller = &input->controllers[0];

    Camera* camera = game_state->camera;
    setup_camera(camera, window_size);

    camera->position = player_entity->entity->position;

    vec2 world_mouse_pos = screen_to_world_coord(game_state->camera, controller->mouse_position);

    b32 moving_entities = controller->keys[GLFW_KEY_LEFT_CONTROL].is_down;

    if(!game_state->editor_mode)
    {
        // inputing and updating
        for(int i = 0; i < game_state->entity_manager.entities.length; i++)
        {
            Entity* entity = &(array_item(game_state->entity_manager.entities, i));

            switch(entity->derived_type)
            {
                case PLAYER_ENTITY:
                {
                    handle_player_movement(entity, &game_state->entity_manager, input);
                }
                break;
                case ENEMY_ENTITY:
                {
                    handle_enemy_movement(entity, &game_state->entity_manager, input);
                }
                break;
            }

            if(entity->flags & PHYSICAL)
            {
                vec2 velocity = entity->velocity;

                if(entity->use_gravity)
                    velocity.y -= GRAVITY;

                vec2 position = entity->position;
                vec2 future_position = position + velocity * input->delta_time * 1.2f;

                vec4 r0 = vec4(entity->position.x, entity->position.y, entity->bounds.x * entity->scale.x, entity->bounds.y * entity->scale.y);

                r0.x = future_position.x;
                r0.y = position.y;
                r0.x -= r0.z * 0.5f;
                r0.y -= r0.w * 0.5f;

                if(rect_collide_with_another_entity(r0, i, &game_state->entity_manager))
                    velocity.x = 0.0f;

                if(rect_collide_with_tilemap(r0, main_tile_map))
                    velocity.x = 0.0f;
                
                r0.x = position.x;
                r0.y = future_position.y;
                r0.x -= r0.z * 0.5f;
                r0.y -= r0.w * 0.5f;

                if(rect_collide_with_another_entity(r0, i, &game_state->entity_manager))
                    velocity.y = 0.0f;

                if(rect_collide_with_tilemap(r0, main_tile_map))
                    velocity.y = 0.0f;

                entity->velocity = velocity;

                entity->position += entity->velocity * input->delta_time;
            }
        }
    }
    else 
    {
        const int cam_movement_speed = 140.0f;
        if(controller->keys[GLFW_KEY_D].is_down)
            camera->position.x += cam_movement_speed * input->delta_time;
        else if(controller->keys[GLFW_KEY_A].is_down)
            camera->position.x -=  cam_movement_speed * input->delta_time;

        if(controller->keys[GLFW_KEY_W].is_down)
            camera->position.y +=  cam_movement_speed * input->delta_time;
        else if(controller->keys[GLFW_KEY_S].is_down)
            camera->position.y -=  cam_movement_speed * input->delta_time;

        if(controller->keys[GLFW_KEY_Q].is_down)
            camera->scale -=  cam_movement_speed * 0.005f * input->delta_time;
        else if(controller->keys[GLFW_KEY_E].is_down)
            camera->scale +=  cam_movement_speed * 0.005f * input->delta_time;

        camera->scale = glm::clamp(camera->scale, 0.0f, 2.0f);

        if(!moving_entities)
        {
            if(controller->mouse_left.is_down || controller->mouse_right.is_down)
            {
                //printf("Screen mouse pos: %f %f \n", controller->mouse_position.x, controller->mouse_position.y);

                Entity* tile_map_entity = get_entity_by_name("main_tile_map", &game_state->entity_manager);
                if(tile_map_entity != NULL)
                {
                    Tile_Map* tile_map = (Tile_Map*)tile_map_entity->derived_ptr;
                    
                    //printf("World mouse pos: %f %f \n", world_mouse_pos.x, world_mouse_pos.y);

                    vec2 local_pos = world_mouse_pos - tile_map_entity->position;
                    local_pos /= tile_map_entity->scale;

                    if(controller->mouse_left.is_down)
                    {
                        vec4 src_rect = vec4(18.0f, 18.0f, 16.0f / tile_map_entity->texture->width, 16.0f / tile_map_entity->texture->height);

                        add_tile(0, local_pos, src_rect, tile_map);
                    }
                    else if(controller->mouse_right.is_down)
                    {
                        remove_tile(local_pos, tile_map);
                    }

                    update_mesh(tile_map_entity->mesh);
                }
            }
        }
    }
    

    // drawing
    begin_game_view((u32)window_size.x, (u32)window_size.y);
    set_projection(game_state->camera->projection);

    for(int i = 0; i < game_state->entity_manager.entities.length; i++)
    {
        Entity* entity = &(array_item(game_state->entity_manager.entities, i));

        if(!(entity->flags & DRAWABLE && entity->flags & VISIBLE)) continue;

        draw_entity_mesh(game_state->sprite_shader, entity);
    }

    world_mouse_pos = screen_to_world_coord(game_state->camera, controller->mouse_position);

    if(game_state->editor_mode)
    {
        // debug
        for(int i = 0; i < game_state->entity_manager.entities.length; i++)
        {
            Entity* entity = &(array_item(game_state->entity_manager.entities, i));

            if(entity->flags & PHYSICAL) 
            {
                // draw_quad( game_state->line_shader, 
                //             entity->position,
                //             (entity->bounds.x * 0.5f) * entity->scale.x,
                //             (entity->bounds.y * 0.5f) * entity->scale.y, 
                //             vec4(0.2f, 1.0f, 0.2f, 0.5f));
            }
                
            if(entity->mesh)
            {
                vec4 bounds = entity->mesh->bounds;
                vec2 p = entity->position + vec2(bounds.x, bounds.y);
                vec2 s = vec2(bounds.z, bounds.w) * entity->scale;

                if(selecting_rect(world_mouse_pos, p, s.x, s.y)
                    || last_selected_entity == entity)
                {
                    vec4 color = vec4(1.0f, 0.2f, 0.2f, 0.5f);

                    if(controller->mouse_left.was_down)
                    {
                        last_selected_entity = entity;

                        diff_on_selection = entity->position - world_mouse_pos;
                    }
                    else
                    {
                        color = vec4(0.2f, 0.2f, 1.0f, 0.5f);
                    }

                    if(last_selected_entity == entity)
                        color = vec4(0.5f, 0.2f, 0.5f, 0.5f);

                    draw_quad(game_state->line_shader, p, s.x, s.y, color);
                }

                float transform_selectors_scale = 2.0f - camera->scale;

                draw_quad( game_state->line_shader, 
                    entity->position + vec2(30.0f * transform_selectors_scale, 0.0f),
                    30.0f * transform_selectors_scale,
                    2.0f * transform_selectors_scale, 
                    vec4(1.0f, 0.0f, 0.0f, 1.0f));

                draw_quad( game_state->line_shader, 
                    entity->position + vec2(0.0f, 30.0f * transform_selectors_scale),
                    2.0f * transform_selectors_scale,
                    30.0f * transform_selectors_scale, 
                    vec4(0.0f, 1.0f, 0.0f, 1.0f));
            }
        }

        if(!controller->mouse_left.is_down)
            last_selected_entity = NULL;

        if(controller->mouse_left.is_down && controller->mouse_is_moving && moving_entities)
        {
            if(last_selected_entity != NULL)
            {
                last_selected_entity->position = world_mouse_pos + diff_on_selection;
            }
        }
    }

    // draw_line(  game_state->line_shader, 
    //             player_entity->entity->position, 
    //             player_entity->entity->position + vec2(0, -300), vec4(1, 1.0f, 1.0f, 1.0f),
    //             2.0f
    //         );

    end_game_view();

    old_window_size = window_size;
}

void watch_callback(filewatch_update_t change, const char* virtual_path, void* udata)
{
    Game_State* game_state = (Game_State*)udata;

	const char* change_string = 0;
	switch (change)
	{
        case FILEWATCH_DIR_ADDED: change_string = "FILEWATCH_DIR_ADDED"; break;
        case FILEWATCH_DIR_REMOVED: change_string = "FILEWATCH_DIR_REMOVED"; break;
        case FILEWATCH_FILE_ADDED: change_string = "FILEWATCH_FILE_ADDED"; break;
        case FILEWATCH_FILE_REMOVED: change_string = "FILEWATCH_FILE_REMOVED"; break;
        case FILEWATCH_FILE_MODIFIED: change_string = "FILEWATCH_FILE_MODIFIED"; break;
	}

	printf("%s at %s.\n", change_string, virtual_path);

    if(str_equal_to(virtual_path, "/data/game.vars"))
    {
        char* path_at = (char*)virtual_path;
        ++path_at;
        get_game_variables(path_at, game_state);
    }
}