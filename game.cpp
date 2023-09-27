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
#include "game.h"
#include "application.h"

#include "assets.cpp"

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
    glUniformMatrix4fv(glGetUniformLocation(active_shader, "model"), (GLsizei)1, false, (float*)&model);
    glUniformMatrix4fv(glGetUniformLocation(active_shader, "projection"), (GLsizei)1, false, (float*)&projection_matrix);
    glUniformMatrix4fv(glGetUniformLocation(active_shader, "view"), (GLsizei)1, false, (float*)&view_matrix);
    glUniform1f(       glGetUniformLocation(active_shader, "time"), seconds);
    glUniform3fv(      glGetUniformLocation(active_shader, "lightPos"), (GLsizei)1, (float*)&light.position);
    glUniform3fv(      glGetUniformLocation(active_shader, "lightColor"), (GLsizei)1, (float*)&light.color);
    glUniform3fv(      glGetUniformLocation(active_shader, "cameraPos"), (GLsizei)1, (float*)&camera.position);
    
    glBindVertexArray(mesh.vao);
    glDrawArrays(GL_PATCHES, 0, mesh.vertices_count);
    glBindVertexArray(0);
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
    
    return (void*)data;
}

// delta_mouse is a relative mouse movement amount
// as opposed to the screen coords of the mouse
function void
update_camera_with_mouse(Camera *camera, v2s delta_mouse)
{
    camera->yaw   += (f32)delta_mouse.x * 0.1f;
    camera->pitch -= (f32)delta_mouse.y * 0.1f;
    
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
update_camera_with_keys(Camera *camera,
                        v3 move_vector,
                        Button forward,
                        Button backward,
                        Button left,
                        Button right,
                        Button up,
                        Button down)
{
    if (is_down(forward))  camera->position += camera->target * move_vector;
    if (is_down(backward)) camera->position -= camera->target * move_vector;
    if (is_down(left))     camera->position -= normalized(cross_product(camera->target, camera->up)) * move_vector;
    if (is_down(right))    camera->position += normalized(cross_product(camera->target, camera->up)) * move_vector;
    if (is_down(up))       camera->position.y += move_vector.y;
    if (is_down(down))     camera->position.y -= move_vector.y;
}

function void
update(Application *app)
{
    Game_Data *data = (Game_Data*)app->data;
    Controller *controller = app->input.active_controller;
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
    
    // DRAW
    u32 gl_clear_flags = 
        GL_COLOR_BUFFER_BIT | 
        GL_DEPTH_BUFFER_BIT | 
        GL_STENCIL_BUFFER_BIT;
    
    glClear(gl_clear_flags);
    
    r32 aspect_ratio = (r32)app->window.dim.width / (r32)app->window.dim.height;
    m4x4 perspective_matrix = perspective_projection(90.0f, aspect_ratio, 0.01f, 1000.0f);
    m4x4 orthographic_matrix = orthographic_projection(0.0f, (r32)app->window.dim.width, (r32)app->window.dim.height,
                                                       0.0f, -3.0f, 3.0f);
    m4x4 view_matrix = get_view(data->camera);
    
    draw_water(&app->assets, data->water, app->time.run_time_s,
               perspective_matrix, view_matrix, data->light, data->camera);
}

#include "application.cpp"