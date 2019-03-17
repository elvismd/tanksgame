#ifdef VERTEX_SHADER

layout (location = 0) in vec3 position;

uniform mat4 model;
uniform mat4 vp;

void main()
{
	gl_Position = vp * model * vec4(position, 1.0f);
}

#endif

#ifdef FRAGMENT_SHADER

uniform vec4 color;

out vec4 frag_color;

void main()
{
	frag_color = color;
}

#endif