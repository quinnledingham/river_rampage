#version 330 core

in vec2 uv; 

out vec4 FragColor;

uniform vec4 user_color;

void main() 
{ 
	FragColor  = vec4(user_color.x/255, user_color.y/255, user_color.z/255, user_color.w);
}