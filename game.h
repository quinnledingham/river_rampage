#ifndef GAME_H
#define GAME_H

enum
{
    FORWARD,       // 0
    BACKWARD,      // 1
    LEFT,          // 2
    RIGHT,         // 3

    DIRECTIONS_2D  // 4 directions needed for 2D
};

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

struct Boat_Coords
{
    union
    {
        struct
        {
            v3 forward;
            v3 backward;
            v3 left;
            v3 right;
            v3 center;
        };
        v3 E[5];
    };
};

inline Boat_Coords
Create_Boat_Coords(v3 c, v3 f, v3 b, v3 l, v3 r) {
    Boat_Coords result = {};

    result.center   = c;
    result.forward  = f;
    result.backward = b;
    result.left     = l;
    result.right    = r;

    return result;
}

struct Boat3D
{
    v3 direction  = {1, 0, 0};
    v3 up         = {0, 1, 0}; 
    v3 side       = {0, 0, 1};
    v3 coords     = { 0, -1.5f, 0 };
    quat rotation = {0, 0, 0, 1};

    r32 speed;
    r32 maximum_speed = 10.0f;
    v3  velocity; // the direction and mag of movement - capped by maximum_speed (magnitude(velocity) has to be < maximum_speed)
    r32 acceleration_magnitude = 5.0f; // always accelerates in the direction of the boat
    r32 drag_magnitude = 0.5f;

    r32 draw_delta;

    Boat_Coords base; // coords without waves applied
    Boat_Coords wave; // coords with waves applied

    f32 lengths[4]; // how far from the center to edge of boat
    v3 debug_wave_coords[4];
    
    Easy_Textboxs easy; // for editing debug values

    v3 draw_coords_history[500];
    u32 newest_draw_coord_index;
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
    EDIT_CAMERA,

    CAMERA_MODES_COUNT,
};

const global Pair camera_modes[CAMERA_MODES_COUNT] = {
        { BOAT_CAMERA, "BOAT_CAMERA" },
        { FREE_CAMERA, "FREE_CAMERA" },
        { EDIT_CAMERA, "EDIT_CAMERA" },
    };

struct Uniform_Buffer_Objects {
    // What the structs are called in the shaders
    const char *tags[3] = {
        "Matrices",
        "Wav",
        "Lights",
    };

    union {
        struct {
            u32 matrices;
            u32 waves;
            u32 lights;
        };
        u32 E[3];
    };
};

struct Game // What both 2D and 3D uses
{
    b8 paused;
    r32 run_time_s = 0.0f;
    Uniform_Buffer_Objects ubos;

    // Menus
    u32 mode = IN_GAME_3D;
    s32 active;

    f32 test_pixel_height = 36.0f;
};

struct Dev_Tools
{
    b8 wire_frame;
    b8 show_fps;
    b8 show_console;
    Console console;
    Onscreen_Notifications onscreen_notifications;

    b8 show_camera_menu;
    Easy_Textboxs camera_menu;
};

struct Game_2D {
    Boat boat;
    r32 water_force;
};

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

// what is passed to the shaders
struct Waves_Data {
    Wave waves[20];
    u32 num_of_waves;
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

struct Game_3D
{
    Light light;

    Camera camera;
    u32 camera_mode;

    Mesh triangle_mesh;
    
    Mesh water;
    Waves_Data waves_data;

    Boat3D boat3D;
    
    Mesh skybox_cube;
    Cubemap skybox;
};

struct Game_Data
{
    Game game;
    Dev_Tools dev_tools;
    Game_2D game_2D;
    Game_3D game_3D;
};

function s32 draw_main_menu(Game *game, Matrices *matrices, Assets *assets, Input *input, v2s window_dim);
function s32 draw_pause_menu(Assets *assets, v2 window_dim, b32 select, s32 active);

#endif //GAME_H
