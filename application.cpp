function void
controller_process_input(Controller *controller, s32 id, b32 state)
{
    for (u32 i = 0; i < ARRAY_COUNT(controller->buttons); i++)
    {
        // loop through all ids associated with button
        for (u32 j = 0; j < controller->buttons[i].num_of_ids; j++)
        {
            if (id == controller->buttons[i].ids[j])
                controller->buttons[i].current_state = state;
        }
    }
}

function b32
process_input(v2s *window_dim, Input *input)
{
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
                        window_dim->width = window_event->data1;
                        window_dim->height = window_event->data2;
                        glViewport(0, 0, window_dim->width, window_dim->height);
                    } break;
                }
            } break;
            
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

function s32
main_loop(Application *app)
{
    while(1)
    {
        if (process_input(&app->window.dim, &app->input)) return 0;
        
        set_orthographic_matrix(app->window.dim);
        update_time(&app->time);
        
        if (update(app)) return 0;
        
        swap_window(&app->window);
    }
    
    return 0;
}

function void
init_opengl(Window *window)
{
    SDL_GL_LoadLibrary(NULL);
    
    // Request an OpenGL 4.6 context (should be core)
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 6);
    
    // Also request a depth buffer
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    
    SDL_GLContext Context = SDL_GL_CreateContext(window->sdl);
    SDL_GL_SetSwapInterval(0);
    
    // Check OpenGL properties
    gladLoadGL((GLADloadfunc) SDL_GL_GetProcAddress);
    log("OpenGL loaded:");
    log("Vendor:   %s", glGetString(GL_VENDOR));
    log("Renderer: %s", glGetString(GL_RENDERER));
    log("Version:  %s", glGetString(GL_VERSION));
    
    window->gl_clear_flags = 
        GL_COLOR_BUFFER_BIT | 
        GL_DEPTH_BUFFER_BIT | 
        GL_STENCIL_BUFFER_BIT;
    
    SDL_GetWindowSize(window->sdl, &window->dim.width, &window->dim.height);
    glViewport(0, 0, window->dim.width, window->dim.height);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
}

function void
init_window(Window *window)
{
    u32 sdl_init_flags =
        SDL_INIT_VIDEO |
        SDL_INIT_GAMECONTROLLER |
        SDL_INIT_HAPTIC |
        SDL_INIT_AUDIO;
    
    u32 sdl_window_flags =
        SDL_WINDOW_RESIZABLE |
        SDL_WINDOW_OPENGL;
    
    SDL_Init(sdl_init_flags);
    
    window->sdl = SDL_CreateWindow("River Rampage",
                                   SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                   800, 800,
                                   sdl_window_flags);
    
    init_opengl(window);
}

function s32
application()
{
    Application app = {};
    init_window(&app.window);
    load_assets(&app.assets, "../ethan.assets");
    app.data = init_data(&app.assets);
    init_controllers(&app.input);
    return main_loop(&app);
}

int main(int argc, char *argv[]) { return application(); }