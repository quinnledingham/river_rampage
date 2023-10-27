#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#include <stb_image.h>
#include <stb_image_resize.h>
#include <stb_truetype.h>
#include <gl.h>
#include <gl.c>
#include <SDL.h>

#include "log.h"
#include "types.h"
#include "assets.h"
#include "shapes.h"
#include "data_structures.h"
#include "game.h"
#include "application.h"

#include "data_structures.cpp"
#include "assets.cpp"
#include "shapes.cpp"
#include "menu.cpp"

//
// 2D
// 

function r32
v2_to_angle(v2 vector)
{
    r32 x = vector.x;
    r32 y = vector.y;
    r32 angle = atanf(y / x);
    if ((x < 0 && y > 0) || (x < 0 && y < 0)) angle += PI;
    return angle;
}


function void 
rotate_v2(v2 *vector, r32 angle)
{
    r32 current_angle = v2_to_angle(*vector);
    r32 new_angle = current_angle + angle;
    
    vector->x = 1.0f * cosf(new_angle);
    vector->y = 1.0f * sinf(new_angle);
}

function v2
delta_velocity(v2 acceleration, r32 delta_time)
{
    return acceleration * delta_time;
}

function v2 
delta_position(v2 vector, r32 time_s)
{
    vector.y = -vector.y; // 2D screen coords +y going down screen
    return (vector * time_s) * 800.0f; // 800 pixels per meter
}

function void
init_boat(Boat *boat)
{
    boat->coords           = { 100, 100 };
    
    boat->mass             = 100.0f; 
    boat->engine_force     = 15.0f;
    boat->rudder_force     = 6.0f;
    boat->water_line_length = 100.0f;
    
    boat->direction = { 1, 0 };
    
    r32 speed_feet_per_s = 1.34f * sqrtf(boat->water_line_length);
    boat->maximum_speed  = speed_feet_per_s * 0.3048; // ft to m
    
    boat->acceleration_magnitude   = boat->engine_force / boat->mass;
    boat->water_acceleration_magnitude = boat->rudder_force / boat->mass;
}

function v2
remove_direction(const v2 v)
{
    v2 result = v;
    if (v.x < 0.0f) result.x = -v.x;
    if (v.y < 0.0f) result.y = -v.y;
    return result;
}

function void
update_boat(Boat *boat, Input *input, r32 delta_time)
{
    v2 rotation_direction = {};
    
    if (is_down(input->active_controller->left))  rotate_v2(&boat->direction,  0.05f * DEG2RAD);
    if (is_down(input->active_controller->right)) rotate_v2(&boat->direction, -0.05f * DEG2RAD);
    
    v2 acceleration_direction = {};
    if (is_down(input->active_controller->up))   acceleration_direction =  boat->direction;
    //if (is_down(input->active_controller->down)) acceleration_direction = -boat->direction;
    
    v2 acceleration = acceleration_direction * boat->acceleration_magnitude;
    if (boat->speed < boat->maximum_speed)
        boat->velocity += delta_velocity(acceleration, delta_time);
    
    boat->speed = magnitude(boat->velocity);
    if (boat->speed > 0.0f)
    {
        f32 angle_dir_to_velocity = angle_between(boat->direction, boat->velocity);
        v2 drag_force = -normalized(boat->velocity) * (pow(boat->velocity, 2)) * angle_dir_to_velocity * 10.0f;
        boat->velocity += delta_velocity(drag_force, delta_time);
    }
    
    boat->coords += delta_position(boat->velocity, delta_time);
}


function v2
game_to_screen_coords_2D(const v2 game_center, const v2 screen_center, const v2 game_coords)
{
    v2 diff = screen_center - game_center;
    return game_coords + diff;
}

//
// 3D
//

function Mesh
create_square_mesh(u32 u, u32 v)
{
    Mesh result = {};
    result.vertices_count = (u + 1) * (v + 1);
    result.vertices = (Vertex*)SDL_malloc(sizeof(Vertex) * result.vertices_count);
    
    f32 du = 2.0f / (f32)u;
    f32 dv = 2.0f / (f32)v;
    
    u32 vertex_count = 0;
    u32 s = 0, t = 0;
    for (u32 i = 0; i < (u + 1); i++, s += 2)
    {
        for (u32 j = 0; j < (v + 1); j++, t += 2)
        {
            v3 vertex_pos = { (f32(i) * du) - 1.0f, (f32)-1, (f32(j) * dv) - 1.0f };
            v2 tex_coords = { (f32)i / f32(u), 1.0f - (f32)j / f32(v) };
            Vertex vertex = { vertex_pos, {0, 1, 0}, tex_coords };
            result.vertices[vertex_count++] = vertex;
        }
    }
    
    result.indices_count = u * v * 6;
    result.indices = (u32*)SDL_malloc(sizeof(u32) * result.indices_count);
    
    u32 indices_count = 0;
    for (u32 i = 0; i < u; i++)
    {
        u32 p1 = i * (v + 1);
        u32 p2 = p1 + (v + 1);
        for (u32 j = 0; j < v; j++, p1++, p2++)
        {
            result.indices[indices_count++] = p1;
            result.indices[indices_count++] = p1 + 1;
            result.indices[indices_count++] = p2 + 1;
            
            result.indices[indices_count++] = p1;
            result.indices[indices_count++] = p2 + 1;
            result.indices[indices_count++] = p2;
        }
    }
    //init_mesh(&result);
    return result;
}

function Mesh
make_square_mesh_into_patches(Mesh *mesh, u32 u, u32 v)
{
    Mesh new_mesh = {};
    
    new_mesh.vertices_count = u * v * 4;
    new_mesh.vertices = (Vertex*)SDL_malloc(sizeof(Vertex) * new_mesh.vertices_count);
    
    u32 vertex_count = 0;
    for (u32 i = 0; i < u; i++)
    {
        u32 p1 = i * (v + 1);
        u32 p2 = p1 + (v + 1);
        for (u32 j = 0; j < v; j++, p1++, p2++)
        {
            new_mesh.vertices[vertex_count++] = mesh->vertices[p1];
            new_mesh.vertices[vertex_count++] = mesh->vertices[p1 + 1];
            new_mesh.vertices[vertex_count++] = mesh->vertices[p2];
            new_mesh.vertices[vertex_count++] = mesh->vertices[p2 + 1];
        }
    }
    
    new_mesh.indices_count = u * v * 4;
    new_mesh.indices = (u32*)SDL_malloc(sizeof(u32) * new_mesh.indices_count);
    
    u32 indices_count = 0;
    for (u32 i = 0; i < u * v * 4; i += 4)
    {
        new_mesh.indices[indices_count++] = i;
        new_mesh.indices[indices_count++] = i + 1;
        new_mesh.indices[indices_count++] = i + 2;
        new_mesh.indices[indices_count++] = i + 3;
    }
    
    init_mesh(&new_mesh);
    return new_mesh;
}

function void
draw_water(Assets *assets, Mesh mesh, r32 seconds, Wave *waves, u32 waves_count, Light_Source light, Camera camera)
{
    u32 active_shader = use_shader(find_shader(assets, "WATER"));
    
    v4 color = {30.0f/255.0f, 144.0f/255.0f, 255.0f/255.0f, 0.9};
    m4x4 model = create_transform_m4x4({0, 0, 0}, get_rotation(0, {1, 0, 0}), {10, 1, 10});
    
    glUniformMatrix4fv(glGetUniformLocation(active_shader, "model"      ), (GLsizei)1, false, (float*)&model);
    glUniform1f(       glGetUniformLocation(active_shader, "time"       ), seconds);
    
    glUniform3fv(      glGetUniformLocation(active_shader, "lightPos"   ), (GLsizei)1, (float*)&light.position);
    glUniform3fv(      glGetUniformLocation(active_shader, "lightColor" ), (GLsizei)1, (float*)&light.color);
    glUniform3fv(      glGetUniformLocation(active_shader, "cameraPos"  ), (GLsizei)1, (float*)&camera.position);
    glUniform4fv(      glGetUniformLocation(active_shader, "objectColor"), (GLsizei)1, (float*)&color);
    
    Bitmap *perlin = find_bitmap(assets, "NORMAL");
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, perlin->handle);
    
    glBindVertexArray(mesh.vao);
    glDrawElements(GL_PATCHES, mesh.indices_count, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

function v3
apply_wave(Wave wave, v3 position, f32 time)
{
    f32 k = 2.0f * PI / wave.wave_length;
    f32 c = sqrt(9.8f / k);
    v2 d = normalized(wave.direction);
    f32 f = k * (dot_product(d, { position.x, position.z }) - c * time);
    f32 a = wave.steepness / k;
    
    return v3{ 
        d.x * (a * cosf(f)),
        a * sinf(f),
        d.y * (a * cosf(f))
    };
}

function v3
update_boat_3D(Boat3D *boat, Wave *waves, u32 waves_count, f32 time)
{
    v3 result = boat->coords;
    const v3 position = boat->coords;
    for (u32 wave_index = 0; wave_index < waves_count; wave_index++)
    {
        result += apply_wave(waves[wave_index], position, time / 5.0);
    }
    return result;
}

//
// General
//

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

// block index is from glUniformBlockBinding or binding == #
function u32
init_uniform_buffer_object(u32 block_size, u32 block_index)
{
    u32 uniform_buffer_object;
    glGenBuffers(1, &uniform_buffer_object);
    
    glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer_object);
    glBufferData(GL_UNIFORM_BUFFER, block_size, NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    
    glBindBufferBase(GL_UNIFORM_BUFFER, block_index, uniform_buffer_object);
    
    return uniform_buffer_object;
}

function void*
init_game_data(Assets *assets)
{
    Game_Data *data = (Game_Data*)malloc(sizeof(Game_Data));
    *data = {};
    
    // 3D
    data->camera.position = { 0, 0, 2 };
    data->camera.up       = { 0, 1, 0 };
    data->camera.target   = { 0, 0, -2 };
    data->camera.yaw      = -90.0f;
    data->camera.fov      = 80.0f;
    
    data->light.position = { 5.0f, 5.0f, 10.0f };
    data->light.color = { 1.0f, 1.0f, 1.0f, 1.0f };
    
    Mesh temp_square_mesh = create_square_mesh(10, 10);
    Mesh temp_patch_mesh = make_square_mesh_into_patches(&temp_square_mesh, 10, 10);
    data->water = temp_patch_mesh;
    
    data->game_mode = MAIN_MENU;
    data->cube = get_cube_mesh();
    
    data->tree = load_obj("../assets/objs/tails/", "tails.obj");
    
    Shader *shader = find_shader(assets, "MATERIAL");
    u32 matrices_uniform_block_index = glGetUniformBlockIndex(shader->handle, "Matrices");
    glUniformBlockBinding(shader->handle, matrices_uniform_block_index, 0);
    
    shader = find_shader(assets, "WATER");
    matrices_uniform_block_index = glGetUniformBlockIndex(shader->handle, "Matrices");
    glUniformBlockBinding(shader->handle, matrices_uniform_block_index, 0);
    u32 waves_uniform_block_index = glGetUniformBlockIndex(shader->handle, "Wav");
    glUniformBlockBinding(shader->handle, waves_uniform_block_index, 1);
    
    data->matrices_ubo = init_uniform_buffer_object(2 * sizeof(m4x4), 0);
    data->wave_ubo = init_uniform_buffer_object(5 * sizeof(Wave), 1);
    
    data->waves[0] = get_wave({ 1.0, 0.0 }, 20.0f, 0.2f);
    data->waves[1] = get_wave({ 1, 1 }, 1.0f, 0.15f);
    data->waves[2] = get_wave({ 0.1, 0.1 }, 2.0f, 0.1f);
    data->waves[3] = get_wave({ 0.7, 0.9 }, 9.0f, 0.05f);
    data->waves[4] = get_wave({ 1.0, 1.0 }, 10.0f, 0.25f);
    
    glBindBuffer(GL_UNIFORM_BUFFER, data->wave_ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(Wave) * 5, (void*)&data->waves);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    
    data->boat3D.coords = { 0, -1.5f, 0 };
    
    // 2D
    init_boat(&data->boat);
    //data->tree = load_obj("../assets/objs/test.obj");
    
    return (void*)data;
}

// returns game mode
function s32
draw_main_menu(Assets *assets, v2 window_dim, b32 select, s32 active)
{
    /*
    Menu main_menu = {};
    main_menu.font = find_font(assets, "CASLON");
    
    main_menu.button_style.default_back_color = { 100, 255, 0, 1 };
    main_menu.button_style.active_back_color  = { 0, 255, 100, 1 };
    main_menu.button_style.default_text_color = { 0, 100, 0, 1 };
    main_menu.button_style.active_text_color  = { 0, 100, 0, 1 };
    
    u32 index = 0;
    
    Rect window_rect = {};
    window_rect. coords = { 0, 0 };
    window_rect.dim = cv2(app->window.dim);
    Rect bounds = get_centered_rect(window_rect, 0.5f, 0.5f);
    main_menu.button_style.dim = { bounds.dim.x, bounds.dim.y / 3.0f };
    draw_rect(bounds.coords, 0, bounds.dim, { 0, 0, 0, 0.2f} );
    main_menu.coords = bounds.coords;
    
    if (menu_button(&main_menu, "2D", index++, active, select))
    {
        data->game_mode = IN_GAME_2D;
        data->active = 0;
    }
    
    if (menu_button(&main_menu, "3D", index++, active, select))
    {
        data->game_mode = IN_GAME_3D;
        app->input.relative_mouse_mode.set(true);
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        data->active = 0;
    }
    
    if (menu_button(&main_menu, "Quit", index++, active, select))
    {
        return true;
    }
*/
}

// return 0 if 
function s32
draw_pause_menu(Assets *assets, v2 window_dim, b32 select, s32 active)
{
    Menu pause_menu = {};
    pause_menu.font = find_font(assets, "CASLON");
    
    pause_menu.button_style.default_back_color = { 100, 255, 0, 1 };
    pause_menu.button_style.active_back_color  = { 0, 255, 100, 1 };
    pause_menu.button_style.default_text_color = { 0, 100, 0, 1 };
    pause_menu.button_style.active_text_color  = { 0, 100, 0, 1 };
    
    u32 index = 0;
    
    Rect window_rect = {};
    window_rect. coords = { 0, 0 };
    window_rect.dim = window_dim;
    Rect bounds = get_centered_rect(window_rect, 0.9f, 0.5f);
    pause_menu.button_style.dim = { bounds.dim.x, bounds.dim.y / 2.0f };
    draw_rect(bounds.coords, 0, bounds.dim, { 0, 0, 0, 0.2f} );
    pause_menu.coords = bounds.coords;
    
    if (menu_button(&pause_menu, "Unpause", index++, active, select))   return 1;
    if (menu_button(&pause_menu, "Main Menu", index++, active, select)) return 2;
    return 0;
}


// returns true if the application should quit
function b8
update(Application *app)
{
    Game_Data *data = (Game_Data*)app->data;
    Controller *controller = app->input.active_controller;
    Controller *menu_controller = app->input.active_controller;
    
    if (app->window.update_matrices)
    {
        app->matrices.perspective_matrix = perspective_projection(data->camera.fov, app->window.aspect_ratio, 0.01f, 1000.0f);
        app->matrices.orthographic_matrix = orthographic_projection(0.0f, (r32)app->window.dim.width, (r32)app->window.dim.height, 0.0f, -3.0f, 3.0f);
        app->window.update_matrices = false;
    }
    
    //Update
    switch (data->game_mode)
    {
        case MAIN_MENU:
        {
            menu_update_active(&data->active, 0, 2, menu_controller->backward, menu_controller->forward);
        } break;
        
        case IN_GAME_2D:
        {
            update_boat(&data->boat, &app->input, app->time.frame_time_s);
            
            if (data->paused)
            {
                menu_update_active(&data->active, 0, 1, menu_controller->backward, menu_controller->forward);
            }
            
            if (on_down(controller->pause)) data->paused = !data->paused;
            
        } break;
        
        case IN_GAME_3D:
        {
            if (!data->paused)
            {
                f32 mouse_m_per_s = 100.0f;
                f32 mouse_move_speed = mouse_m_per_s * app->time.frame_time_s;
                update_camera_with_mouse(&data->camera, controller->mouse, {mouse_move_speed, mouse_move_speed});
                
                f32 m_per_s = 5.0f; 
                f32 move_speed = m_per_s * app->time.frame_time_s;
                update_camera_with_keys(&data->camera,
                                        {move_speed, move_speed, move_speed},
                                        controller->forward,
                                        controller->backward,
                                        controller->left,
                                        controller->right,
                                        controller->up,
                                        controller->down);
                
                //update_boat_3D(&data->boat3D, data->waves, 5, app->time.run_time_s);
            }
            
            if (on_down(controller->pause)) 
            {
                data->paused = !data->paused;
                if (data->paused) app->input.relative_mouse_mode.set(false);
                else              app->input.relative_mouse_mode.set(true);
            }
            
            if (data->paused)
            {
                menu_update_active(&data->active, 0, 1, menu_controller->backward, menu_controller->forward);
            }
        } break;
    }
    
    if (on_down(controller->wire_frame))
    {
        data->wire_frame = !data->wire_frame;
        if (data->wire_frame) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else                  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }
    
    if (on_down(controller->reload_shaders))
    {
        Shader *shader = find_shader(&app->assets, "WATER");
        load_shader(shader);
        compile_shader(shader);
        
        u32 matrices_uniform_block_index = glGetUniformBlockIndex(shader->handle, "Matrices");
        glUniformBlockBinding(shader->handle, matrices_uniform_block_index, 0);
        u32 waves_uniform_block_index = glGetUniformBlockIndex(shader->handle, "Wav");
        glUniformBlockBinding(shader->handle, waves_uniform_block_index, 1);
    }
    
    
    // DRAW
    u32 gl_clear_flags = 
        GL_COLOR_BUFFER_BIT | 
        GL_DEPTH_BUFFER_BIT | 
        GL_STENCIL_BUFFER_BIT;
    
    glClear(gl_clear_flags);
    
    switch (data->game_mode)
    {
        case MAIN_MENU:
        {
            orthographic(data->matrices_ubo, &app->matrices);
            
            Menu main_menu = {};
            main_menu.font = find_font(&app->assets, "CASLON");
            
            main_menu.button_style.default_back_color = { 100, 255, 0, 1 };
            main_menu.button_style.active_back_color  = { 0, 255, 100, 1 };
            main_menu.button_style.default_text_color = { 0, 100, 0, 1 };
            main_menu.button_style.active_text_color  = { 0, 100, 0, 1 };
            
            b32 select = on_down(menu_controller->select);
            u32 index = 0;
            
            Rect window_rect = {};
            window_rect. coords = { 0, 0 };
            window_rect.dim = cv2(app->window.dim);
            Rect bounds = get_centered_rect(window_rect, 0.5f, 0.5f);
            main_menu.button_style.dim = { bounds.dim.x, bounds.dim.y / 3.0f };
            draw_rect(bounds.coords, 0, bounds.dim, { 0, 0, 0, 0.2f} );
            main_menu.coords = bounds.coords;
            
            if (menu_button(&main_menu, "2D", index++, data->active, select))
            {
                data->game_mode = IN_GAME_2D;
                data->active = 0;
            }
            
            if (menu_button(&main_menu, "3D", index++, data->active, select))
            {
                data->game_mode = IN_GAME_3D;
                app->input.relative_mouse_mode.set(true);
                glEnable(GL_DEPTH_TEST);
                glEnable(GL_CULL_FACE);
                data->active = 0;
            }
            
            if (menu_button(&main_menu, "Quit", index++, data->active, select))
            {
                return true;
            }
            
        } break;
        
        case IN_GAME_2D:
        {
            orthographic(data->matrices_ubo, &app->matrices);
            
            Rect rect = {};
            rect.dim = { 100, 100 };
            v2 center = (cv2(app->window.dim) / 2.0f);
            center_on(&rect, center);
            
            draw_rect( { 0, 0 }, 0.0f, cv2(app->window.dim), { 0.0f, 100.0f, 255.0f, 1.0f } );
            draw_circle( game_to_screen_coords_2D(data->boat.coords, center, { 0, 0 } ), 0.0f, 100.0f, { 255.0f, 0.0f, 0.0f, 1.0f } );
            
            Bitmap *boat = find_bitmap(&app->assets, "BOAT");
            draw_rect(rect.coords, v2_to_angle(data->boat.direction), rect.dim, boat);
            
            if (data->paused) 
            {
                draw_rect( { 0, 0 }, 0, cv2(app->window.dim), { 0, 0, 0, 0.5f} );
                
                s32 pause = draw_pause_menu(&app->assets, cv2(app->window.dim), on_down(menu_controller->select), data->active);
                if      (pause == 1) 
                    data->paused = false;
                else if (pause == 2) { 
                    data->game_mode = MAIN_MENU; 
                    data->paused = false; 
                    data->active = 0; 
                }
            }
            
        } break;
        
        case IN_GAME_3D:
        {
            app->matrices.view_matrix = get_view(data->camera);
            perspective(data->matrices_ubo, &app->matrices);
            
            glEnable(GL_DEPTH_TEST);
            glEnable(GL_CULL_FACE);
            
            draw_water(&app->assets, data->water, app->time.run_time_s, data->waves, 5, data->light, data->camera);
            draw_model(&app->assets, &data->tree, data->light, data->camera);
            draw_cube(data->light.position, 0, { 1, 1, 1 }, data->light.color * 255.0f);
            draw_cube(update_boat_3D(&data->boat3D, data->waves, 5, app->time.run_time_s), 0, { 1, 1, 1 }, { 255, 0, 255, 255 });
            
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
            
            orthographic(data->matrices_ubo, &app->matrices);
            
            Font *caslon = find_font(&app->assets, "CASLON");
            draw_string(caslon, ftos(app->time.frames_per_s), { 100, 100 }, 50, { 255, 150, 0, 1 });
            
            if (data->paused) 
            {
                draw_rect( { 0, 0 }, 0, cv2(app->window.dim), { 0, 0, 0, 0.5f} );
                
                s32 pause = draw_pause_menu(&app->assets, cv2(app->window.dim), on_down(menu_controller->select), data->active);
                if      (pause == 1) 
                    data->paused = false;
                else if (pause == 2) { 
                    data->game_mode = MAIN_MENU; 
                    data->paused = false; 
                    app->input.relative_mouse_mode.set(true);
                    //glDisable(GL_DEPTH_TEST);
                    data->active = 0;
                }
            }
        } break;
    }
    
    return false;
}

#include "application.cpp"