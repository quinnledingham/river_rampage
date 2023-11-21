#ifndef PARTICLES_H
#define PARTICLES_H

struct Particle
{
    v3 position;
    v3 speed;
    v4 color;
    r32 size;
    v3 direction;
    r32 weight;
    r32 life;
};

struct Particles
{
    Particle *data;
    u32 count;
    u32 max;
    Mesh mesh; // particle mesh
    
    u32 opengl_buffer;
    u32 opengl_life_buffer;
};

void add_particle(v3 position, v3 direction, r32 weight);
void draw_particles(Assets *assets, r32 seconds);
void update_particles(r32 frame_time_s);

#endif //PARTICLES_H