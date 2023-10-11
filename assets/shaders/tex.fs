#version 330 core

uniform sampler2D tex0;

in vec2 uv;
out vec4 FragColor;

void main()
{
	vec4 tex = texture(tex0, uv);

	FragColor = vec4(tex.rgb, 1.0);
}