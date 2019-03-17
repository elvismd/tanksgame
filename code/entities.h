#pragma once

#include "agnostic.h"
#include "array.h"
#include "entity_manager.h"

#define PLAYER_ENTITY 0
#define CANNON_ENTITY 1
#define TILEMAP_ENTITY 2
#define PROJECTILE_ENTITY 3
#define ENEMY_ENTITY 4

#define PROJECTILE_SPEED 7
struct Projectile 
{
	USING_ENTITY(PROJECTILE_ENTITY)

	vec2 dir;
};

struct Player
{
	USING_ENTITY(PLAYER_ENTITY)

	r32 movement_speed;

	Entity* cannon_entity;
	Projectile* projectiles[10];
};

struct Cannon
{
	USING_ENTITY(CANNON_ENTITY)
};

struct Enemy
{
	USING_ENTITY(ENEMY_ENTITY)

	Entity* target;
	Entity* cannon_entity;
	r32 speed;
};

#define TILEMAP_TILE_SCALE 10
#define TILEMAP_SIZE 150
struct Tile_Map
{
	USING_ENTITY(TILEMAP_ENTITY)

	vec2 tile_positions[TILEMAP_SIZE][TILEMAP_SIZE];
    vec4 tile_rects[TILEMAP_SIZE][TILEMAP_SIZE];
    u64 tile_vertices_indexes[TILEMAP_SIZE][TILEMAP_SIZE];
    s32 tile_types[TILEMAP_SIZE][TILEMAP_SIZE];
};

