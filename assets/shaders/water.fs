#version 410 core

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 cameraPos;
uniform vec4 objectColor;

uniform sampler2D normal_map;
uniform float time;

in vec3 frag_normal;
in vec3 frag_tangent;
in vec3 frag_bitangent;
in vec3 frag_position;
in vec2 uv_coords;

out vec4 FragColor;

float random(float x) {
 
    return fract(sin(x) * 10000.);
          
}

float noise(vec2 p) {

    return random(p.x + p.y * 10000.);
            
}

vec2 sw(vec2 p) { return vec2(floor(p.x), floor(p.y)); }
vec2 se(vec2 p) { return vec2(ceil(p.x), floor(p.y)); }
vec2 nw(vec2 p) { return vec2(floor(p.x), ceil(p.y)); }
vec2 ne(vec2 p) { return vec2(ceil(p.x), ceil(p.y)); }

float smoothNoise(vec2 p) {

    vec2 interp = smoothstep(0., 1., fract(p));
    float s = mix(noise(sw(p)), noise(se(p)), interp.x);
    float n = mix(noise(nw(p)), noise(ne(p)), interp.x);
    return mix(s, n, interp.y);
}

float fractalNoise(vec2 p) {

    float x = 0.;
    x += smoothNoise(p      );
    x += smoothNoise(p * 2. ) / 2.;
    x += smoothNoise(p * 4. ) / 4.;
    x += smoothNoise(p * 8. ) / 8.;
    x += smoothNoise(p * 16.) / 16.;
    x /= 1. + 1./2. + 1./4. + 1./8. + 1./16.;
    return x;
            
}

float movingNoise(vec2 p) {
 
    float x = fractalNoise(p + time / 5.0);
    float y = fractalNoise(p - time / 5.0);
    return fractalNoise(p + vec2(x, y));   
    
}

// call this for water noise function
float nestedNoise(vec2 p) {
    
    float x = movingNoise(p);
    float y = movingNoise(p + 100.);
    return movingNoise(p + vec2(x, y));
    
}

// distance of b from a
float distance(vec3 a, vec3 b)
{
	vec3 temp = a - b;
	return sqrt(temp.x * temp.x + temp.y * temp.y + temp.z * temp.z);
}

void main()
{
	vec4 tex = texture(normal_map, uv_coords);
	mat3 TBN = transpose(mat3(frag_tangent, frag_bitangent, frag_normal));

	//vec3 normal = texture(normal_map, uv).rgb;
	vec3 normal = frag_normal;

	// ambient
	float ambientStrength = 0.3f;
	vec3 ambient = ambientStrength * lightColor;

	// diffuse
    vec3 lightDir = normalize(lightPos - frag_position);
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = diff * lightColor;

	// secular
	float specular_strength = 0.5;
	vec3 view_dir = normalize(cameraPos - frag_position);
	vec3 reflect_dir = reflect(-lightDir, normal);
	float spec = pow(max(dot(view_dir, reflect_dir), 0.0), 16);
	vec3 specular = specular_strength * spec * lightColor;

	vec4 color = objectColor;
	color.xyz -= 0.2;
	
	if (distance(frag_position, cameraPos) < 20)
	{
		float n = nestedNoise(uv_coords * 6.);
		color = vec4(mix(objectColor.xyz, objectColor.xyz - 0.5, n), 1);
	}

	vec3 result = (ambient + diffuse + specular) * color.xyz;
	FragColor = vec4(result, objectColor.w);
}