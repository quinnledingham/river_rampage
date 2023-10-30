#include <gl.h>
#include <gl.c>
#include <SDL.h>

#include "log.h"
#include "types.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_resize.h>
#include <stb_truetype.h>

#include "assets.h"
#include "shapes.h"
#include "data_structures.h"
#include "game.h"
#include "application.h"

#include "log.cpp"
#include "data_structures.cpp"
#include "assets.cpp"
#include "shapes.cpp"


void *platform_malloc(u32 size)
{
    return SDL_malloc(size);
}

void platform_free(void *ptr)
{
    SDL_free(ptr);
}

//
// Renderer
//

// block index is from glUniformBlockBinding or binding == #
u32 init_uniform_buffer_object(u32 block_size, u32 block_index)
{
    u32 uniform_buffer_object;
    glGenBuffers(1, &uniform_buffer_object);
    
    glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer_object);
    glBufferData(GL_UNIFORM_BUFFER, block_size, NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    
    glBindBufferBase(GL_UNIFORM_BUFFER, block_index, uniform_buffer_object);
    
    return uniform_buffer_object;
}

void platform_set_uniform_block_binding(u32 shader_handle, const char *tag, u32 index)
{
    u32 tag_uniform_block_index = glGetUniformBlockIndex(shader_handle, tag);
    glUniformBlockBinding(shader_handle, tag_uniform_block_index, index);
}

void platform_set_uniform_buffer_data(u32 ubo, u32 size, void *data)
{
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, size, data);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

// functions to set matrices in uniform buffer
void orthographic(u32 ubo, Matrices *matrices)
{
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0,            sizeof(m4x4), (void*)&matrices->orthographic_matrix);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(m4x4), sizeof(m4x4), (void*)&identity_m4x4());
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void perspective(u32 ubo, Matrices *matrices)
{
    GLenum target = GL_UNIFORM_BUFFER;
    glBindBuffer(target, ubo);
    glBufferSubData(target, 0,            sizeof(m4x4), (void*)&matrices->perspective_matrix);
    glBufferSubData(target, sizeof(m4x4), sizeof(m4x4), (void*)&matrices->view_matrix);
    glBindBuffer(target, 0);
}

void platform_uniform_m4x4(u32 shader_handle, const char *tag, m4x4 *m) { glUniformMatrix4fv(glGetUniformLocation(shader_handle, tag), (GLsizei)1, false, (float*) m); }
void platform_uniform_f32 (u32 shader_handle, const char *tag, f32   f) { glUniform1f       (glGetUniformLocation(shader_handle, tag),                             f); }
void platform_uniform_v3  (u32 shader_handle, const char *tag, v3    v) { glUniform3fv      (glGetUniformLocation(shader_handle, tag), (GLsizei)1,        (float*)&v); }
void platform_uniform_v4  (u32 shader_handle, const char *tag, v4    v) { glUniform4fv      (glGetUniformLocation(shader_handle, tag), (GLsizei)1,        (float*)&v); }

void platform_set_texture(Bitmap *bitmap)
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, bitmap->handle);
}

void platform_blend_function(u32 source_factor, u32 destination_factor)
{
    glBlendFunc(source_factor, destination_factor);
}

void platform_set_polygon_mode(u32 mode)
{
    switch(mode)
    {
        case PLATFORM_POLYGON_MODE_POINT: glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
        case PLATFORM_POLYGON_MODE_LINE:  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        case PLATFORM_POLYGON_MODE_FILL:  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
}

void opengl_set_capability(GLenum capability, b32 state)
{
    if (state) glEnable(capability);
    else       glDisable(capability);
}

void platform_set_capability(u32 capability, b32 state)
{
    switch(capability)
    {
        case PLATFORM_CAPABILITY_DEPTH_TEST: opengl_set_capability(GL_DEPTH_TEST, state); break;
        case PLATFORM_CAPABILITY_CULL_FACE:  opengl_set_capability(GL_CULL_FACE, state);  break;
    }
}

//
//
//

function void
prepare_controller_for_input(Controller *controller)
{
    controller->mouse = {};
    for (u32 j = 0; j < ARRAY_COUNT(controller->buttons); j++)
        controller->buttons[j].previous_state = controller->buttons[j].current_state;
}

function void
controller_process_input(Controller *controller, s32 id, b32 state)
{
    for (u32 i = 0; i < ARRAY_COUNT(controller->buttons); i++)
    {
        // loop through all ids associated with button
        for (u32 j = 0; j < controller->buttons[i].num_of_ids; j++)
        {
            if (id == controller->buttons[i].ids[j]) controller->buttons[i].current_state = state;
        }
    }
}

function void
update_window(Window *window)
{
    glViewport(0, 0, window->dim.width, window->dim.height);
    window->aspect_ratio = (r32)window->dim.width / (r32)window->dim.height;
    *window->update_matrices = true;
}

function b32
process_input(Window *window, Input *input)
{
    for (u32 i = 0; i < input->num_of_controllers; i++) prepare_controller_for_input(&input->controllers[i]);
    
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
                input->active_controller->mouse.x = mouse_motion_event->xrel;
                input->active_controller->mouse.y = mouse_motion_event->yrel;
            } break;
            
            // keyboard input
            case SDL_KEYDOWN:
            case SDL_KEYUP:
            {
                input->active_controller = &input->controllers[0];
                SDL_KeyboardEvent *keyboard_event = &event.key;
                s32 key_id = keyboard_event->keysym.sym;
                b32 state = false;
                if (keyboard_event->state == SDL_PRESSED) state = true;
                controller_process_input(&input->controllers[0], key_id, state);
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
    u32 last_run_time_ms = time->run_time_ms;
    
    time->run_time_ms = SDL_GetTicks64();
    time->run_time_s = (f32)time->run_time_ms / 1000.0f;
    time->frame_time_ms = time->run_time_ms - last_run_time_ms;
    time->frame_time_s = (f32)time->frame_time_ms / 1000.0f;
    
    if (time->frame_time_s == 0.0f) time->frame_time_s = 0.0001f;
    
    // get fps
    time->frames_per_s = 1000.0f;
    if (time->frame_time_s > 0.0f) time->frames_per_s = 1.0f / time->frame_time_s;
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

function void
update_matrices(Matrices *m, r32 fov, r32 aspect_ratio, v2s window_dim)
{
    m->perspective_matrix = perspective_projection(fov, aspect_ratio, 0.01f, 1000.0f);
    m->orthographic_matrix = orthographic_projection(0.0f, (r32)window_dim.width, (r32)window_dim.height, 0.0f, -3.0f, 3.0f);
    m->update = false;
}

function void swap_window(SDL_Window *sdl_window) { SDL_GL_SwapWindow(sdl_window); }

function int
main_loop(Application *app, SDL_Window *sdl_window)
{
    Game_Data *data = (Game_Data*)app->data;

    while(1)
    {
        if (process_input(&app->window, &app->input)) return 0; // quit if process_input returns false

        if (app->matrices.update) update_matrices(&app->matrices, data->camera.fov, app->window.aspect_ratio, app->window.dim);
        update_time(&app->time);

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
    
    set(&keyboard->wire_frame,     SDLK_t);
    set(&keyboard->reload_shaders, SDLK_r);
    
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
    SDL_GL_SetSwapInterval(0);
    
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
        SDL_INIT_VIDEO | 
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

    u64 assets_loading_time_started = SDL_GetTicks64();
    if (load_assets(&app.assets, "../assets.ethan")) return 1;
    log("time loading assets: %f", get_seconds(assets_loading_time_started, SDL_GetTicks64()));

    init_controllers(&app.input);
    app.input.relative_mouse_mode.set(false);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glPatchParameteri(GL_PATCH_VERTICES, 4); 

    init_shapes(find_shader(&app.assets, "COLOR"), find_shader(&app.assets, "TEX"));

    app.data = (void*)init_data(&app.assets);

    return main_loop(&app, sdl_window);
}