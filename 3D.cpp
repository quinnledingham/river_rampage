internal void
update_camera_target(Camera *camera) {
    v3 camera_direction = 
    {
        cosf(DEG2RAD * camera->yaw) * cosf(DEG2RAD * camera->pitch),
        sinf(DEG2RAD * camera->pitch),
        sinf(DEG2RAD * camera->yaw) * cosf(DEG2RAD * camera->pitch)
    };
    camera->target = normalized(camera_direction);
}

// delta_mouse is a relative mouse movement amount
// as opposed to the screen coords of the mouse
function void
update_camera_with_mouse(Camera *camera, v2s delta_mouse, v2 move_speed)
{    
    camera->yaw   += (f32)delta_mouse.x * move_speed.x;
    camera->pitch -= (f32)delta_mouse.y * move_speed.y;
    
    // doesnt break withou this - just so it does not keep getting higher and higher
    r32 max_yaw = 360.0f;
    if (camera->yaw > max_yaw) camera->yaw = 0;
    if (camera->yaw < 0)       camera->yaw = max_yaw;

    // breaks with out this check
    r32 max_pitch = 89.0f;
    if (camera->pitch >  max_pitch) camera->pitch =  max_pitch;
    if (camera->pitch < -max_pitch) camera->pitch = -max_pitch;
}

function void
update_camera_with_keys(Camera *camera, v3 target, v3 up_v, v3 magnitude,
                        Button forward, Button backward,
                        Button left, Button right,
                        Button up, Button down)
{
    if (is_down(forward))  camera->position   += target * magnitude;
    if (is_down(backward)) camera->position   -= target * magnitude;
    if (is_down(left))     camera->position   -= normalized(cross_product(target, up_v)) * magnitude;
    if (is_down(right))    camera->position   += normalized(cross_product(target, up_v)) * magnitude;
    if (is_down(up))       camera->position.y += magnitude.y;
    if (is_down(down))     camera->position.y -= magnitude.y;
}

internal void
align_camera_with_boat(Camera *camera, Boat3D *boat) {
    camera->position.x = boat->coords.x;
    camera->position.z = boat->coords.z;
    camera->position.y = 40.0f;

    camera->fov      = 70.0f;
    camera->yaw      = 0.0f;
    camera->pitch    = -85.0f;
}

function Mesh
create_square_mesh(u32 u, u32 v, b32 rand)
{
    Mesh result = {};
    result.vertices_count = (u + 1) * (v + 1);
    result.vertices = (Vertex*)platform_malloc(sizeof(Vertex) * result.vertices_count);
    
    f32 du = 2.0f / (f32)u;
    f32 dv = 2.0f / (f32)v;
    
    u32 vertex_count = 0;
    u32 s = 0, t = 0;
    for (u32 i = 0; i < (u + 1); i++, s += 2) {
        for (u32 j = 0; j < (v + 1); j++, t += 2) {
            s32 y = -1;
            if (rand)
                y = -5;
                //y = random(-10, 10);
            v3 vertex_pos = { (f32(i) * du) - 1.0f, (f32)y, (f32(j) * dv) - 1.0f };
            v2 tex_coords = { (f32)i / f32(u), 1.0f - (f32)j / f32(v) };
            Vertex vertex = { vertex_pos, {0, 1, 0}, tex_coords };
            result.vertices[vertex_count++] = vertex;
        }
    }
    
    result.indices_count = u * v * 6;
    result.indices = (u32*)platform_malloc(sizeof(u32) * result.indices_count);
    
    u32 indices_count = 0;
    for (u32 i = 0; i < u; i++) {
        u32 p1 = i * (v + 1);
        u32 p2 = p1 + (v + 1);
        for (u32 j = 0; j < v; j++, p1++, p2++) {
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
    new_mesh.vertices = (Vertex*)platform_malloc(sizeof(Vertex) * new_mesh.vertices_count);
    
    u32 vertex_count = 0;
    for (u32 i = 0; i < u; i++) {
        u32 p1 = i * (v + 1);
        u32 p2 = p1 + (v + 1);
        for (u32 j = 0; j < v; j++, p1++, p2++) {
            new_mesh.vertices[vertex_count++] = mesh->vertices[p1];
            new_mesh.vertices[vertex_count++] = mesh->vertices[p1 + 1];
            new_mesh.vertices[vertex_count++] = mesh->vertices[p2];
            new_mesh.vertices[vertex_count++] = mesh->vertices[p2 + 1];
        }
    }
    
    new_mesh.indices_count = u * v * 4;
    new_mesh.indices = (u32*)platform_malloc(sizeof(u32) * new_mesh.indices_count);
    
    u32 indices_count = 0;
    for (u32 i = 0; i < u * v * 4; i += 4) {
        new_mesh.indices[indices_count++] = i;
        new_mesh.indices[indices_count++] = i + 1;
        new_mesh.indices[indices_count++] = i + 2;
        new_mesh.indices[indices_count++] = i + 3;
    }
    
    init_mesh(&new_mesh);
    return new_mesh;
}

function v3
apply_wave(Wave wave, v3 position, f32 time)
{
    f32 k = 2.0f * PI / wave.wave_length;
    f32 c = (f32)sqrt(9.8f / k);
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
apply_waves(const v3 position, Wave *waves, u32 waves_count, f32 time)
{
    v3 result = position;
    for (u32 wave_index = 0; wave_index < waves_count; wave_index++) {
        result += apply_wave(waves[wave_index], position, time / 4.0f);
    }
    return result;
}

internal void
init_boat_3D(Boat3D *boat, Font *font) {
    boat->lengths[FORWARD] = 4.0f;
    boat->lengths[BACKWARD] = 4.0f;
    boat->lengths[LEFT] = 2.0f;
    boat->lengths[RIGHT] = 2.0f;

    u32 index = 0;
    boat->easy.boxs[index++] = f32_textbox("Max Speed", &boat->maximum_speed);
    boat->easy.boxs[index++] = f32_textbox("Drag Mag", &boat->drag_magnitude);
    boat->easy.boxs[index++] = f32_textbox("Acc Mag", &boat->acceleration_magnitude);
    boat->easy.boxs[index++] = v3_textbox("Velocity", &boat->velocity);
    boat->easy.boxs[index++] = v3_textbox("Direction", &boat->direction);
    boat->easy.num_of_boxs = 5;
    boat->easy.draw = default_draw_textbox;
    boat->easy.draw.font = font;
    set_easy_textboxs_longest_tag(&boat->easy);
}

// updates the boats direction vectors to the new rotation
inline void
update_boat_vectors(Boat3D *boat) {
    boat->direction = boat->rotation * v3{ 1, 0, 0 };
    boat->up        = boat->rotation * v3{ 0, 1, 0 };
    boat->side      = boat->rotation * v3{ 0, 0, 1 };
}

// Left Hand because of OpenGL
struct Left_Hand {
    v3 forward;
    v3 up;
    v3 side;
};

internal Left_Hand
get_Left_Hand(v3 forward, v3 up) {
    Left_Hand hand = {};
    hand.forward = forward;
    hand.up = up;
    hand.side = cross_product(forward, up);
    return hand;
}

internal v3
get_axis(u32 index, Left_Hand hand) {
    switch(index) {
        case FORWARD:  return  hand.forward;
        case BACKWARD: return -hand.forward;
        case LEFT:     return -hand.side;
        case RIGHT:    return  hand.side;
        default:       return  hand.side;
    }
}

internal v3
get_perpendical_axis(u32 index, Left_Hand hand) {
    switch(index) {
        case FORWARD:  return  hand.side;
        case BACKWARD: return -hand.side;
        case LEFT:     return  hand.forward;
        case RIGHT:    return -hand.forward;
        default:       return  hand.side;
    }
}

internal void
apply_wave_rotation(Boat3D *boat, u32 index, Left_Hand hand, Wave *waves, u32 waves_count, r32 run_time_s) {
    r32 length = boat->lengths[index];

    v3 straight_pos = boat->wave.E[index];
    v3 wave_pos = apply_waves({straight_pos.x, -1.0f, straight_pos.z}, waves, waves_count, run_time_s);

    if (straight_pos.y < wave_pos.y) {
        r32 mag = wave_pos.y - straight_pos.y;
        boat->rotation = boat->rotation * get_rotation(DEG2RAD * mag, normalized(get_perpendical_axis(index, hand)));
    }

    update_boat_vectors(boat);

    boat->debug_wave_coords[index] = wave_pos;
}

internal Boat_Coords
Create_Boat_Coords(v3 coords, Left_Hand hand, f32 lengths[4]) {
    Boat_Coords result = {};

    result.forward  = coords + (normalized(get_axis(FORWARD,  hand)) * lengths[FORWARD]);
    result.backward = coords + (normalized(get_axis(BACKWARD, hand)) * lengths[BACKWARD]);
    result.left     = coords + (normalized(get_axis(LEFT,     hand)) * lengths[LEFT]);
    result.right    = coords + (normalized(get_axis(RIGHT,    hand)) * lengths[RIGHT]);
    result.center   = coords;

    return result;
}

internal void
update_boat_3D_draw_coord(Boat3D *boat, Wave *waves, u32 waves_count, r32 run_time_s)
{
    Left_Hand base = get_Left_Hand(boat->direction, { 0, 1, 0 });
    Left_Hand wave = get_Left_Hand(boat->direction, boat->up);

    boat->base = Create_Boat_Coords(boat->coords, base, boat->lengths);

    v3 draw_coords = apply_waves(boat->coords, waves, waves_count, run_time_s);
    v3 last_draw_coords = boat->wave.center;

    boat->wave = Create_Boat_Coords(draw_coords, wave, boat->lengths);
/*
    boat->draw_coords_history[boat->newest_draw_coord_index] = draw_coords;
    
    v3 sum = draw_coords;
    u32 indices_added = 1;
    u32 index = boat->newest_draw_coord_index + 1;
    while(index != boat->newest_draw_coord_index) {
        sum += boat->draw_coords_history[index];
        index++;
        indices_added++;
        if (index >= ARRAY_COUNT(boat->draw_coords_history)) 
            index = 0;
    }
    
    boat->draw_coords = sum / (f32)indices_added;

    boat->newest_draw_coord_index++;
    if (boat->newest_draw_coord_index >= ARRAY_COUNT(boat->draw_coords_history)) 
        boat->newest_draw_coord_index = 0;
*/

    for (u32 i = 0; i < DIRECTIONS_2D; i++)
        apply_wave_rotation(boat, i,  wave, waves, waves_count, run_time_s);

    boat->draw_delta += magnitude(draw_coords - last_draw_coords);
    if (boat->draw_delta >= 1.5f) {
        add_particle(boat->base.E[BACKWARD], -boat->direction, 2.0f);
        boat->draw_delta = 0.0f;
    }
}

function v3
update_boat_3D(Boat3D *boat, r32 delta_time, Button forward, Button backward, Button left, Button right) {
    f32 rotation_speed = 50.0f * delta_time;
    r32 rotation_radians = DEG2RAD * rotation_speed;
    f32 m_per_s = 6.0f; 
    f32 m_moved = m_per_s * delta_time;
    v3 move_vector = { m_moved, 0.0f, m_moved };
    v3 time_vector = { delta_time, 0.0f, delta_time };

    if (is_down(left)) {
        boat->rotation = boat->rotation * get_rotation( rotation_radians, {0, 1, 0});
        update_boat_vectors(boat);
    } else if (is_down(right)) {
        boat->rotation = boat->rotation * get_rotation(-rotation_radians, {0, 1, 0});
        update_boat_vectors(boat);
    }

    // acceleration velocity
    v3 acceleration_velocity = {};
    if (is_down(forward)) {
        v3 acceleration = boat->direction * boat->acceleration_magnitude;
        acceleration_velocity = acceleration * delta_time;
    }

    // drag velocity
    f32 angle_dir_to_velocity = angle_between(boat->direction, boat->velocity);
    if (angle_dir_to_velocity < 0.001f) {
        angle_dir_to_velocity = 0.001f; // make sure that there is always aleast some drag on the boat (would have to go in a straight line for a while to get this aligned). 
    }

    //v3 drag_force = -normalized(boat->velocity) * power(boat->velocity, 2) * boat->drag_magnitude; // just needs less than acceleration
    v3 drag_force = -normalized(boat->velocity) * magnitude(boat->velocity) * angle_dir_to_velocity * boat->drag_magnitude;
    v3 drag_velocity = drag_force * delta_time;

    v3 new_velocity = acceleration_velocity + drag_velocity;
    boat->velocity = boat->velocity + new_velocity;

    // good way to limit the speed of the boat
    if (magnitude(boat->velocity) > boat->maximum_speed) {
        boat->velocity = normalized(boat->velocity) * boat->maximum_speed;
    }

    v3 coords_delta = boat->velocity * time_vector; // how much to move the boat
    boat->coords += coords_delta;

    return coords_delta; // returns how much the boat was moved by to move the camera by the same amount.
}

internal void
draw_v3(v3 position, v3 v) {
    v4 cube_color = { 0, 255, 0, 1 };
    v3 cube_scale = {0.2f, 0.2f, 0.2f};
    draw_cube(position, 0, cube_scale, cube_color);
    draw_cube(position + v, 0, cube_scale, cube_color);
}

internal void
draw_boat(Boat3D *boat3D, Assets *assets, Camera camera) {
    Model *boat = find_model(assets, "BOAT2");
    boat->color_shader = find_shader(assets, "MATERIAL");
    boat->texture_shader = find_shader(assets, "MATERIAL_TEX");
    draw_model(boat, camera, boat3D->wave.center, boat3D->rotation);

    v4 cube_color = { 255, 0, 0, 1 };
    v4 cube_color2 = { 255, 0, 255, 1 };
    v3 cube_scale = {0.2f, 0.2f, 0.2f};
    draw_cube(boat3D->wave.center, 0, cube_scale, cube_color);

    draw_v3(boat3D->wave.center, boat3D->direction);
    draw_v3(boat3D->wave.center, boat3D->velocity);

    /*
    for (u32 i = 0; i < DIRECTIONS_2D; i++)
        draw_cube(boat3D->wave.E[i], 0, cube_scale, cube_color);
    for (u32 i = 0; i < DIRECTIONS_2D; i++)
        draw_cube(boat3D->debug_wave_coords[i], 0, cube_scale, cube_color2);
    */
}

internal void
update_game_3D(Game *game, Game_3D *data, Dev_Tools *tools, Input *input, const Time time)
{
    Controller *controller = input->active_controller;
    Camera *camera = &data->camera;
    
    if (input->mode == INPUT_MODE_GAME && on_down(controller->pause)) 
    {
        game->paused = !game->paused;
        if (game->paused) input->relative_mouse_mode.set(false);
        else              input->relative_mouse_mode.reset();
        return;
    }

    if (game->paused) {
        menu_update_active(&game->active, 0, 1, controller->backward, controller->forward);
        return;
    }

    // Only reaches here if the game is not paused. Update game time if not paused.
    game->run_time_s += time.frame_time_s; 

    switch(input->mode) {
        case INPUT_MODE_KEYBOARD: {
            if (tools->show_console) {
                tools->show_console = update_console(&tools->console, input);

                if (tools->show_console)
                    input->mode = INPUT_MODE_KEYBOARD;
                else
                    input->mode = INPUT_MODE_GAME;
            }
        } break;

        case INPUT_MODE_GAME: {
            // console
            if (!tools->show_console && on_down(controller->toggle_console)) {
                    tools->show_console = true;
                    input->mode = INPUT_MODE_KEYBOARD;
                    tools->console.lines_up_index = tools->console.lines;
            }

            // align camera
            if (console_command(&tools->console, ALIGN_CAMERA)) {
                align_camera_with_boat(&data->camera, &data->boat3D);
            }

            // update camera mode
            if (on_down(controller->toggle_camera_mode))
            {
                data->camera_mode++;
                if (data->camera_mode == CAMERA_MODES_COUNT) data->camera_mode = 0;

                add_onscreen_notification(&tools->onscreen_notifications, pair_get_value(camera_modes, CAMERA_MODES_COUNT, data->camera_mode));

                if (data->camera_mode == EDIT_CAMERA) {
                    input->relative_mouse_mode.set(false);
                } else {
                    input->relative_mouse_mode.set(true);
                }
            }



            switch(data->camera_mode) {
                case FREE_CAMERA: {
                    f32 mouse_m_per_s = 100.0f;
                    f32 mouse_move_speed = mouse_m_per_s * time.frame_time_s;
                    update_camera_with_mouse(camera, controller->mouse_rel, {mouse_move_speed, mouse_move_speed});
                    
                    f32 m_per_s = 6.0f; 
                    f32 m_moved = m_per_s * time.frame_time_s;
                    v3 move_vector = {m_moved, m_moved, m_moved};
                    update_camera_with_keys(&data->camera, data->camera.target, data->camera.up, move_vector,
                                            controller->forward, controller->backward,
                                            controller->left,    controller->right,
                                            controller->up,      controller->down);
                } break;

                case BOAT_CAMERA: {
                    // actual gameplay
                    data->camera.position += update_boat_3D(&data->boat3D, time.frame_time_s,
                                                            controller->forward, controller->backward, controller->left, controller->right);
                } break;
            }
        } break;
    }

    // camera menu
    if (console_command(&tools->console, TOGGLE_CAMERA_MENU)) {
        tools->show_camera_menu = !tools->show_camera_menu;
    }

    update_camera_target(&data->camera);
    update_boat_3D_draw_coord(&data->boat3D, data->waves_data.waves, data->waves_data.num_of_waves, game->run_time_s);
}

internal void
draw_skybox(Assets *assets, Cubemap *cubemap, Mesh *cube)
{
    platform_set_depth_mask(false);
    Shader *shader = find_shader(assets, "SKYBOX");
    u32 active_shader = use_shader(shader);

    m4x4 model_matrix = create_transform_m4x4({0, 0, 0}, get_rotation(0, {0, 1, 0}), {1000, 1000, 1000});
    uniform_m4x4(active_shader, "model", &model_matrix);

    platform_set_texture_cube_map(cubemap, active_shader);
    draw_mesh(cube);
    platform_set_depth_mask(true);
}

function void
draw_water(Assets *assets, Mesh mesh, r32 seconds, Camera camera, u32 depth_buffer_texture, u32 color_buffer_texture)
{
    u32 active_shader = use_shader(find_shader(assets, "WATER"));
    
    v4 color = {30.0f/255.0f, 144.0f/255.0f, 255.0f/255.0f, 0.5f};
    m4x4 model = create_transform_m4x4({0, 0, 0}, get_rotation(0, {0, 1, 0}), {100, 1, 100});

    uniform_m4x4(active_shader, "model",     &model);
    uniform_f32 (active_shader, "time",       seconds);
    uniform_v3  (active_shader, "camera_pos", camera.position);
    uniform_v4  (active_shader, "user_color", color);

    // assign texture locations
    uniform_s32(active_shader, "depth_buffer", 0);
    uniform_s32(active_shader, "color_buffer", 1);
    uniform_s32(active_shader, "foam",         2);
    uniform_s32(active_shader, "normal_map",   3);

    // set textures
    platform_set_texture(depth_buffer_texture,          0);
    platform_set_texture(color_buffer_texture,          1);
    platform_set_texture(find_bitmap(assets, "FOAM"),   2);
    platform_set_texture(find_bitmap(assets, "NORMAL"), 3);

    draw_mesh_patches(&mesh);
}

function void
draw_ground(Assets *assets, Mesh mesh, r32 seconds, Camera camera) {
    u32 active_shader = use_shader(find_shader(assets, "MATERIAL_TEX"));
    
    m4x4 model = create_transform_m4x4({0, -5, 0}, get_rotation(0, {0, 1, 0}), {100, 1, 100});

    uniform_m4x4(active_shader, "model", &model);
    uniform_v3(active_shader, "viewPos", camera.position);

    Material material = {};
    material.ambient = { 0.1f, 0.1f, 0.1f };
    material.diffuse = { 0.0f, 0.1f, 0.25f };
    material.specular = { 0.1f, 0.1f, 0.1f };
    material.specular_exponent = 15.0f;

    uniform_v3(active_shader, "material.ambient", material.ambient);
    uniform_v3(active_shader, "material.diffuse", material.diffuse);
    uniform_v3(active_shader, "material.specular", material.specular);
    uniform_f32(active_shader, "material.shininess", material.specular_exponent);

    platform_set_texture(find_bitmap(assets, "FOAM"));

    draw_mesh(&mesh);
}

internal void
draw_game_3D(Game *game, 
             Game_3D *data, 
             Dev_Tools *tools,
             Matrices *matrices,
             const Time time,
             Assets *assets, 
             Input *input, 
             const v2s window_dim, 
             u32 tex_depth_buffer,
             u32 color_buffer_texture)
{
	Controller *menu_controller = input->active_controller;

	matrices->view_matrix = get_view(data->camera);
	perspective(game->ubos.matrices, matrices); // 3D
    
    // skybox
    draw_skybox(assets, &data->skybox, &data->skybox_cube);

	platform_set_capability(PLATFORM_CAPABILITY_DEPTH_TEST, true);
	platform_set_capability(PLATFORM_CAPABILITY_CULL_FACE, true);

    draw_ground(assets, data->triangle_mesh, game->run_time_s, data->camera);
    update_particles(time.frame_time_s);
    draw_particles(assets, game->run_time_s);
    draw_boat(&data->boat3D, assets, data->camera);
    draw_cube({50, -5, 20}, 0, { 20, 15, 5 }, {50, 255, 0, 1});

    copy_buffers_to_textures(tex_depth_buffer, color_buffer_texture, window_dim);

    draw_water(assets, data->water, game->run_time_s, data->camera, tex_depth_buffer, color_buffer_texture);
    draw_cube(data->light.position, 0, { 1, 1, 1 }, data->light.color * 255.0f);

    // 2D
	orthographic(game->ubos.matrices, matrices); 
    platform_set_capability(PLATFORM_CAPABILITY_DEPTH_TEST, false);
    platform_set_capability(PLATFORM_CAPABILITY_CULL_FACE, false);
    set_polygon_mode(POLYGON_MODE_FILL); // wireframe off

    // Tools
    draw_easy_textboxs(&data->boat3D.easy, menu_controller->mouse_left, menu_controller->mouse, input);

    if (tools->show_camera_menu) {
        draw_easy_textboxs(&tools->camera_menu, menu_controller->mouse_left, menu_controller->mouse, input);
    }
    
    if (tools->show_fps) {
        Font *caslon = find_font(assets, "CASLON");
        draw_string(caslon, float_to_char_array(time.frames_per_s), { 100, 100 }, 50, { 255, 150, 0, 1 });
    }

    if (tools->show_console) {
        draw_console(&tools->console, window_dim, menu_controller->mouse_left, menu_controller->mouse);
    }
    
    if (tools->onscreen_notifications.lines > 0) {
        draw_onscreen_notifications(&tools->onscreen_notifications, window_dim, time.frame_time_s);
    }

    // Menu
    if (game->paused) {
        draw_rect( { 0, 0 }, 0, cv2(window_dim), { 0, 0, 0, 0.5f} );
        
        s32 pause = draw_pause_menu(assets, cv2(window_dim), on_down(menu_controller->select), game->active);
        
        switch(pause) {
            case 1: game->paused = false;   break;
            case 2: game->mode = MAIN_MENU; break;
        }
    }

    // reset wireframe
    if (tools->wire_frame) {
        set_polygon_mode(POLYGON_MODE_LINE);
    } else {
        set_polygon_mode(POLYGON_MODE_FILL);
    }
}