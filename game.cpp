#include "log.h"
#include "types.h"
#include "types_math.h"
#include "char_array.h"
#include "assets.h"
#include "shapes.h"
#include "data_structures.h"
#include "gui.h"
#include "game.h"
#include "application.h"

#include "menu.cpp"

/*
TODO

Make main menu and pause menu look better

Clean up obj file loader (make string to float a seperate function)
Clean up mtl file loader
Improve rendering of models

Make boat movement better
Add pitch and yaw to boat

*/

// returns game mode
function s32
draw_main_menu(Application *app, Game_Data *data)
{
    Controller *menu_controller = app->input.active_controller;
    
    Rect window_rect = {};
    window_rect.coords = { 0, 0 };
    window_rect.dim    = cv2(app->window.dim);

    Menu main_menu = {};
    main_menu.font = find_font(&app->assets, "CASLON");
    main_menu.rect = get_centered_rect(window_rect, 0.5f, 0.5f);

    main_menu.button_style.default_back_color = {  34,  44, 107,   1 };
    main_menu.button_style.active_back_color  = {  42,  55, 131,   1 };
    main_menu.button_style.default_text_color = { 234,   0,  39,   1 };
    main_menu.button_style.active_text_color  = { 171, 160, 200,   1 };;
    //main_menu.button_style.active_text_color  = { ,   0,  255,   1 };
    
    main_menu.button_style.dim = { main_menu.rect.dim.x, main_menu.rect.dim.y / 3.0f };

    b32 select = on_down(menu_controller->select);
    u32 index = 0;

    orthographic(data->matrices_ubo, &app->matrices);
    draw_rect({ 0, 0 }, 0, cv2(app->window.dim), { 37, 38, 90, 1.0f} );
    draw_rect(main_menu.rect.coords, 0, main_menu.rect.dim, { 0, 0, 0, 0.2f} );
    if (menu_button(&main_menu, "2D",   index++, data->active, select)) data->game_mode = IN_GAME_2D;
    if (menu_button(&main_menu, "3D",   index++, data->active, select)) data->game_mode = IN_GAME_3D;    
    if (menu_button(&main_menu, "Quit", index++, data->active, select)) return true;

    return false;
}

// return 0 if 
function s32
draw_pause_menu(Assets *assets, v2 window_dim, b32 select, s32 active)
{
    Rect window_rect = {};
    window_rect. coords = { 0, 0 };
    window_rect.dim = window_dim;

    Menu pause_menu = {};
    pause_menu.font = find_font(assets, "CASLON");
    pause_menu.rect = get_centered_rect(window_rect, 0.7f, 0.5f);
    
    pause_menu.button_style.default_back_color = {  34,  44, 107,   1 };
    pause_menu.button_style.active_back_color  = {  42,  55, 131,   1 };
    pause_menu.button_style.default_text_color = { 234,   0,  39,   1 };
    pause_menu.button_style.active_text_color  = { 171, 160, 200,   1 };;
    pause_menu.button_style.dim = { pause_menu.rect.dim.x, pause_menu.rect.dim.y / 2.0f };

    u32 index = 0;
    draw_rect(pause_menu.rect.coords, 0, pause_menu.rect.dim, { 0, 0, 0, 0.2f} );
    if (menu_button(&pause_menu, "Unpause", index++, active, select))   return 1;
    if (menu_button(&pause_menu, "Main Menu", index++, active, select)) return 2;
    return 0;
}

#include "2D.cpp"
#include "3D.cpp"

void* init_data(Assets *assets)
{
    Game_Data *data = (Game_Data*)malloc(sizeof(Game_Data));
    *data = {};

    init_console(&data->console, assets);
    
    // 3D
    data->camera.position = { 0, 5, 10 };
    data->camera.up       = { 0, 1, 0 };
    data->camera.target   = { 0, 0, -2 };
    data->camera.yaw      = -90.0f;
    data->camera.fov      = 80.0f;
    
    data->light.position = { 5.0f, 20.0f, 10.0f };
    data->light.color = { 1.0f, 1.0f, 1.0f, 1.0f };
    
    Mesh temp_square_mesh = create_square_mesh(10, 10);
    Mesh temp_patch_mesh = make_square_mesh_into_patches(&temp_square_mesh, 10, 10);
    data->water = temp_patch_mesh;
    
    data->game_mode = IN_GAME_3D;
    data->cube = get_cube_mesh();
    
    data->tree = load_obj("../assets/objs/tails/", "tails.obj");
    data->boat_model = load_obj("../assets/objs/boat/", "boat2.obj");
    
    Shader *shader = find_shader(assets, "MATERIAL");
    platform_set_uniform_block_binding(shader->handle, "Matrices", 0);
    
    shader = find_shader(assets, "WATER");
    platform_set_uniform_block_binding(shader->handle, "Matrices", 0);
    platform_set_uniform_block_binding(shader->handle, "Wav",      1);
    
    data->matrices_ubo = init_uniform_buffer_object(2 * sizeof(m4x4), 0);
    data->wave_ubo = init_uniform_buffer_object(5 * sizeof(Wave), 1);
    
    data->waves[0] = get_wave({ 1.0, 0.0 }, 20.0f, 0.2f);
    data->waves[1] = get_wave({ 1.0, 1.0 }, 1.0f, 0.15f);
    data->waves[2] = get_wave({ 0.0, 0.4 }, 2.0f, 0.1f);
    data->waves[3] = get_wave({ 0.7, 0.9 }, 9.0f, 0.05f);
    data->waves[4] = get_wave({ 0.0, -1.0 }, 10.0f, 0.25f);

    platform_set_uniform_buffer_data(data->wave_ubo, sizeof(Wave) * 5, (void*)&data->waves);
    
    data->boat3D.coords = { 0, -1.5f, 0 };
    
    data->skybox_cube = get_cube_mesh(false);
    data->skybox = load_cubemap();

    // 2D
    init_boat(&data->boat);
    
    return (void*)data;
}

// returns true if the application should quit
b8 update(void *application)
{
    Application *app = (Application*)application;
    Game_Data *data = (Game_Data*)app->data;
    Controller *controller = app->input.active_controller;

    if (console_command(&data->console, TOGGLE_WIREFRAME))
    {
        data->wire_frame = !data->wire_frame;
        if (data->wire_frame) platform_set_polygon_mode(PLATFORM_POLYGON_MODE_LINE);
        else                  platform_set_polygon_mode(PLATFORM_POLYGON_MODE_FILL);
    }

    if (console_command(&data->console, RELOAD_SHADERS))
    {
        Shader *shader = find_shader(&app->assets, "WATER");
        load_shader(shader);
        compile_shader(shader);
        
        platform_set_uniform_block_binding(shader->handle, "Matrices", 0);
        platform_set_uniform_block_binding(shader->handle, "Wav",      1);

        shader = find_shader(&app->assets, "MATERIAL");
        load_shader(shader);
        compile_shader(shader);
    }

    if (console_command(&data->console, TOGGLE_FPS))
    {
        data->show_fps = !data->show_fps;
    }

    local_persist u32 last_game_mode = GAME_MODES_COUNT;
    if (last_game_mode != data->game_mode)
    {
        last_game_mode = data->game_mode;

        // stuff I want to check every time the game mode is changed
        switch(data->game_mode)
        {
            case MAIN_MENU:  app->input.relative_mouse_mode.set(false); break;
            case IN_GAME_3D: app->input.relative_mouse_mode.set(true);  break; // change mouse mode if it just switched to 3D
        }

        data->paused = false;
        data->active = 0;
    }

    switch (data->game_mode)
    {
        case MAIN_MENU:
        {
            menu_update_active(&data->active, 0, 2, controller->backward, controller->forward);
            if (draw_main_menu(app, data)) return true; // true if quit button is clicked
        } break;
        
        case IN_GAME_2D:
        {
            update_game_2D(app, data);
            draw_game_2D(app, data);
        } break;
        
        case IN_GAME_3D:
        {
            update_game_3D(data, &data->camera, &app->input, app->time);
            draw_game_3D(app, data);
        } break;
    }
    


    return false;
}