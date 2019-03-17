#include "entity_manager.h"

Entity* get_next_entity(void* derived_ptr, u32 type, Entity_Manager* entity_manager)
{
    Entity new_entity = {};

    Entity* result = &(array_add(entity_manager->entities, new_entity));
    result->scale = vec2(1.0f, 1.0f);
    result->derived_ptr = derived_ptr;
    result->derived_type = type;
    result->color = vec4(1.0f);

    result->flags = DRAWABLE | VISIBLE;

    return result;
}

Entity* get_entity_by_name(const char* name, Entity_Manager* entity_manager)
{
    for(int i = 0; i < entity_manager->entities.length; i++)
    {
        auto entity = &(array_item(entity_manager->entities, i));

        if(str_equal_to(entity->name, name))
            return entity;
    }

    return NULL;
}