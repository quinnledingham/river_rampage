#version 420 core
#extension GL_NV_uniform_buffer_std430_layout : enable

struct Light_Source {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    vec4 color;
};

struct Light {
    float f[16];
};


in vec2 uv_coords;
in vec3 frag_normal;
in vec3 frag_position;

out vec4 FragColor;

uniform vec3 camera_pos;
uniform vec4 user_color;
uniform float time;

layout (std430) uniform Lights
{
    Light lights;
};

Light_Source get_light(float a[16]) {
    Light_Source l;
    l.position = vec3(a[0],  a[1],  a[2]);
    l.ambient  = vec3(a[3],  a[4],  a[5]);
    l.diffuse  = vec3(a[6],  a[7],  a[8]);
    l.specular = vec3(a[9],  a[10], a[11]);
    l.color    = vec4(a[12], a[13], a[14], a[15]);
    return l;
}

Light_Source light = get_light(lights.f);


//
// Noise
//

float random(float x) {
    return fract(sin(x) * 10000.);
}

float noise(vec2 p) {
    return random(p.x + p.y * 10000.);       
}

vec2 sw(vec2 p) { return vec2(floor(p.x), floor(p.y)); }
vec2 se(vec2 p) { return vec2(ceil(p.x),  floor(p.y)); }
vec2 nw(vec2 p) { return vec2(floor(p.x), ceil(p.y) ); }
vec2 ne(vec2 p) { return vec2(ceil(p.x),  ceil(p.y) ); }

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

//
// End of Noise
//

// distance of b from a
float distance(vec3 a, vec3 b)
{
	vec3 temp = a - b;
	return sqrt(temp.x * temp.x + temp.y * temp.y + temp.z * temp.z);
}

void main() {
	vec3 normal = -normalize(frag_normal);

	// ambient
	vec3 ambient = light.ambient * light.color.rgb;

	// diffuse
    vec3 lightDir = normalize(light.position - frag_position);
	float diff = max(dot(normal, lightDir), 0.0);
	vec3 diffuse = light.diffuse * (diff * light.color.rgb);

	// secular
	vec3 view_dir = normalize(camera_pos - frag_position);
	vec3 reflect_dir = reflect(-lightDir, normal);
	float spec = pow(max(dot(view_dir, reflect_dir), 0.0), 16);
	vec3 specular = light.specular * (spec * light.color.rgb);
	
	vec4 color = user_color;
	color.xyz -= 0.2;
	
	if (distance(frag_position, camera_pos) < 100)
	{
		float n = nestedNoise(uv_coords * 6.);
		color = vec4(mix(user_color.xyz, user_color.xyz - 0.5, n), 0.1);
	}

	vec3 result = (ambient + diffuse + specular) * color.xyz;
	FragColor = vec4(result, user_color.w);
}