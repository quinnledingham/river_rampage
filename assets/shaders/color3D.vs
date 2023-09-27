#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texture_coords;

uniform mat4 model;
uniform mat4 projection;
uniform mat4 view;

void main(void)
{
	gl_Position = projection * view * model * vec4(position, 1.0f);
}