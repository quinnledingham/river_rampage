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
#include "rect.h"
#include "application.h"

#include "assets.cpp"
#include "rect.cpp"
#include "application.cpp"

struct Vector
{
    r32 magitude;
    v2 direction;
};

struct Boat
{
    v2 coords;
    r32 rotation_speed;
    Vector vector;
};

struct Game_Data
{
    v2 jeff_coords;
    
    Boat boat;
};

function r32
vector_to_angle(Vector vector)
{
    r32 x = vector.direction.x;
    r32 y = vector.direction.y;
    r32 angle = atanf(y / x);
    
    if ((x < 0 && y > 0) || (x < 0 && y < 0)) angle += PI;
    
    return angle;
}

function void
rotate_vector(Vector *vector, r32 angle)
{
    r32 current_angle = vector_to_angle(*vector);
    r32 new_angle = current_angle + angle;
    
    vector->direction.x = 1.0f * cosf(new_angle);
    vector->direction.y = 1.0f * sinf(new_angle);
}

function void*
init_data(Assets *assets)
{
    Game_Data *data = (Game_Data*)malloc(sizeof(Game_Data));
    *data = {};
    
    data->boat.coords = { 200, 200 };
    data->boat.rotation_speed = 0.01f;
    data->boat.vector.magitude = 0.01f;
    data->boat.vector.direction = { 1, 0 };
    
    return (void*)data;
}

function b32
update(Application *app)
{
    Window    *window = (Window*)    &app->window;
    Time      *time   = (Time*)      &app->time;
    Input     *input  = (Input*)     &app->input;
    Assets    *assets = (Assets*)    &app->assets;
    Game_Data *data   = (Game_Data*) app->data;
    
    Boat *boat = &data->boat;
    
    if (is_down(input->active_controller->left) || is_down(input->active_controller->right)) 
    {
        r32 deg = boat->rotation_speed;
        if (is_down(input->active_controller->right)) deg = -deg;
        r32 rad = deg * DEG2RAD;
        rotate_vector(&boat->vector, rad);
    }
    
    v2 dir = boat->vector.direction;
    dir.y = -dir.y;
    if (is_down(input->active_controller->up))    boat->coords += dir * boat->vector.magitude;
    if (is_down(input->active_controller->down))  boat->coords += dir * -boat->vector.magitude;
    
    glClear(window->gl_clear_flags);
    
    Rect rect = {};
    rect.dim = { 100, 100 };
    center_on(&rect, boat->coords);
    
    Bitmap *jeff = find_bitmap(assets, "jeff");
    draw_rect(rect.coords, vector_to_angle(boat->vector), rect.dim, jeff);
    
    return false;
}