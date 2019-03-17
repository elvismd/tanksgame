#pragma once

#include "agnostic.h"

struct Game_Button_State
{
	int half_transition_count;
	b32 is_down;
	b32 was_down;
};

struct Game_Controller_Input 
{
	b32 is_connected;
	b32 is_analog;

	b32 mouse_is_moving;
	vec2 mouse_position;

	Game_Button_State mouse_left;
	Game_Button_State mouse_right;
	
	Game_Button_State keys[512];
};

struct Game_Input 
{
	r32 delta_time;
	Game_Controller_Input controllers[5];
};

