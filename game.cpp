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
#include "game.h"
#include "application.h"

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

function r32
magnitude(v2 vector)
{
    return sqrtf((vector.x * vector.x) + (vector.y * vector.y));
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
    boat->water_acceleration_magnitude   = boat->rudder_force / boat->mass;
}

function void
update_boat(Boat *boat, Input *input, r32 delta_time)
{
    v2 rotation_direction = {};
    //if (is_down(input->active_controller->right)) acceleration_direction =  boat->direction;
    //if (is_down(input->active_controller->left)) acceleration_direction = -boat->direction;
    
    if (is_down(input->active_controller->right))
        rotate_v2(&boat->direction, 0.05f * DEG2RAD);
    if (is_down(input->active_controller->left))
        rotate_v2(&boat->direction, -0.05f * DEG2RAD);
    
    v2 acceleration_direction = {};
    if (is_down(input->active_controller->up))   acceleration_direction =  boat->direction;
    if (is_down(input->active_controller->down)) acceleration_direction = -boat->direction;
    
    v2 acceleration = acceleration_direction * boat->acceleration_magnitude;
    if (boat->speed < boat->maximum_speed)
        boat->velocity += delta_velocity(acceleration, delta_time);
    
    v2 water_acceleration = -normalized(boat->velocity) * boat->water_acceleration_magnitude;
    //log(boat->velocity);
    //log("%f", boat->speed);
    if (boat->speed > 0)
        boat->velocity += delta_velocity(water_acceleration, delta_time);
    
    boat->speed = magnitude(boat->velocity);
    
    boat->coords += delta_position(boat->velocity, delta_time);
    
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
            v2 tex_coords = { (f32)i, (f32)j };
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
    
    init_mesh(&new_mesh);
    return new_mesh;
}

function void
draw_water(Assets *assets, Mesh mesh, r32 seconds,
           m4x4 projection_matrix, m4x4 view_matrix, Light_Source light, Camera camera)
{
    u32 active_shader = use_shader(find_shader(assets, "WATER"));
    v4 color = {30.0f/255.0f, 144.0f/255.0f, 255.0f/255.0f, 0.9};
    m4x4 model = create_transform_m4x4({0, 0, 0}, get_rotation(0, {1, 0, 0}), {20, 1, 20});
    
    glUniform4fv(      glGetUniformLocation(active_shader, "objectColor"), (GLsizei)1, (float*)&color);
    glUniformMatrix4fv(glGetUniformLocation(active_shader, "model"      ), (GLsizei)1, false, (float*)&model);
    glUniformMatrix4fv(glGetUniformLocation(active_shader, "projection" ), (GLsizei)1, false, (float*)&projection_matrix);
    glUniformMatrix4fv(glGetUniformLocation(active_shader, "view"       ), (GLsizei)1, false, (float*)&view_matrix);
    glUniform1f(       glGetUniformLocation(active_shader, "time"       ), seconds);
    glUniform3fv(      glGetUniformLocation(active_shader, "lightPos"   ), (GLsizei)1, (float*)&light.position);
    glUniform3fv(      glGetUniformLocation(active_shader, "lightColor" ), (GLsizei)1, (float*)&light.color);
    glUniform3fv(      glGetUniformLocation(active_shader, "cameraPos"  ), (GLsizei)1, (float*)&camera.position);
    
    glBindVertexArray(mesh.vao);
    glDrawArrays(GL_PATCHES, 0, mesh.vertices_count);
    glBindVertexArray(0);
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
    set(&keyboard->wire_frame,    SDLK_t);
    
    input->num_of_controllers = 1;
}

function void*
init_game_data(Assets *assets)
{
    Game_Data *data = (Game_Data*)malloc(sizeof(Game_Data));
    *data = {};
    
    data->camera.position = { 0, 0, 2 };
    data->camera.up       = { 0, 1, 0 };
    data->camera.target   = { 0, 0, -2 };
    data->camera.yaw      = -90.0f;
    
    data->light.position = { 0.0f, 5.0f, 0.0f };
    data->light.color = { 1.0f, 1.0f, 1.0f, 1.0f };
    
    Mesh temp_square_mesh = create_square_mesh(10, 10);
    Mesh temp_patch_mesh = make_square_mesh_into_patches(&temp_square_mesh, 10, 10);
    data->water = temp_patch_mesh;
    
    data->game_mode = MAIN_MENU;
    
    init_boat(&data->boat);
    
    return (void*)data;
}

// return 0 if 
function s32
draw_pause_menu(Assets *assets, v2 window_dim, b32 select, s32 active)
{
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    
    Menu pause_menu = {};
    pause_menu.font = find_font(assets, "CASLON");
    
    pause_menu.button_style.default_back_color = { 100, 255, 0, 1 };
    pause_menu.button_style.active_back_color  = { 0, 255, 0, 1 };
    pause_menu.button_style.default_text_color = { 0, 100, 0, 1 };
    pause_menu.button_style.active_text_color  = { 0, 100, 0, 1 };
    
    u32 index = 0;
    
    Rect window_rect = {};
    window_rect. coords = { 0, 0 };
    window_rect.dim = window_dim;
    Rect bounds = get_centered_rect(window_rect, 0.5f, 0.5f);
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
                update_camera_with_mouse(&data->camera, controller->mouse);
                f32 m_per_s = 3.0f;
                f32 move_speed = m_per_s * app->time.frame_time_s;
                update_camera_with_keys(&data->camera,
                                        {move_speed, move_speed, move_speed},
                                        controller->forward,
                                        controller->backward,
                                        controller->left,
                                        controller->right,
                                        controller->up,
                                        controller->down);
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
            Menu main_menu = {};
            main_menu.font = find_font(&app->assets, "CASLON");
            
            main_menu.button_style.default_back_color = { 100, 255, 0, 1 };
            main_menu.button_style.active_back_color  = { 0, 255, 0, 1 };
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
            
            draw_circle({ 100, 100 }, 0, 500, {0, 0, 255, 1} );
        } break;
        
        case IN_GAME_2D:
        {
            draw_rect( { 0, 0 }, 0.0f, cv2(app->window.dim), { 0.0f, 100.0f, 255.0f, 1.0f } );
            draw_circle( { 0, 0 }, 0.0f, 100.0f, { 255.0f, 0.0f, 0.0f, 1.0f } );
            
            Rect rect = {};
            rect.dim = { 100, 100 };
            center_on(&rect, data->boat.coords);
            
            Bitmap *jeff = find_bitmap(&app->assets, "JEFF");
            draw_rect(rect.coords, v2_to_angle(data->boat.direction), rect.dim, jeff);
            
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
            r32 aspect_ratio = (r32)app->window.dim.width / (r32)app->window.dim.height;
            m4x4 perspective_matrix = perspective_projection(90.0f, aspect_ratio, 0.01f, 1000.0f);
            m4x4 orthographic_matrix = orthographic_projection(0.0f, (r32)app->window.dim.width, (r32)app->window.dim.height,
                                                               0.0f, -3.0f, 3.0f);
            m4x4 view_matrix = get_view(data->camera);
            
            draw_water(&app->assets, data->water, app->time.run_time_s,
                       perspective_matrix, view_matrix, data->light, data->camera);
            
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
                    glDisable(GL_DEPTH_TEST);
                    glDisable(GL_CULL_FACE);
                    data->active = 0;
                }
            }
        } break;
    }
    
    return false;
}

#include "application.cpp"