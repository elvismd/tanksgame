#include "opengl.h"

// glad already includes opengl
#define GLFW_INCLUDE_NONE
#include <glfw/glfw3.h>

#include <stdlib.h>

#include "draw.h"

#include "game.h"

void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void cursor_position_callback(GLFWwindow* window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

int main(int argc, char** argv)
{
    Logger* logger = (Logger*)malloc(sizeof(Logger));
	set_logger(logger);

    if (!glfwInit()) return 0;

    glfwSetErrorCallback(error_callback);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);

    GLFWwindow* window = glfwCreateWindow(800, 600, "Window", NULL, NULL);
    if (!window)
    {
        // TODO: Log
        return 0;
    }
    
    glfwMakeContextCurrent(window);
    init_renderer(glfwGetProcAddress);
    glfwSwapInterval(1);

    Game_Memory game_memory = {};
    game_memory.permanent_storage_size = Megabytes(500);
    game_memory.transient_storage_size = Megabytes(300);
    
    u64 TotalSize = game_memory.permanent_storage_size + game_memory.transient_storage_size;
    game_memory.permanent_storage = malloc(TotalSize);

    game_memory.transient_storage = ((u8 *)game_memory.permanent_storage + 
                                    game_memory.permanent_storage_size);

    Game_Input game_input = {};
    Game_Input* game_input_ptr = &game_input;

    r64 started_time = glfwGetTime();
    r64 started_time_to_update_fps = 0.0;

    glfwSetKeyCallback(window, key_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    glfwSetWindowUserPointer(window, game_input_ptr);

    while (!glfwWindowShouldClose(window))
    {
        int window_width, window_height;
        glfwGetWindowSize(window, &window_width, &window_height);

        game_update_and_render(&game_memory, &game_input, vec2(window_width, window_height));
        
        glfwSwapBuffers(window);

        for(int i = 0; i < array_count(game_input.controllers); i++)
        {
            auto controller = &game_input.controllers[0];
            controller->mouse_is_moving = false;

            controller->mouse_left.was_down = false;
            controller->mouse_right.was_down = false;

            for(int j = 0; j < array_count(controller->keys); j++)
            {
                auto button_state = &controller->keys[j];
                button_state->was_down = false;
            }
        }

        glfwPollEvents();

        r64 end_time = glfwGetTime();
        r64 ms_per_frame = (end_time - started_time);
        r32 delta_time = (ms_per_frame * 1000.0f);
        r32 fps = 1000.0f / delta_time;

        game_input.delta_time = ms_per_frame;

        if((end_time - started_time_to_update_fps) > 1)
        {
            char title[1024];
            snprintf(title, 1024, "Ms per frame: %.1f - FPS: %.1f", delta_time, fps);
            glfwSetWindowTitle(window, title);

            started_time_to_update_fps = glfwGetTime();
        }

        started_time = glfwGetTime();
    }

    glfwTerminate();   
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    Game_Input* game_input = (Game_Input*)glfwGetWindowUserPointer(window);

    for(int i = 0; i < array_count(game_input->controllers); i++)
    {
        auto controller = &game_input->controllers[0];
        if(key < array_count(controller->keys))
        {
            auto button_state = &controller->keys[key];

            if(action == GLFW_PRESS)
            {
                if(button_state->is_down) button_state->was_down = true;
                button_state->is_down = true;
            }
            else if(action == GLFW_REPEAT) button_state->is_down = true;
            else if(action == GLFW_RELEASE) button_state->is_down = false;
        }
    }
}

void cursor_position_callback(GLFWwindow* window, double xpos, double ypos)
{
     Game_Input* game_input = (Game_Input*)glfwGetWindowUserPointer(window);

     for(int i = 0; i < array_count(game_input->controllers); i++)
     {
         auto controller = &game_input->controllers[i];

         controller->mouse_is_moving = true;
         controller->mouse_position = vec2(xpos, ypos);
     }
}

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    Game_Input* game_input = (Game_Input*)glfwGetWindowUserPointer(window);

    for(int i = 0; i < array_count(game_input->controllers); i++)
    {
        auto controller = &game_input->controllers[i];
 
        auto button_state = &controller->mouse_left;

        if(button == GLFW_MOUSE_BUTTON_RIGHT)
            button_state = &controller->mouse_right;
        else if(button == GLFW_MOUSE_BUTTON_LEFT)
            button_state = &controller->mouse_left;
        else 
            break;

        if(action == GLFW_PRESS)
        {
            button_state->was_down = true;
            button_state->is_down = true;
        }
        else if(action == GLFW_REPEAT) button_state->is_down = true;
        else if(action == GLFW_RELEASE) button_state->is_down = false;

    }
}