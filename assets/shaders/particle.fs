#version 330 core

in float alpha;
in vec2 uv;

uniform vec4 user_color;
out vec4 FragColor;

void main()
{
	FragColor  = vec4(user_color.x/255, user_color.y/255, user_color.z/255, alpha);
}