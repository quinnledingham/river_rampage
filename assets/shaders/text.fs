#version 330 core

in vec2 uv;
out vec4 FragColor;

uniform sampler2D tex0;
uniform vec4 text_color;

void main() 
{ 
	vec3 norm_text_color = vec3(text_color.x/255, text_color.y/255, text_color.z/255);
	float alpha = texture(tex0, uv).r * text_color.a;
	vec4 tex = vec4(1.0, 1.0, 1.0, alpha); 
	FragColor = vec4(norm_text_color, 1.0) * tex;
}