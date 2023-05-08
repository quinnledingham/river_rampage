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
#include "application.h"

#include "assets.cpp"
#include "shapes.cpp"
#include "application.cpp"

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

struct Boat
{
    v2 coords;
    
    r32 mass;
    r32 engine_force;
    r32 rudder_force;
    r32 water_line_length;
    
    r32 speed;
    r32 maximum_speed;
    r32 rotation_speed;
    v2 velocity; // the direciton and mag of movement
    
    v2 direction; // the way the ship is pointing 
    r32 acceleration_magnitude; // always accelerates in the direction of the boat
    r32 water_acceleration_magnitude;
    
    
};

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
    
    init_boat(&data->boat);
    
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
    update_boat(boat, input, time->frame_time_s);
    /*
    {
        r32 acceleration = boat->rudder_force / boat->mass;
        r32 water_acceleration = data->water_force / boat->mass;
        
        if (boat->vector.magnitude > 0)
        {
            if (is_down(input->active_controller->right)) if (boat->rotation_speed > -boat->vector.magnitude) boat->rotation_speed -= acceleration * time->frame_time_s;
            if (is_down(input->active_controller->left)) if (boat->rotation_speed < boat->vector.magnitude) boat->rotation_speed += acceleration * time->frame_time_s;
        }
        else
        {
            if (is_down(input->active_controller->right)) if (boat->rotation_speed < -boat->vector.magnitude) boat->rotation_speed -= acceleration * time->frame_time_s;
            if (is_down(input->active_controller->left)) if (boat->rotation_speed > boat->vector.magnitude) boat->rotation_speed += acceleration * time->frame_time_s;
        }
        //if (is_down(input->active_controller->right)) boat->rotation_speed = boat->vector.magnitude - (acceleration * time->frame_time_s);
        //if (is_down(input->active_controller->left))  boat->rotation_speed = boat->vector.magnitude + (acceleration * time->frame_time_s);
        
        if (boat->rotation_speed > 0) boat->rotation_speed -= (water_acceleration * time->frame_time_s);
        if (boat->rotation_speed < 0) boat->rotation_speed += (water_acceleration * time->frame_time_s);
        
        rotate_vector(&boat->vector, boat->rotation_speed * DEG2RAD);
    }
    */
    
    //log(boat->vector);
    //log(boat->coords);
    
    glClear(window->gl_clear_flags);
    
    draw_rect( { 0, 0 }, 0.0f, cv2(window->dim), { 0.0f, 100.0f, 255.0f, 1.0f } );
    draw_circle( { 0, 0 }, 0.0f, { 100, 100 }, { 255.0f, 0.0f, 0.0f, 1.0f } );
    
    Rect rect = {};
    rect.dim = { 100, 100 };
    center_on(&rect, boat->coords);
    
    Bitmap *jeff = find_bitmap(assets, "jeff");
    draw_rect(rect.coords, v2_to_angle(boat->direction), rect.dim, jeff);
    
    return false;
}
