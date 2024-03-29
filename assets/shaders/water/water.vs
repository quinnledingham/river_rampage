#version 420 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texture_coords;

out vec3 v_position;
out vec2 tex_coords;

void main()
{
	//gl_Position = vec4(position, 1.0);
	v_position = position;
	tex_coords = texture_coords;
}