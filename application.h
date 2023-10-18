#ifndef APPLICATION_H
#define APPLICATION_H

struct Window
{
    SDL_Window *sdl;
    v2s dim;
    r32 aspect_ratio; // update with dim change
    
    b32 update_matrices;
};

function void swap_window(Window *window) { SDL_GL_SwapWindow(window->sdl); }

struct Matrices // for rendering
{
    m4x4 perspective_matrix;
    m4x4 orthographic_matrix;
    m4x4 view_matrix;
};

// functions to set matrices in uniform buffer
function void
orthographic(u32 ubo, Matrices *matrices)
{
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0,            sizeof(m4x4), (void*)&matrices->orthographic_matrix);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(m4x4), sizeof(m4x4), (void*)&identity_m4x4());
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}
function void
perspective(u32 ubo, Matrices *matrices)
{
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0,            sizeof(m4x4), (void*)&matrices->perspective_matrix);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(m4x4), sizeof(m4x4), (void*)&matrices->view_matrix);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

struct Time
{
    u64 run_time_ms;
    r32 run_time_s;
    u32 frame_time_ms;
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

void set(Button *button, s32 id) 
{
    if (button->num_of_ids > 2)
        error("set() too many ids trying to be assigned to button");
    
    button->ids[button->num_of_ids++] = id;
    button->id = id; 
}

b32 is_down(Button button) 
{ 
    if (button.current_state) return true; return false; 
}

b32 on_down(Button button)
{
    if (button.current_state && button.current_state != button.previous_state) return true;
    return false;
}

// delta_mouse is a relative mouse movement amount
// as opposed to the screen coords of the mouse
function void
update_camera_with_mouse(Camera *camera, v2s delta_mouse, v2 move_speed)
{
    //printf("delta %f, move %f\n", (f32)delta_mouse.x, move_speed.x);
    
    camera->yaw   += (f32)delta_mouse.x * move_speed.x;
    camera->pitch -= (f32)delta_mouse.y * move_speed.y;
    
    if (camera->pitch > 89.0f) camera->pitch = 89.0f;
    if (camera->pitch < -89.0f) camera->pitch = -89.0f;
    
    v3 camera_direction = 
    {
        cosf(DEG2RAD * camera->yaw) * cosf(DEG2RAD * camera->pitch),
        sinf(DEG2RAD * camera->pitch),
        sinf(DEG2RAD * camera->yaw) * cosf(DEG2RAD * camera->pitch)
    };
    camera->target = normalized(camera_direction);
}

function void
update_camera_with_keys(Camera *camera, v3 move_vector,
                        Button forward, Button backward,
                        Button left, Button right,
                        Button up, Button down)
{
    if (is_down(forward))  camera->position += camera->target * move_vector;
    if (is_down(backward)) camera->position -= camera->target * move_vector;
    if (is_down(left))     camera->position -= normalized(cross_product(camera->target, camera->up)) * move_vector;
    if (is_down(right))    camera->position += normalized(cross_product(camera->target, camera->up)) * move_vector;
    if (is_down(up))       camera->position.y += move_vector.y;
    if (is_down(down))     camera->position.y -= move_vector.y;
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
        };
        Button buttons[10];
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
    
    SDL_Joystick *joysticks[4];
    SDL_GameController *game_controllers[4];
    u32 num_of_joysticks;
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
