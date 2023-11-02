#ifndef GAME_H
#define GAME_H

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

struct Boat3D
{
    v3 coords;
    v3 draw_coords; // boat coords with waves applied

    v3 direction = {1, 0, 0};
};

enum Game_Modes
{
    MAIN_MENU,
    IN_GAME_2D,
    IN_GAME_3D,

    GAME_MODES_COUNT
};

const global Pair game_modes[3] = {
        { MAIN_MENU, "MAIN_MENU" },
        { IN_GAME_2D, "IN_GAME_2D" },
        { IN_GAME_3D, "IN_GAME_3D" },
    };

enum Camera_Modes
{
    BOAT_CAMERA,
    FREE_CAMERA,

    CAMERA_MODES_COUNT,
};

const global Pair camera_modes[2] = {
        { BOAT_CAMERA, "BOAT_CAMERA" },
        { FREE_CAMERA, "FREE_CAMERA" },
    };


struct Game_Data
{
    b8 paused;
    b8 wire_frame;
    b8 show_fps;
    b8 show_console;
    Console console;
    Onscreen_Notifications onscreen_notifications;

    // 3D
    Light_Source light;
    Camera camera;
    Mesh water;
    Mesh cube;
    Wave waves[5];
    
    u32 matrices_ubo; // uniform buffer object
    u32 wave_ubo; 
    
    Boat3D boat3D;
    u32 camera_mode;

    Mesh skybox_cube;
    Cubemap skybox;
    
    // 2D
    Boat boat;
    r32 water_force;
    
    u32 game_mode;
    s32 active;
};

b8 update(void *application);
void* init_data(Assets *assets);

#endif //GAME_H
