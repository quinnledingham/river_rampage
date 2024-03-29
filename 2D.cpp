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

// converts boat velocity to pixel change on screen
function v2 
delta_position(v2 vector, r32 time_s)
{
    vector.y = -vector.y; // 2D screen coords +y going down screen
    return (vector * time_s) * cv2(renderer_window_dim); // pixels per meter
}

function void
init_boat(Boat *boat)
{
    boat->coords            = { 100, 100 };
    
    boat->mass              = 100.0f; 
    boat->engine_force      = 15.0f;
    boat->rudder_force      = 6.0f;
    boat->water_line_length = 100.0f;
    
    boat->direction = { 1, 0 };
    
    r32 speed_feet_per_s = 1.34f * sqrtf(boat->water_line_length);
    boat->maximum_speed  = speed_feet_per_s * 0.3048f; // ft to m
    
    boat->acceleration_magnitude       = boat->engine_force / boat->mass;
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
    r32 rotation_speed = 0.05f;
    r32 rotation_radians = DEG2RAD * rotation_speed;
    
    if (is_down(input->active_controller->left))  rotate_v2(&boat->direction,  rotation_radians);
    if (is_down(input->active_controller->right)) rotate_v2(&boat->direction, -rotation_radians);
    
    v2 acceleration_direction = {};
    if (is_down(input->active_controller->up)) 
        acceleration_direction = boat->direction;
    
    v2 acceleration = acceleration_direction * boat->acceleration_magnitude;
    if (boat->speed < boat->maximum_speed)
        boat->velocity += acceleration * delta_time;
    
    boat->speed = magnitude(boat->velocity);
    if (boat->speed > 0.0f) {
        f32 angle_dir_to_velocity = angle_between(boat->direction, boat->velocity);
        v2 drag_force = -normalized(boat->velocity) * pow(boat->velocity, 2) * angle_dir_to_velocity * 10.0f;
        boat->velocity += drag_force * delta_time;
    }
    
    boat->coords += delta_position(boat->velocity, delta_time);
}


function v2
game_to_screen_coords_2D(const v2 game_center, const v2 screen_center, const v2 game_coords)
{
    v2 diff = screen_center - game_center;
    return game_coords + diff;
}

function void
update_game_2D(Game *game, Game_2D *data, Input *input, const Time time)
{
    Controller *controller = input->active_controller;
    Controller *menu_controller = input->active_controller;

    update_boat(&data->boat, input, (r32)time.frame_time_s);
    
    if (game->paused) {
        menu_update_active(&game->active, 0, 1, menu_controller->backward, menu_controller->forward);
    }
    
    if (on_down(controller->pause)) 
        game->paused = !game->paused;
}

function void
draw_game_2D(Game *game, Game_2D *data, Matrices *matrices, Assets *assets, Input *input, v2s window_dim)
{
    Controller *menu_controller = input->active_controller;

    orthographic(game->ubos.matrices, matrices);
    
    Rect rect = {};
    rect.dim = { 100, 100 };
    v2 center = (cv2(window_dim) / 2.0f);
    center_on(&rect, center);
    
    draw_rect( { 0, 0 }, 0.0f, cv2(window_dim), { 0.0f, 100.0f, 255.0f, 1.0f } );
    draw_circle( game_to_screen_coords_2D(data->boat.coords, center, { 0, 0 } ), 0.0f, 100.0f, { 255.0f, 0.0f, 0.0f, 1.0f } );
    
    Bitmap *boat = find_bitmap(assets, "BOAT");
    draw_rect(rect.coords, v2_to_angle(data->boat.direction), rect.dim, boat);
    
    if (game->paused) 
    {
        draw_rect( { 0, 0 }, 0, cv2(window_dim), { 0, 0, 0, 0.5f} );
        
        s32 pause = draw_pause_menu(assets, cv2(window_dim), on_down(menu_controller->select), game->active);
        switch(pause) {
            case 1: game->paused = false;   break;
            case 2: game->mode = MAIN_MENU; break;
        }
    }
}