#version 420 core
#extension GL_NV_uniform_buffer_std430_layout : enable

in vec2 uv_coords;
in vec3 frag_normal;
in vec3 frag_position;

out vec4 FragColor;

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

uniform vec3 camera_pos;
uniform vec4 user_color;
uniform float time;

uniform sampler2D depth_buffer;
uniform sampler2D color_buffer;
uniform sampler2D foam;
uniform sampler2D normal_map;

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

layout (std140) uniform Matrices
{
    mat4 projection;
    mat4 view;

    float near;
    float far;
    float window_width;
    float window_height;
};

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
  
float linearize_depth(float depth) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return ((2.0 * near * far) / (far + near - z * (far - near))) / far;    
}

vec2 get_window_coords(vec2 coords) {
    vec2 result = coords;
    result.x /= window_width;
    result.y /= window_height;
    return result;
}

void main() {
    vec2 window_coords = get_window_coords(gl_FragCoord.xy);
	vec3 normal = -normalize(frag_normal);

    vec3 normal_m = texture(normal_map, uv_coords * 50).xyz;
    // have to convert it from 0 to 1 range to -1 to 1 range
    normal_m = normalize(vec3(normal_m.r * 2.0 - 1.0, normal_m.b, normal_m.g * 2.0 - 1.0));
    normal += (normal_m * 0.2);
    normal = normalize(normal);

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
		color = vec4(mix(user_color.xyz, user_color.xyz - 0.5, n), color.w);
	}

    vec3 water_color = (ambient + diffuse + specular) * color.xyz;

    float refraction = 0.01;
    vec4 back = texture(color_buffer, window_coords + (refraction * normal.xz));

    vec4 result = vec4(mix(back.xyz, water_color, color.w), 1.0);

    //
    // Very helpful tutorial
    // https://fire-face.com/personal/water/
    //

    float channel_a = texture(foam, uv_coords * 70).r;
    float channel_b = texture(foam, uv_coords * 70).b;

    float mask = (channel_a + channel_b) * 0.95;
    mask = pow(mask, 2);
    mask = clamp(mask, 0.0, 1.0);

    float depth = linearize_depth(gl_FragCoord.z);

    float previous_depth = texture2D(depth_buffer, window_coords).x;
    previous_depth = linearize_depth(previous_depth);
    
    float depth_diff = previous_depth - depth;
    float falloff_distance = 0.0015;
    float leading_edge_fallout = 0.2;
    float edge_falloff_bias = 0.5;
    vec4 edge_falloff_color = vec4(1, 1, 1, 1);

    if (depth_diff < falloff_distance * leading_edge_fallout) {
        float leading = depth_diff / (falloff_distance * leading_edge_fallout);
        result.a *= leading;
        mask *= leading;
    }

    float falloff = 1.0 - (depth_diff / falloff_distance) + edge_falloff_bias;
    float a = falloff - mask;
    vec3 edge = edge_falloff_color.rgb * falloff * edge_falloff_color.a;
    result.rgb = mix(result.rgb, vec3(1), clamp(a, 0.0, 0.5));
    //result.rgb += clamp(edge - vec3(mask), 0.0, 1.0);

    //result = texture(color_buffer, uv_coords);
	FragColor = result;
}