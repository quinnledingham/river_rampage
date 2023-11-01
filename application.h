#ifndef APPLICATION_H
#define APPLICATION_H

struct Window
{
    //SDL_Window *sdl; 
    v2s dim;
    r32 aspect_ratio; // update with dim change
    
    b32 *update_matrices;
};

struct Matrices // for rendering
{
    b32 update;

    m4x4 perspective_matrix;
    m4x4 orthographic_matrix;
    m4x4 view_matrix;
};

// functions to set matrices in uniform buffer
void orthographic(u32 ubo, Matrices *matrices);
void perspective(u32 ubo, Matrices *matrices);

u32 init_uniform_buffer_object(u32 block_size, u32 block_index);
void platform_set_uniform_block_binding(u32 shader_handle, const char *tag, u32 index);
void platform_set_uniform_buffer_data(u32 ubo, u32 size, void *data);

void platform_uniform_m4x4(u32 shader_handle, const char *tag, m4x4 *m);
void platform_uniform_f32(u32 shader_handle, const char *tag, f32 f);
void platform_uniform_v3(u32 shader_handle, const char *tag, v3 v);
void platform_uniform_v4(u32 shader_handle, const char *tag, v4 v);
void platform_set_texture(Bitmap *bitmap);
void platform_set_texture_cube_map(Cubemap *cubemap, u32 shader);

void platform_blend_function(u32 source_factor, u32 destination_factor);

enum
{
    PLATFORM_POLYGON_MODE_POINT,
    PLATFORM_POLYGON_MODE_LINE,
    PLATFORM_POLYGON_MODE_FILL,

    PLATFORM_CAPABILITY_DEPTH_TEST,
    PLATFORM_CAPABILITY_CULL_FACE,
};

void platform_set_polygon_mode(u32 mode);
void platform_set_capability(u32 capability, b32 state);
void platform_set_depth_mask(b32 state);

struct Time
{
    u64 run_time_ms;
    r32 run_time_s;
    u64 frame_time_ms;
    r32 frame_time_s;
    r32 frames_per_s;
};

struct Button
{
    s32 id;
    
    s32 ids[3];
    u32 num_of_ids;
    
    b32 current_state; 
    b32 previous_state;
};

inline void set(Button *button, s32 id) 
{
    if (button->num_of_ids > 2)
        error("set() too many ids trying to be assigned to button");
    
    button->ids[button->num_of_ids++] = id;
    button->id = id; 
}

inline b32 is_down(Button button) 
{ 
    if (button.current_state) return true; return false; 
}

inline b32 on_down(Button button)
{
    if (button.current_state && button.current_state != button.previous_state) return true;
    return false;
}

struct Controller
{
    v2s mouse;
    union
    {
        struct
        {
            Button forward;
            Button backward;
            Button left;
            Button right;
            Button up;
            Button down;
            Button select;
            Button pause;
            
            Button wire_frame;
            Button reload_shaders;
            Button toggle_camera_mode;
        };
        Button buttons[11];
    };
};

struct Flag
{
    b8 state;
    b8 updated;
    
    void toggle() { state = !state; updated = true; }
    void set(b8 new_state) { state = new_state; updated = true; }
    b8 get() {  updated = false; return state; }
    b8 changed() { b8 up = updated; updated = false; return up; }
};

struct Input
{
    Controller controllers[5];
    u32 num_of_controllers;
    Controller *active_controller;
    
    Flag relative_mouse_mode;
    
    //SDL_Joystick *joysticks[4];
    //SDL_GameController *game_controllers[4];
    //u32 num_of_joysticks;
};

struct Application
{
    Window window;
    Matrices matrices;
    Time time;
    Input input;
    Assets assets;
    Audio_Player player;
    
    void *data;
};

void *platform_malloc(u32 size);
void platform_free(void *ptr);

#endif //APPLICATION_H
