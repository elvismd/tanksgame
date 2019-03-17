#include "opengl.h"

#include "logger.h"

#define  CHECK_GL(...)   check_gl_error(__FILE__, __LINE__);
int check_gl_error(char * file, int line)
{
	GLuint err = glGetError();
	if (err > 0)
	{
		log_warning("GL Error - file:%s line: %d error: %d", file, line, err);

		switch(err)
		{
			case GL_INVALID_ENUM: log_warning("GL_INVALID_ENUM: Given when an enumeration parameter is not a legal enumeration for that function. This is given only for local problems; if the spec allows the enumeration in certain circumstances, where other parameters or state dictate those circumstances, then GL_INVALID_OPERATION is the result instead."); break;
			case GL_INVALID_VALUE: log_warning("GL_INVALID_VALUE: Given when a value parameter is not a legal value for that function. This is only given for local problems; if the spec allows the value in certain circumstances, where other parameters or state dictate those circumstances, then GL_INVALID_OPERATION is the result instead."); break;
			case GL_INVALID_OPERATION: log_warning("GL_INVALID_OPERATION: Given when the set of state for a command is not legal for the parameters given to that command. It is also given for commands where combinations of parameters define what the legal parameters are."); break;
			case GL_STACK_OVERFLOW: log_warning("GL_STACK_OVERFLOW: Given when a stack pushing operation cannot be done because it would overflow the limit of that stack's size."); break;
			case GL_STACK_UNDERFLOW: log_warning("GL_STACK_UNDERFLOW: Given when a stack popping operation cannot be done because the stack is already at its lowest point."); break;
			case GL_OUT_OF_MEMORY: log_warning("GL_OUT_OF_MEMORY: Given when performing an operation that can allocate memory, and the memory cannot be allocated. The results of OpenGL functions that return this error are undefined; it is allowable for partial operations to happen."); break;
			case GL_INVALID_FRAMEBUFFER_OPERATION: log_warning("GL_INVALID_FRAMEBUFFER_OPERATION: Given when doing anything that would attempt to read from or write/render to a framebuffer that is not complete."); break;
			case GL_CONTEXT_LOST: log_warning("GL_CONTEXT_LOST: Given if the OpenGL context has been lost, due to a graphics card reset."); break; 
		}

	}
	return err;
}

void init_renderer(void* data)
{
	if (!gladLoadGLLoader((GLADloadproc)data))
	{
		printf("Failed to init GLAD. \n");
		Assert(false);
	}

	// TODO : Print those
	char* opengl_vendor = (char*)glGetString(GL_VENDOR);
	char* opengl_renderer = (char*)glGetString(GL_RENDERER);
	char* opengl_version = (char*)glGetString(GL_VERSION);
	char* shading_language_version = (char*)glGetString(GL_SHADING_LANGUAGE_VERSION);

    log_info("GL Vendor %s", opengl_vendor);
    log_info("GL Renderer %s", opengl_renderer);
    log_info("GL Version %s", opengl_version);
    log_info("GL Shading Language Version %s", shading_language_version);

	// GLint n, i;
	// glGetIntegerv(GL_NUM_EXTENSIONS, &n);
	// for (i = 0; i < n; i++)
	// {
	// 	char* extension = (char*)glGetStringi(GL_EXTENSIONS, i);
	// }
}