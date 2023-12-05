#include <gl.h>
#include <gl.c>
#include <SDL.h>

#include "log.h"
#include "types.h"
#include "types_math.h"
#include "char_array.h"
#include "assets.h"
#include "renderer.h"
#include "data_structures.h"
#include "shapes.h"
#include "particles.h"
#include "application.h"

#include "log.cpp"
#include "data_structures.cpp"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_resize.h>
#include <stb_truetype.h>

#include "assets.cpp"
#include "renderer.cpp"
#include "shapes.cpp"
#include "particles.cpp"

#if WINDOWS

// Enabling Dedicated Graphics on Laptops
// https://www.reddit.com/r/gamedev/comments/bk7xbe/psa_for_anyone_developing_a_gameengine_in_c/
//
// NOTE: If the laptop is in power saving mode (like it usually is when not plugged in) the graphics card
// will be severely limited. 
//
extern "C" 
{
    __declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001; // Nvidia
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;   // AMD
}

#define WIN32_LEAN_AND_MEAN
#define WIN32_EXTRA_LEAN
#include <windows.h>

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "shell32.lib")

#include <mmsystem.h>
#include <dsound.h>
#include <intrin.h>
#include <xinput.h>

#include "win32_application.cpp"

#endif // WINDOWS

// function that are defined in the game code
b8 update(void *application);
void* init_data(Assets *assets);

void *platform_malloc(u32 size) {
    return SDL_malloc(size);
}

void platform_free(void *ptr) {
    SDL_free(ptr);
}

function void
update_window(Window *window)
{
    glViewport(0, 0, window->dim.width, window->dim.height);
    window->aspect_ratio = (r32)window->dim.width / (r32)window->dim.height;
    *window->update_matrices = true;
}

function void
reset_controller(Controller *controller)
{
    //controller->mouse = {};
    controller->mouse_rel = {};
    for (u32 j = 0; j < ARRAY_COUNT(controller->buttons); j++) {
        if (j == 11) { 
            continue; // skip mouse_left
        }
        controller->buttons[j].current_state = 0;
        controller->buttons[j].previous_state = 0;
    }
}

function void
prepare_controller_for_input(Controller *controller)
{
    //controller->mouse = {};
    controller->mouse_rel = {};
    for (u32 j = 0; j < ARRAY_COUNT(controller->buttons); j++)
        controller->buttons[j].previous_state = controller->buttons[j].current_state;
}

function void
controller_process_input(Controller *controller, s32 id, b32 state)
{
    for (u32 i = 0; i < ARRAY_COUNT(controller->buttons); i++) {
        // loop through all ids associated with button
        for (u32 j = 0; j < controller->buttons[i].num_of_ids; j++) {
            if (id == controller->buttons[i].ids[j]) controller->buttons[i].current_state = state;
        }
    }
}

internal void
keyboard_input_to_char_array(s32 id, char *buffer, u32 *buffer_index, b32 shift)
{
    s32 ch = 0;

    if (is_ascii(id)) {
        ch = id;
        if (isalpha(ch) && shift) ch -= 32;
        if (ch == '3'   && shift) ch = '#';
        if (ch == '-'   && shift) ch = '_';
    }
    else if (id == SDLK_LEFT)  ch = 37;
    else if (id == SDLK_UP)    ch = 38;
    else if (id == SDLK_RIGHT) ch = 39;
    else if (id == SDLK_DOWN)  ch = 40;

    buffer[(*buffer_index)++] = ch;
}

function b32
process_input(Window *window, Input *input)
{
    for (u32 i = 0; i < input->num_of_controllers; i++) prepare_controller_for_input(&input->controllers[i]);
    
    u32 buffer_index = 0;
    input->buffer_index = 0;
    SDL_memset(input->buffer, 0, 10);

    // Clear the controllers if the input mode is switch from game to keyboard.
    // Keyboard is always cleared so no need going the other way.
    local_persist u32 last_input_mode = 0;
    if (last_input_mode != input->mode)
    {
        last_input_mode = input->mode;
        if (input->mode == INPUT_MODE_KEYBOARD)
        {
            //for (u32 i = 0; i < input->num_of_controllers; i++) reset_controller(&input->controllers[i]);
        }
    }

    SDL_Event event;
    while(SDL_PollEvent(&event))
    {
        switch(event.type)
        {
            case SDL_QUIT: return true;
            
            case SDL_WINDOWEVENT:
            {
                SDL_WindowEvent *window_event = &event.window;
                
                switch(window_event->event)
                {
                    case SDL_WINDOWEVENT_RESIZED:
                    case SDL_WINDOWEVENT_SIZE_CHANGED:
                    {
                        window->dim.width  = window_event->data1;
                        window->dim.height = window_event->data2;
                        
                        update_window(window);
                    } break;
                }
            } break;
            
            case SDL_MOUSEMOTION:
            {
                input->active_controller = &input->controllers[0];
                SDL_MouseMotionEvent *mouse_motion_event = &event.motion;
                input->active_controller->mouse.x = mouse_motion_event->x;
                input->active_controller->mouse.y = mouse_motion_event->y;
                input->active_controller->mouse_rel.x = mouse_motion_event->xrel;
                input->active_controller->mouse_rel.y = mouse_motion_event->yrel;
            } break;
            
            case SDL_MOUSEBUTTONDOWN:
            case SDL_MOUSEBUTTONUP: 
            {
                input->active_controller = &input->controllers[0];
                SDL_MouseButtonEvent *mouse_button_event = &event.button;
                
                s32 button_id = mouse_button_event->button;
                b32 state = false;
                if (mouse_button_event->state == SDL_PRESSED) state = true;

                controller_process_input(&input->controllers[0], button_id, state);

                input->active_controller->mouse.x = mouse_button_event->x;
                input->active_controller->mouse.y = mouse_button_event->y;
            } break;

            // keyboard input
            case SDL_KEYDOWN:
            case SDL_KEYUP:
            {
                input->active_controller = &input->controllers[0];
                SDL_KeyboardEvent *keyboard_event = &event.key;
                s32 key_id = keyboard_event->keysym.sym;
                b32 shift = keyboard_event->keysym.mod & KMOD_LSHIFT;
                b32 state = false;
                if (keyboard_event->state == SDL_PRESSED) state = true;

                controller_process_input(&input->controllers[0], key_id, state);
            
                if (input->mode == INPUT_MODE_KEYBOARD && state)
                {
                    if (buffer_index >= 10) {
                        error("process_input(): too many inputs for buffer in input_mode keyboard");
                        break;
                    }

                    keyboard_input_to_char_array(key_id, input->buffer, &buffer_index, shift);
                }
            } break;
        }
    }
    
    return false;
}

function f32
get_seconds(u64 start, u64 end)
{
    u64 diff = end - start;
    return (f32)diff / 1000.0f;
}

function void
update_time(Time *time)
{
    r32 last_run_time_ms = time->run_time_ms;
    
    time->run_time_ms = (r32)SDL_GetTicks64();
    time->run_time_s = (f32)time->run_time_ms / 1000.0f;
    time->frame_time_ms = time->run_time_ms - last_run_time_ms;
    time->frame_time_s = (f32)time->frame_time_ms / 1000.0f;

    if (time->frame_time_ms == 0) 
        time->frame_time_s = 0.001f;
    
    // get fps
    time->frames_per_s = 1000.0f;
    if (time->frame_time_s > 0.0f) 
        time->frames_per_s = 1.0f / time->frame_time_s;
}

function void
update_relative_mouse_mode(Flag *flag)
{
    if (flag->changed()) 
    {
        if (flag->get()) SDL_SetRelativeMouseMode(SDL_TRUE);
        else             SDL_SetRelativeMouseMode(SDL_FALSE);
    }
}

function void swap_window(SDL_Window *sdl_window) { SDL_GL_SwapWindow(sdl_window); }

function int
main_loop(Application *app, SDL_Window *sdl_window)
{
    srand(SDL_GetTicks());
    
    Bitmap *icon = find_bitmap(&app->assets, "ICON");
    SDL_Surface *icon_surface = SDL_CreateRGBSurfaceFrom(icon->memory, icon->dim.width, icon->dim.height, 32, icon->pitch, 0x00000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
    SDL_SetWindowIcon(sdl_window, icon_surface);

    s64 last_time = 0;

    while(1)
    {
        if (process_input(&app->window, &app->input)) return 0; // quit if process_input returns false

        win32_update_time(&app->time);

        if (update(app)) return 0;

        update_relative_mouse_mode(&app->input.relative_mouse_mode);

        swap_window(sdl_window);

        u32 gl_clear_flags = 
            GL_COLOR_BUFFER_BIT | 
            GL_DEPTH_BUFFER_BIT | 
            GL_STENCIL_BUFFER_BIT;
    
        glClear(gl_clear_flags);
    }
}

function void
init_controllers(Input *input)
{
    input->active_controller = &input->controllers[0];
    
    Controller *keyboard = &input->controllers[0];
    set(&keyboard->right,    SDLK_d);
    set(&keyboard->left,     SDLK_a);
    set(&keyboard->forward,  SDLK_w);
    set(&keyboard->backward, SDLK_s);
    set(&keyboard->up,       SDLK_SPACE);
    set(&keyboard->down,     SDLK_LSHIFT);
    set(&keyboard->select,   SDLK_RETURN);
    set(&keyboard->pause,    SDLK_ESCAPE);
    
    set(&keyboard->reload_shaders, SDLK_r);
    set(&keyboard->toggle_camera_mode, SDLK_c);
    set(&keyboard->toggle_console, SDLK_t);
    set(&keyboard->mouse_left, SDL_BUTTON_LEFT);
    
    input->num_of_controllers = 1;
}

function void
init_opengl(SDL_Window *sdl_window)
{
    SDL_GL_LoadLibrary(NULL);
    
    // Request an OpenGL 4.6 context (should be core)
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL,    1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    
    // Also request a depth buffer
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE,   24);
    
    SDL_GLContext Context = SDL_GL_CreateContext(sdl_window);
    SDL_GL_SetSwapInterval(0); // vsync: 0 off, 1 on
    
    // Check OpenGL properties
    gladLoadGL((GLADloadfunc) SDL_GL_GetProcAddress);
    log("OpenGL loaded:");
    log("Vendor:   %s", glGetString(GL_VENDOR));
    log("Renderer: %s", glGetString(GL_RENDERER));
    log("Version:  %s", glGetString(GL_VERSION));
}

// update_matrices is a pointer to the bool that controls if the matrices should be updated
// to be used when the window changes size.
function SDL_Window*
init_window(Window *window, b32 *update_matrices)
{
    u32 sdl_init_flags = 
        SDL_INIT_VIDEO          | 
        SDL_INIT_GAMECONTROLLER | 
        SDL_INIT_AUDIO;
    
    u32 sdl_window_flags = 
        SDL_WINDOW_RESIZABLE | 
        SDL_WINDOW_OPENGL;
    
    SDL_Init(sdl_init_flags);
    
    SDL_Window *sdl_window = SDL_CreateWindow("River Rampage", 
                                   SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                   900, 800, 
                                   sdl_window_flags);
    init_opengl(sdl_window);
    SDL_GetWindowSize(sdl_window, &window->dim.width, &window->dim.height);
    window->update_matrices = update_matrices;
    update_window(window);

    return sdl_window;
}

int main(int argc, char *argv[])  
{ 
    Application app = {};
    SDL_Window *sdl_window = init_window(&app.window, &app.matrices.update);

    global_perf_count_frequency = win32_performance_frequency();
    app.time.start = win32_get_ticks();

    // Loading assets
    u64 assets_loading_time_started = SDL_GetTicks64();

    if (equal(argv[1], "load_assets")) {
        if (load_assets(&app.assets, "../assets.ethan")) 
            return 1;
        save_assets(&app.assets, "assets.save");
    } else {
        if (load_saved_assets(&app.assets, "assets.save")) 
            return 1;
    }

    init_assets(&app.assets);

    // Setting up app and data
    app.data = (void*)init_data(&app.assets);
    log("time loading assets: %f", get_seconds(assets_loading_time_started, SDL_GetTicks64()));

    init_controllers(&app.input);
    app.input.relative_mouse_mode.set(false);

    // GL defaults
    set_blend_function(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glPatchParameteri(GL_PATCH_VERTICES, 4); 

    init_particles(&global_particles, 1000);
    init_shapes();
    app.tex_depth_buffer = get_depth_buffer_texture(app.window.dim);
    app.color_buffer_texture = get_color_buffer_texture(app.window.dim);

    app.matrices.p_near = 0.1f;
    app.matrices.p_far = 1000.0f;
    app.matrices.update = true;

    return main_loop(&app, sdl_window);
}