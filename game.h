#ifndef GAME_H
#define GAME_H

struct Light_Source
{
    v3 position;
    v4 color;
};

struct Camera
{
    v3 position;
    v3 target;
    v3 up;
    r32 fov;
    r32 yaw;
    r32 pitch;
};
function inline m4x4 get_view(Camera camera) 
{ 
    return look_at(camera.position, camera.position + camera.target, camera.up); 
}

struct Wave
{
    v2 direction;
    f32 wave_length;
    f32 steepness;
    
    //f32 k;
    //f32 c;
    //v2 d;
    //f32 a;
};

function Wave
get_wave(v2 direction, f32 wave_length, f32 steepness)
{
    Wave wave = {};
    wave.direction = direction;
    wave.wave_length = wave_length;
    wave.steepness = steepness;
    
    //wave.k = 2.0f * PI / wave_length;
    //wave.c = sqrt(9.8f / wave.k);
    //wave.d = normalized(wave.direction);
    //wave.a = wave.steepness / wave.k;
    return wave;
}


struct Boat
{
    v2 coords;
    
    r32 mass;
    r32 engine_force;
    r32 rudder_force;
    r32 water_line_length;
    
    r32 speed;
    r32 maximum_speed;
    r32 rotation_speed;
    v2 velocity; // the direciton and mag of movement
    
    v2 direction; // the way the ship is pointing 
    r32 acceleration_magnitude; // always accelerates in the direction of the boat
    f32 water_acceleration_magnitude;
    
};

enum Game_Modes
{
    MAIN_MENU,
    IN_GAME_2D,
    IN_GAME_3D,
};

struct Game_Data
{
    // 3D
    Light_Source light;
    Camera camera;
    Mesh water;
    Mesh cube;
    Wave waves[5];
    
    u32 uniform_buffer_object;
    u32 wave_ubo; // uniform buffer object
    
    // 2D
    Boat boat;
    r32 water_force;
    
    u32 game_mode;
    b8 paused;
    s32 active;
    b8 wire_frame;
    
    Model tree;
};

#endif //GAME_H
