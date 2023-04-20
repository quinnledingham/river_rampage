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
    r32 magnitude;
    v2 direction;
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

function v2
get_screen_displacement(Vector vector)
{
    vector.direction.y = -vector.direction.y;
    return vector.direction * vector.magnitude;
}

function void
log(Vector vector)
{
    log("magnitude: %f, direction: %f, %f", vector.magnitude, vector.direction); 
}

struct Boat
{
    v2 coords;
    r32 rotation_speed;
    r32 rudder_force;
    
    Vector vector;
    
    r32 mass;
    r32 engine_force;
};

struct Game_Data
{
    Boat boat;
    r32 water_force;
};

function void*
init_data(Assets *assets)
{
    Game_Data *data = (Game_Data*)malloc(sizeof(Game_Data));
    *data = {};
    
    data->boat.coords = { 200, 200 };
    data->boat.mass = 100.0f;
    data->boat.engine_force = 2.0f;
    data->boat.rudder_force = 2.0f;
    data->boat.rotation_speed = 0.0f;
    data->boat.vector.direction = { 1, 0 };
    
    data->water_force = 0.9f;
    
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
    
    {
        r32 acceleration = boat->rudder_force / boat->mass;
        r32 water_acceleration = data->water_force / boat->mass;
        
        if (boat->vector.magnitude > 0)
        {
            if (is_down(input->active_controller->right)) if (boat->rotation_speed > -boat->vector.magnitude/2.0f) boat->rotation_speed -= acceleration * time->frame_time_s;
            if (is_down(input->active_controller->left)) if (boat->rotation_speed < boat->vector.magnitude/2.0f) boat->rotation_speed += acceleration * time->frame_time_s;
        }
        else
        {
            if (is_down(input->active_controller->right)) if (boat->rotation_speed < -boat->vector.magnitude/2.0f) boat->rotation_speed -= acceleration * time->frame_time_s;
            if (is_down(input->active_controller->left)) if (boat->rotation_speed > boat->vector.magnitude/2.0f) boat->rotation_speed += acceleration * time->frame_time_s;
        }
        //if (is_down(input->active_controller->right)) boat->rotation_speed = boat->vector.magnitude - (acceleration * time->frame_time_s);
        //if (is_down(input->active_controller->left))  boat->rotation_speed = boat->vector.magnitude + (acceleration * time->frame_time_s);
        
        if (boat->rotation_speed > 0) boat->rotation_speed -= (water_acceleration * time->frame_time_s);
        if (boat->rotation_speed < 0) boat->rotation_speed += (water_acceleration * time->frame_time_s);
        
        rotate_vector(&boat->vector, boat->rotation_speed * DEG2RAD);
    }
    {
        r32 acceleration = boat->engine_force / boat->mass;
        r32 water_acceleration = data->water_force / boat->mass;
        
        if (is_down(input->active_controller->up)) if (boat->vector.magnitude < 0.02f) boat->vector.magnitude += (acceleration * time->frame_time_s);
        if (is_down(input->active_controller->down)) if (boat->vector.magnitude > -0.02f) boat->vector.magnitude -= (acceleration * time->frame_time_s);
        
        if (boat->vector.magnitude > 0) boat->vector.magnitude -= (water_acceleration * time->frame_time_s);
        if (boat->vector.magnitude < 0) boat->vector.magnitude += (water_acceleration * time->frame_time_s);
        
        boat->coords += get_screen_displacement(boat->vector);
    }
    //log(boat->vector);
    
    glClear(window->gl_clear_flags);
    
    Rect rect = {};
    rect.dim = { 100, 100 };
    center_on(&rect, boat->coords);
    
    Bitmap *jeff = find_bitmap(assets, "jeff");
    draw_rect(rect.coords, vector_to_angle(boat->vector), rect.dim, jeff);
    
    return false;
}
