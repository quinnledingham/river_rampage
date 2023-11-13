#ifndef APPLICATION_H
#define APPLICATION_H

struct Window
{
    //SDL_Window *sdl; 
    v2s dim;
    r32 aspect_ratio; // update with dim change
    
    b32 *update_matrices;
};

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
            
            Button reload_shaders;
            Button toggle_camera_mode;
            Button toggle_console;
        };
        Button buttons[12];
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

enum
{
    INPUT_MODE_GAME,
    INPUT_MODE_KEYBOARD,
};

// if I want to have a game controller seperate from the application layer then I will need to
// pass all of the inputs from application to game
struct Input
{
    Controller controllers[5];
    u32 num_of_controllers;
    Controller *active_controller;
    
    char buffer[10];
    u32 mode; // game or keyboard

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

#endif //APPLICATION_H
