#pragma once

#include "agnostic.h"
#include "glad/glad.h"

#define  CHECK_GL(...)   check_gl_error(__FILE__, __LINE__);
int check_gl_error(char * file, int line);

void init_renderer(void * data);