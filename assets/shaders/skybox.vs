#version 330 core
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texture_coords;

layout (std140) uniform Matrices
{
    mat4 projection;
    mat4 view;

    float near;
    float far;
    float window_width;
    float window_height;
};
uniform mat4 model; 

out vec3 uv;

void main(void) 
{ 
	uv = position;
	gl_Position = projection * view * model * vec4(position, 1.0f); 
}