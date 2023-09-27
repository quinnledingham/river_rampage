#version 410 core

in vec3 frag_normal;
in vec3 frag_tangent;
in vec3 frag_bitangent;
in vec3 frag_position;
in vec2 uv;

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 cameraPos;
uniform vec4 objectColor;
uniform sampler2D normal_map;

out vec4 FragColor;

void main()
{
	mat3 TBN = transpose(mat3(frag_tangent, frag_bitangent, frag_normal));

	//vec3 normal = texture(normal_map, uv).rgb;
	//normal = (normal) * 2.0 - 1.0;
	//normal = normalize(normal);

	vec3 normal = frag_normal;

	//vec4 tex = texture(normal_map, uv);

	// ambient
	float ambientStrength = 0.1f;
	vec3 ambient = ambientStrength * lightColor;

	// diffuse
    vec3 lightDir = normalize(lightPos - frag_position);
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = diff * lightColor;

	// secular
	float specular_strength = 0.5;
	vec3 view_dir = normalize(cameraPos - frag_position);
	vec3 reflect_dir = reflect(-lightDir, normal);
	float spec = pow(max(dot(view_dir, reflect_dir), 0.0), 32);
	vec3 specular = specular_strength * spec * lightColor;

	vec3 result = (ambient + diffuse + specular) * objectColor.xyz;
	FragColor = vec4(result, objectColor.w);
}