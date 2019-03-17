#ifdef VERTEX_SHADER

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoords;

out vec2 tex_coords;
out vec3 frag_pos;

uniform mat4 model;
uniform mat4 vp;

uniform int flip;

void main()
{
	frag_pos = vec3(model * vec4(position, 1.0));

	if(flip == 1)
	{
		tex_coords = vec2(texCoords.x * -1, 1.0 - texCoords.y);
	}
	else 
	{
		tex_coords = vec2(texCoords.x, 1.0 - texCoords.y);
	}

	gl_Position = vp * model * vec4(position, 1.0f);
}

#endif

#ifdef FRAGMENT_SHADER

in vec2 tex_coords;
in vec3 frag_pos;  

uniform vec4 color;
uniform sampler2D image;

uniform vec3 world_light_color;
uniform vec3 world_light_pos;
uniform float world_light_intensity;

out vec4 frag_color;
		
vec4 calc_light(vec3 pos, vec3 color, float intensity)
{
	float dist = length(pos - frag_pos);
	float attenuation = intensity / dist;
	return vec4(attenuation, attenuation, attenuation, 1) * vec4(color, 1);
}

vec4 calc_global_light()
{
	float dist = length(world_light_pos - frag_pos);
	float attenuation = world_light_intensity;
	return vec4(attenuation, attenuation, attenuation, 1) * vec4(world_light_color, 1);
}

void main()
{
	//frag_color = texture(image, tex_coords) * calc_global_light() * color;
	//frag_color = color;
	frag_color = texture(image, tex_coords) * color;
}

#endif