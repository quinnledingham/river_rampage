#version 330 core 
layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec2 texture_coords;
layout (location = 3) in mat4 model; // takes up 3 - 6
layout (location = 7) in float life;

out float alpha;
out vec2 uv; 

struct Wave {
    vec2 direction;
    float wave_length;
    float steepness;

    //float k;
    //float c;
    //vec2  d;
    //float a;
};

layout (std140) uniform Matrices
{
    mat4 projection;
    mat4 view;
};

layout (std140) uniform Wav
{
    Wave waves[5];
};

uniform float time;

#define PI 3.14159265359f

vec3 apply_wave(vec3 pos, Wave wave, float time)
{
    float k = 2 * PI / wave.wave_length;
    float c = sqrt(9.8 / k);
    vec2 d = normalize(wave.direction);
    float f = k * (dot(d, pos.xz) - c * time);
    float a = wave.steepness / k;

    return vec3(d.x * (a * cos(f)),
                a * sin(f),
                d.y * (a * cos(f)));
}

void main(void) 
{ 
    vec3 pos = (model * vec4(position, 1.0f)).xyz;
    vec3 const_pos = pos;
    for (int i = 0; i < 5; i++) 
        pos += apply_wave(const_pos, waves[i], time / 4.0);

    gl_Position = projection * view * vec4(pos, 1.0);
    alpha = life;
    uv = texture_coords;
}