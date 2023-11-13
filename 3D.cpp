// delta_mouse is a relative mouse movement amount
// as opposed to the screen coords of the mouse
function void
update_camera_with_mouse(Camera *camera, v2s delta_mouse, v2 move_speed)
{    
    camera->yaw   += (f32)delta_mouse.x * move_speed.x;
    camera->pitch -= (f32)delta_mouse.y * move_speed.y;
    
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
update_camera_with_keys(Camera *camera, v3 target, v3 up_v, v3 magnitude,
                        Button forward, Button backward,
                        Button left, Button right,
                        Button up, Button down)
{
    if (is_down(forward))  camera->position += target * magnitude;
    if (is_down(backward)) camera->position -= target * magnitude;
    if (is_down(left))     camera->position -= normalized(cross_product(target, up_v)) * magnitude;
    if (is_down(right))    camera->position += normalized(cross_product(target, up_v)) * magnitude;
    if (is_down(up))       camera->position.y += magnitude.y;
    if (is_down(down))     camera->position.y -= magnitude.y;
}

function Mesh
create_square_mesh(u32 u, u32 v)
{
    Mesh result = {};
    result.vertices_count = (u + 1) * (v + 1);
    result.vertices = (Vertex*)platform_malloc(sizeof(Vertex) * result.vertices_count);
    
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
    result.indices = (u32*)platform_malloc(sizeof(u32) * result.indices_count);
    
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
    new_mesh.vertices = (Vertex*)platform_malloc(sizeof(Vertex) * new_mesh.vertices_count);
    
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
    new_mesh.indices = (u32*)platform_malloc(sizeof(u32) * new_mesh.indices_count);
    
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
    for (u32 wave_index = 0; wave_index < waves_count; wave_index++)
    {
        result += apply_wave(waves[wave_index], position, time / 5.0f);
    }
    return result;
}

function void
update_boat_3D(Boat3D *boat,   v3 target, v3 up,
               v3 move_vector, r32 rotation_speed,
               Button forward, Button backward,
               Button left,    Button right)
{
    if (is_down(left))
    {
        quat rot = get_rotation(DEG2RAD * rotation_speed, up);
        boat->direction = rot * boat->direction;
    }
    else if (is_down(right))
    {
        quat rot = get_rotation(DEG2RAD * -rotation_speed, up);
        boat->direction = rot * boat->direction;
    }

    if (is_down(forward))  boat->coords += target * move_vector;
    if (is_down(backward)) boat->coords -= target * move_vector;

    //if (is_down(left))     boat->coords -= normalized(cross_product(target, up)) * move_vector;
    //if (is_down(right))    boat->coords += normalized(cross_product(target, up)) * move_vector;
}

internal void
update_boat_3D_draw_coord(Boat3D *boat, Wave *waves, u32 waves_count, r32 run_time_s)
{
    v3 draw_coords = apply_waves(boat->coords, waves, waves_count, run_time_s);

    boat->draw_coords_history[boat->newest_draw_coord_index] = draw_coords;
    
    v3 sum = draw_coords;
    u32 indices_added = 1;
    u32 index = boat->newest_draw_coord_index + 1;
    while(index != boat->newest_draw_coord_index) {
        sum += boat->draw_coords_history[index];
        index++;
        indices_added++;
        if (index >= ARRAY_COUNT(boat->draw_coords_history)) index = 0;
    }
    
    boat->draw_coords = sum / (f32)indices_added;

    boat->newest_draw_coord_index++;
    if (boat->newest_draw_coord_index >= ARRAY_COUNT(boat->draw_coords_history)) boat->newest_draw_coord_index = 0;
}

internal void
update_game_3D(Game_Data *data, Camera *camera, Input *input, const Time time)
{
	Controller *controller = input->active_controller;

	if (on_down(controller->pause)) 
    {
        data->paused = !data->paused;
        if (data->paused) input->relative_mouse_mode.set(false);
        else              input->relative_mouse_mode.set(true);
    }

    if (data->show_console) {
        data->show_console = update_console(&data->console, input);
        if (!data->show_console && !data->paused) input->relative_mouse_mode.set(true);
    }
    else {
        if (on_down(controller->toggle_console))
        {
            data->show_console = !data->show_console;

            if (data->show_console) {
                input->mode = INPUT_MODE_KEYBOARD;
                input->relative_mouse_mode.set(false);
                data->console.lines_up_index = data->console.lines;
            }
            else {
                input->mode = INPUT_MODE_GAME;
                input->relative_mouse_mode.set(true);
            }
        }
    }

    if (!data->paused)
    {
        data->game_run_time_s += time.frame_time_s;

        // update camera
        if (on_down(controller->toggle_camera_mode))
        {
            data->camera_mode++;
            if (data->camera_mode == CAMERA_MODES_COUNT) data->camera_mode = 0;

            add_onscreen_notification(&data->onscreen_notifications, pair_get_value(camera_modes, 2, data->camera_mode));
        }

        Button null_button = {};
        f32 m_per_s = 2.5f; 
        f32 move_speed = m_per_s * time.frame_time_s;

        f32 boat_rotation_speed = 50.0f * time.frame_time_s;

        if (data->camera_mode == FREE_CAMERA)
        {
            f32 mouse_m_per_s = 100.0f;
            f32 mouse_move_speed = mouse_m_per_s * time.frame_time_s;
            update_camera_with_mouse(camera, controller->mouse, {mouse_move_speed, mouse_move_speed});
            
            v3 move_vector = {move_speed, move_speed, move_speed};
            update_camera_with_keys(&data->camera, data->camera.target, data->camera.up, move_vector,
                                    controller->forward, controller->backward,
    								controller->left,    controller->right,
                                    controller->up,      controller->down);
        }
        else if (data->camera_mode == BOAT_CAMERA)
        {
            v3 move_vector = {move_speed, 0.0f, move_speed};
            update_camera_with_keys(&data->camera, data->boat3D.direction, data->camera.up, move_vector,
                                    controller->forward, controller->backward,
                                    null_button, null_button,
                                    null_button, null_button);

            update_boat_3D(&data->boat3D, data->boat3D.direction, {0, 1, 0}, 
                           move_vector, boat_rotation_speed,
                           controller->forward, controller->backward,
                           controller->left,    controller->right);

        }

        update_boat_3D_draw_coord(&data->boat3D, data->waves, 5, data->game_run_time_s);
    }
    else
    {
        menu_update_active(&data->active, 0, 1, controller->backward, controller->forward);
    }
}

internal void
draw_skybox(Assets *assets, Cubemap *cubemap, Mesh *cube)
{
    platform_set_depth_mask(false);
    Shader *shader = find_shader(assets, "SKYBOX");
    u32 active_shader = use_shader(shader);

    m4x4 model_matrix = create_transform_m4x4({0, 0, 0}, get_rotation(0, {0, 1, 0}), {1000, 1000, 1000});
    platform_uniform_m4x4(active_shader, "model", &model_matrix);

    platform_set_texture_cube_map(cubemap, active_shader);
    draw_mesh(cube);
    platform_set_depth_mask(true);
}

function void
draw_water(Assets *assets, Mesh mesh, r32 seconds, Camera camera)
{
    u32 active_shader = use_shader(find_shader(assets, "WATER"));
    
    v4 color = {30.0f/255.0f, 144.0f/255.0f, 255.0f/255.0f, 0.9f};
    m4x4 model = create_transform_m4x4({0, 0, 0}, get_rotation(0, {0, 1, 0}), {50, 1, 50});

    platform_uniform_m4x4(active_shader, "model", &model);
    platform_uniform_f32(active_shader, "time", seconds);
    platform_uniform_v3(active_shader, "camera_pos", camera.position);
    platform_uniform_v4(active_shader, "user_color", color);

    draw_mesh_patches(&mesh);
}

internal void
draw_game_3D(Application *app, Game_Data *data)
{
	Controller *menu_controller = app->input.active_controller;

	app->matrices.view_matrix = get_view(data->camera);
	perspective(data->matrices_ubo, &app->matrices); // 3D
    
    // skybox
    
    draw_skybox(&app->assets, &data->skybox, &data->skybox_cube);

	platform_set_capability(PLATFORM_CAPABILITY_DEPTH_TEST, true);
	platform_set_capability(PLATFORM_CAPABILITY_CULL_FACE, true);
    
    draw_water(&app->assets, data->water, data->game_run_time_s, data->camera);

    Model *tails = find_model(&app->assets, "TAILS");
    tails->color_shader = find_shader(&app->assets, "MATERIAL");
    tails->texture_shader = find_shader(&app->assets, "MATERIAL_TEX");
    draw_model(tails, data->camera, {0, 0, 0}, get_rotation(0, {0, 1, 0}));
    draw_cube(data->light.position, 0, { 1, 1, 1 }, data->light.color * 255.0f);

    //draw_cube({1, 5, 1}, 0, { 1, 1, 1 }, find_bitmap(&app->assets, "BOAT"));
    //draw_cube(data->boat3D.draw_coords, 0, { 1, 1, 1 }, { 255, 0, 255, 255 });

    r32 angle = v2_to_angle({data->boat3D.direction.x, data->boat3D.direction.z});
    //log("x: %f, y: %f, %f", data->boat3D.direction.x, data->boat3D.direction.z, angle);
    quat rot = get_rotation(-angle, {0, 1, 0});

    Model *boat = find_model(&app->assets, "BOAT2");
    boat->color_shader = find_shader(&app->assets, "MATERIAL");
    boat->texture_shader = find_shader(&app->assets, "MATERIAL_TEX");
    draw_model(boat, data->camera, data->boat3D.draw_coords, rot);
    
	orthographic(data->matrices_ubo, &app->matrices); // 2D

    platform_set_capability(PLATFORM_CAPABILITY_DEPTH_TEST, false);
    platform_set_capability(PLATFORM_CAPABILITY_CULL_FACE, false);
    
    if (data->show_fps)
    {
        Font *caslon = find_font(&app->assets, "CASLON");
        draw_string(caslon, ftos(app->time.frames_per_s), { 100, 100 }, 50, { 255, 150, 0, 1 });
    }
    
    platform_set_polygon_mode(PLATFORM_POLYGON_MODE_FILL);  
    if (data->show_console) draw_console(&data->console, app->window.dim);
    draw_onscreen_notifications(&data->onscreen_notifications, app->window.dim, app->time.frame_time_s);
    if (data->wire_frame) platform_set_polygon_mode(PLATFORM_POLYGON_MODE_LINE);
    else                  platform_set_polygon_mode(PLATFORM_POLYGON_MODE_FILL);

    if (data->paused) 
    {
        draw_rect( { 0, 0 }, 0, cv2(app->window.dim), { 0, 0, 0, 0.5f} );
        
        s32 pause = draw_pause_menu(&app->assets, cv2(app->window.dim), on_down(menu_controller->select), data->active);
        if      (pause == 1) data->paused = false;
        else if (pause == 2) data->game_mode = MAIN_MENU;
    }
}