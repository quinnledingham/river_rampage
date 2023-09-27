#ifndef APPLICATION_H
#define APPLICATION_H

struct Window
{
    SDL_Window *sdl;
    v2s dim;
    
    s32 gl_clear_flags;
};


function void swap_window(Window *window) 
{ 
    SDL_GL_SwapWindow(window->sdl);
}

struct Time
{
    u32 run_time_ms;
    r32 run_time_s;
    u32 frame_time_ms;
    r32 frame_time_s;
    r32 frames_per_s;
};

function void
update_time(Time *time)
{
    u32 last_run_time_ms = time->run_time_ms;
    
    time->run_time_ms = SDL_GetTicks();
    time->run_time_s = (f32)time->run_time_ms / 1000.0f;
    time->frame_time_ms = time->run_time_ms - last_run_time_ms;
    time->frame_time_s = (f32)time->frame_time_ms / 1000.0f;
    
    // get fps
    time->frames_per_s = 1000.0f;
    if (time->frame_time_s > 0.0f) time->frames_per_s = 1.0f / time->frame_time_s;
}

struct Button
{
    s32 id;
    
    s32 ids[3];
    u32 num_of_ids;
    
    b32 current_state; 
    b32 previous_state;
};

function void
set(Button *button, s32 id) 
{
    if (button->num_of_ids > 2)
        error("set() too many ids trying to be assigned to button");
    
    button->ids[button->num_of_ids++] = id;
    button->id = id; 
}

function b32
is_down(Button button)
{ 
    if (button.current_state) return true; 
    return false; 
}

function b32
on_down(Button button)
{
    if (button.current_state && button.current_state != button.previous_state) return true;
    return false;
}

struct Controller
{
    union
    {
        struct
        {
            Button right;
            Button up;
            Button left;
            Button down;
            
            Button select;
            Button pause;
        };
        Button buttons[6];
    };
};

struct Input
{
    Controller controllers[5];
    u32 num_of_controllers;
    Controller *active_controller;
    
    SDL_Joystick *joysticks[4];
    SDL_GameController *game_controllers[4];
    u32 num_of_joysticks;
};

function void
init_controllers(Input *input)
{
    input->num_of_controllers = 1; // keyboard
    Controller *keyboard = &input->controllers[0];
    set(&keyboard->right, SDLK_d);
    set(&keyboard->right, SDLK_RIGHT);
    set(&keyboard->up, SDLK_w);
    set(&keyboard->up, SDLK_UP);
    set(&keyboard->left, SDLK_a);
    set(&keyboard->left, SDLK_LEFT);
    set(&keyboard->down, SDLK_s);
    set(&keyboard->down, SDLK_DOWN);
    
    input->active_controller = keyboard;
}

struct Application
{
    Window window;
    Time time;
    Input input;
    Assets assets;
    
    void *data;
};

function b32 update(Application *app);
function void* init_data(Assets *assets);

#endif //APPLICATION_H
