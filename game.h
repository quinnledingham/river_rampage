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
    r32 water_acceleration_magnitude;
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
    
    // 2D
    Boat boat;
    r32 water_force;
    
    u32 game_mode;
    b8 paused;
    s32 active;
    b8 wire_frame;
    
    Mesh tree;
};

#endif //GAME_H
