#include "enemy_ai.h"
#include "game.h"

void handle_enemy_movement(Entity* entity, Entity_Manager* entity_manager, Game_Input* input)
{
    Enemy* enemy = (Enemy*)entity->derived_ptr;
    auto controller = &input->controllers[0];

    local_persist u32 projectile_index = 0;
    local_persist vec2 dir;
    local_persist vec2 cannon_dir;

    // Tank body movement
    vec2 velocity = entity->velocity;
    velocity.x = 0.0f;
    velocity.y = 0.0f;

    Entity* target = enemy->target;

    velocity = target->position - entity->position;

    // TODO: velocity of the tank body

    if(glm::length(velocity) > 0.0f)
    {
        dir = lerp(dir, (entity->position + velocity) - entity->position, input->delta_time * 10);
        float rot = atan2(dir.y, dir.x);
        entity->rotation = glm::degrees(rot) + 90.0f;
    }

    entity->velocity = velocity;

    // Cannon rotation
    vec2 cannon_velocity = vec2(0.0f);

    // TODO: velocity dir of the cannon

    if(glm::length(cannon_velocity) > 0.0f)
    {
        cannon_dir = lerp(cannon_dir, (entity->position + cannon_velocity) - entity->position, input->delta_time * 10);
        float rot = atan2(cannon_dir.y, cannon_dir.x);

        enemy->cannon_entity->rotation = glm::degrees(rot) + 90.0f;
    }
    enemy->cannon_entity->position = entity->position + vec2(0.0f, 0.5);

    // TODO: Projectiles
    // Projectile shooting
    // if(controller->keys[GLFW_KEY_SPACE].was_down)
    // {
    //     Projectile* projectile = enemy->projectiles[projectile_index++];
    //     Entity* projectile_entity = projectile->entity;
        
    //     projectile->dir = cannon_dir;

    //     projectile_entity->flags = DRAWABLE | VISIBLE;
    //     projectile_entity->position = entity->position;
    //     projectile_entity->rotation = enemy->cannon_entity->rotation;

    //     if(projectile_index >= 10) projectile_index = 0;
    // }

    // for(int i = 0; i < 10; i++)
    // {
    //     Projectile* projectile = enemy->projectiles[i];
    //     Entity* projectile_entity = projectile->entity;

    //     if(projectile_entity->flags & VISIBLE)
    //     {
    //         vec2 projectile_velocity = projectile->dir;
    //         projectile_velocity.x *= input->delta_time * PROJECTILE_SPEED;
    //         projectile_velocity.y *= input->delta_time * PROJECTILE_SPEED;

    //         projectile_entity->position += projectile_velocity;
    //     }
    // }
}