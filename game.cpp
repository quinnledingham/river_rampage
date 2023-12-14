#include "log.h"
#include "types.h"
#include "types_math.h"
#include "char_array.h"
#include "assets.h"
#include "renderer.h"
#include "data_structures.h"
#include "shapes.h"
#include "particles.h"
#include "platform.h"

#include "gui.h"
#include "game.h"

#include "menu.cpp"
#include "2D.cpp"
#include "3D.cpp"

// srand at beginning of main_loop()
function s32
random(s32 lower, s32 upper) 
{
    return lower + (rand() % (upper - lower));
}


// returns game mode
function s32
draw_main_menu(Game *game, Matrices *matrices, Assets *assets, Input *input, v2s window_dim)
{
    Controller *menu_controller = input->active_controller;

    Rect window_rect = {};
    window_rect.coords = { 0, 0 };
    window_rect.dim    = cv2(window_dim);

    Menu main_menu = {};
    main_menu.font = find_font(assets, "CASLON");
    main_menu.rect = get_centered_rect(window_rect, 0.5f, 0.5f);

    main_menu.button_style.default_back_color = {  34,  44, 107, 1 };
    main_menu.button_style.active_back_color  = {  42,  55, 131, 1 };
    main_menu.button_style.default_text_color = { 234,   0,  39, 1 };
    main_menu.button_style.active_text_color  = { 171, 160, 200, 1 };;
    //main_menu.button_style.active_text_color = { ,   0,  255, 1 };
    
    main_menu.button_style.dim = { main_menu.rect.dim.x, main_menu.rect.dim.y / 5.0f };

    b32 select = on_down(menu_controller->select);
    u32 index = 0;

    orthographic(game->ubos.matrices, matrices);
    draw_rect({ 0, 0 }, 0, cv2(window_dim), { 37, 38, 90, 1.0f} );
    draw_rect(main_menu.rect.coords, 0, main_menu.rect.dim, { 0, 0, 0, 0.2f} );

    if (menu_button(&main_menu, "2D",   index++, game->active, select)) 
        game->mode = IN_GAME_2D;
    if (menu_button(&main_menu, "3D",   index++, game->active, select))
        game->mode = IN_GAME_3D;    
    if (menu_button(&main_menu, "Quit", index++, game->active, select)) 
        return true;
    if (menu_button(&main_menu, "Px++", index++, game->active, select))
        game->test_pixel_height++;
    if (menu_button(&main_menu, "Px--", index++, game->active, select))
        game->test_pixel_height--;

    const char *test_string = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    draw_string(main_menu.font, test_string, { 0, 75 }, game->test_pixel_height, { 255, 255, 255, 1} );
    draw_string(main_menu.font, float_to_char_array(game->test_pixel_height), { 0, 200 }, game->test_pixel_height, { 255, 255, 255, 1 });

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


function void
set_shader_uniform_block_bindings(Shader *shader, const Uniform_Buffer_Objects *ubos) {
    for (u32 i = 0; i < ARRAY_COUNT(ubos->E); i++) {
        platform_set_uniform_block_binding(shader->handle, ubos->tags[i], i);
    }
}

internal void
init_camera_menu(Easy_Textboxs *easy, Font *font, Camera *camera) {
    u32 index = 0;
    easy->boxs[index++] = v3_textbox(&camera->position);
    easy->boxs[index++] = v3_textbox(&camera->target);
    easy->boxs[index++] = v3_textbox(&camera->up);
    easy->boxs[index++] = f32_textbox(&camera->fov);
    easy->boxs[index++] = f32_textbox(&camera->yaw);
    easy->boxs[index++] = f32_textbox(&camera->pitch);
    easy->num_of_boxs = 6;
    easy->draw = default_draw_textbox;
    easy->draw.font = font;
}

internal void
init_waves_data(Waves_Data *waves_data, u32 ubo) {
    waves_data->waves[0] = get_wave({  1.0f,  0.0f }, 20.0f, 0.2f);
    waves_data->waves[1] = get_wave({  1.0f,  1.0f }, 50.0f, 0.3f);
    waves_data->waves[2] = get_wave({  0.0f,  0.4f },  5.0f, 0.1f);
    waves_data->waves[3] = get_wave({ -0.7f,  0.9f },  9.0f, 0.1f);
    waves_data->waves[4] = get_wave({  0.1f, -0.9f }, 15.0f, 0.25f);
    waves_data->num_of_waves = 5;

    {
        u32 target = gl_get_buffer_target(UNIFORM_BUFFER);
        u32 offset = 0;

        bind_buffer(target, ubo);
        offset = BUFFER_SUB_DATA_ARRAY(target, offset, waves_data->waves);
        offset = BUFFER_SUB_DATA(target, offset, waves_data->num_of_waves);
        unbind_buffer(target);
    }
}

void* init_data(Assets *assets)
{
    Game_Data *data = (Game_Data*)malloc(sizeof(Game_Data));
    *data = {};

    Game *game = &data->game;
    Dev_Tools *tools = &data->dev_tools;
    Game_2D *game_2D = &data->game_2D;
    Game_3D *game_3D = &data->game_3D;

    // the sizes I'm creating here are to match the size of the structs in the shaders
    // so I don't have to wait for any thing from the game to determine the size
    game->ubos.matrices = init_uniform_buffer_object(2 * sizeof(m4x4) + 4 * sizeof(f32),                                  0); // ubos.E[0]
    game->ubos.waves    = init_uniform_buffer_object(ARRAY_COUNT(game_3D->waves_data.waves) * sizeof(Wave) + sizeof(u32), 1); // ubos.E[1]
    game->ubos.lights   = init_uniform_buffer_object(sizeof(Light),                                                       2); // ubos.E[2]

    // set up all of the shaders with the uniform block bindings
    for (u32 shader_index = 0; shader_index < assets->types[ASSET_TYPE_SHADER].num_of_assets; shader_index++) {
        set_shader_uniform_block_bindings((Shader*)&assets->types[ASSET_TYPE_SHADER].data[shader_index].memory, &game->ubos);
        /*
        Shader *shader = (Shader *)&assets->types[ASSET_TYPE_SHADER].data[shader_index].memory;
        for (u32 ubo_index = 0; ubo_index < ARRAY_COUNT(game->ubos.E); ubo_index++) {
            // gives the shader my uniform block bindings
            platform_set_uniform_block_binding(shader->handle, game->ubos.tags[ubo_index], ubo_index);
        }
        */
    }

    Font *caslon = find_font(assets, "CASLON");

    // Dev Tools
    init_console(&tools->console, caslon);

    tools->onscreen_notifications.font = caslon;
    tools->onscreen_notifications.text_color = { 255, 255, 255, 1 };

    init_camera_menu(&tools->camera_menu, caslon, &game_3D->camera);

    // 3D
    game_3D->camera.position = { 5, 40, 0 };
    game_3D->camera.target   = { 0, 0, -2 };
    game_3D->camera.up       = { 0, 1, 0 };
    game_3D->camera.fov      = 70.0f;
    game_3D->camera.yaw      = 0.0f;
    game_3D->camera.pitch    = -85.0f;
    
    game_3D->light.position = { 5.0f, 60.0f, 10.0f };
    game_3D->light.ambient  = { 0.3f, 0.3f, 0.3f };
    game_3D->light.diffuse  = { 0.9f, 0.9f, 0.9f };
    game_3D->light.specular = { 0.5f, 0.5f, 0.5f };
    game_3D->light.color    = { 1.0f, 1.0f, 1.0f, 1.0f };

    // init light ubo
    {
        u32 target = gl_get_buffer_target(UNIFORM_BUFFER);
        u32 offset = 0;
        u32 ubo = game->ubos.lights;

        bind_buffer(target, ubo);
        UNIFORM_BUFFER_SUB_DATA(0, game_3D->light);
        unbind_buffer(target);
    }
    
    game_3D->triangle_mesh = create_square_mesh(100, 100, true);
    init_mesh(&game_3D->triangle_mesh);

    Mesh temp = create_square_mesh(100, 100, false);
    game_3D->water = make_square_mesh_into_patches(&temp, 100, 100);
    
    init_waves_data(&game_3D->waves_data, game->ubos.waves);

    init_boat_3D(&game_3D->boat3D, find_font(assets, "CASLON"));

    game_3D->skybox_cube = get_cube_mesh(false);
    game_3D->skybox = load_cubemap();

    // 2D
    init_boat(&game_2D->boat);
    
    return (void*)data;
}

function void
update_matrices(Matrices *m, r32 fov, r32 aspect_ratio, v2s window_dim)
{
    m->window_width = (f32)window_dim.width;
    m->window_height = (f32)window_dim.height;
    m->perspective_matrix = perspective_projection(fov, aspect_ratio, m->p_near, m->p_far);
    m->orthographic_matrix = orthographic_projection(0.0f, (r32)window_dim.width, (r32)window_dim.height, 0.0f, -3.0f, 3.0f);
    m->update = false;
}

// returns true if the application should quit
b8 update(void *application)
{
    Application *app = (Application*)application;
    Game_Data *data = (Game_Data*)app->data;
    Game *game = &data->game;
    Dev_Tools *tools = &data->dev_tools;
    Game_2D *game_2D = &data->game_2D;
    Game_3D *game_3D = &data->game_3D;
    
    Controller *controller = app->input.active_controller;

    renderer_window_dim = app->window.dim;

    if (app->matrices.update) // only updates on load and window size change
        update_matrices(&app->matrices, game_3D->camera.fov, app->window.aspect_ratio, app->window.dim);

    if (console_command(&tools->console, TOGGLE_WIREFRAME)) {
        tools->wire_frame = !tools->wire_frame;
        if (tools->wire_frame) 
            set_polygon_mode(POLYGON_MODE_LINE);
        else                  
            set_polygon_mode(POLYGON_MODE_FILL);
    }

    if (console_command(&tools->console, RELOAD_SHADERS))
    {
        Shader *shader = find_shader(&app->assets, "WATER");
        load_shader(shader);
        compile_shader(shader);
        set_shader_uniform_block_bindings(shader, &game->ubos);

        shader = find_shader(&app->assets, "MATERIAL");
        load_shader(shader);
        compile_shader(shader);
        set_shader_uniform_block_bindings(shader, &game->ubos);

        shader = find_shader(&app->assets, "MATERIAL_TEX");
        load_shader(shader);
        compile_shader(shader);
        set_shader_uniform_block_bindings(shader, &game->ubos);

        shader = find_shader(&app->assets, "PARTICLE");
        load_shader(shader);
        compile_shader(shader);
        set_shader_uniform_block_bindings(shader, &game->ubos);
    }

    if (console_command(&tools->console, TOGGLE_FPS)) {
        tools->show_fps = !tools->show_fps;
    }

    local_persist u32 last_game_mode = GAME_MODES_COUNT;
    if (last_game_mode != game->mode) {
        last_game_mode = game->mode;

        // stuff I want to check every time the game mode is changed
        switch(game->mode) {
            case MAIN_MENU:  app->input.relative_mouse_mode.set(false); break;
            case IN_GAME_3D: app->input.relative_mouse_mode.set(true);  break; // change mouse mode if it just switched to 3D
        }

        game->paused = false;
        game->active = 0;
    }

    switch (game->mode)
    {
        case MAIN_MENU: {
            menu_update_active(&game->active, 0, 4, controller->backward, controller->forward);
            if (draw_main_menu(game, &app->matrices, &app->assets, &app->input, app->window.dim)) 
                return true; // true if quit button is clicked
        } break;
        
        case IN_GAME_2D: {
            update_game_2D(game, game_2D, &app->input, app->time);
            draw_game_2D(game, game_2D, &app->matrices, &app->assets, &app->input, app->window.dim);
        } break;
        
        case IN_GAME_3D: {
            update_game_3D(game, game_3D, tools, &app->input, app->time);
            draw_game_3D(game, game_3D, tools, &app->matrices, app->time, &app->assets, &app->input, app->window.dim, app->tex_depth_buffer, app->color_buffer_texture);
        } break;
    }
    
    return false;
}