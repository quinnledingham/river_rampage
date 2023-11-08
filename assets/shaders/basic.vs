#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texture_coords;

out vec2 uv;

layout (std140) uniform Matrices
{
	mat4 projection;
	mat4 view;
};
uniform mat4 model; 

void main(void) 
{ 
	gl_Position = projection * view * model * vec4(position, 1.0f); 
	uv = texture_coords;
}