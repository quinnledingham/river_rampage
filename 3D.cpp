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
update_camera_with_keys(Camera *camera, v3 move_vector,
                        Button forward, Button backward,
                        Button left, Button right,
                        Button up, Button down)
{
    if (is_down(forward))  camera->position += camera->target * move_vector;
    if (is_down(backward)) camera->position -= camera->target * move_vector;
    if (is_down(left))     camera->position -= normalized(cross_product(camera->target, camera->up)) * move_vector;
    if (is_down(right))    camera->position += normalized(cross_product(camera->target, camera->up)) * move_vector;
    if (is_down(up))       camera->position.y += move_vector.y;
    if (is_down(down))     camera->position.y -= move_vector.y;
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
apply_waves(const v3 position, Wave *waves, u32 waves_count, f32 time)
{
    v3 result = position;
    for (u32 wave_index = 0; wave_index < waves_count; wave_index++)
    {
        result += apply_wave(waves[wave_index], position, time / 5.0);
    }
    return result;
}

function void
draw_water(Assets *assets, Mesh mesh, r32 seconds, Wave *waves, u32 waves_count, Light_Source light, Camera camera)
{
    u32 active_shader = use_shader(find_shader(assets, "WATER"));
    
    v4 color = {30.0f/255.0f, 144.0f/255.0f, 255.0f/255.0f, 0.9};
    m4x4 model = create_transform_m4x4({0, 0, 0}, get_rotation(0, {1, 0, 0}), {10, 1, 10});

    platform_uniform_m4x4(active_shader, "model", &model);
    platform_uniform_f32(active_shader, "time", seconds);
    platform_uniform_v3(active_shader, "lightPos", light.position);
    platform_uniform_v3(active_shader, "lightColor", light.color.rgb);
    platform_uniform_v3(active_shader, "cameraPos", camera.position);
    platform_uniform_v4(active_shader, "objectColor", color);
    
    Bitmap *perlin = find_bitmap(assets, "NORMAL");\
    platform_set_texture(perlin);

    draw_mesh_patches(&mesh);
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

    if (!data->paused)
    {
        f32 mouse_m_per_s = 100.0f;
        f32 mouse_move_speed = mouse_m_per_s * time.frame_time_s;
        update_camera_with_mouse(camera, controller->mouse, {mouse_move_speed, mouse_move_speed});
        
        f32 m_per_s = 5.0f; 
        f32 move_speed = m_per_s * time.frame_time_s;
        v3 move_vector = {move_speed, move_speed, move_speed};
        
        update_camera_with_keys(camera, move_vector,
                                controller->forward, controller->backward,
								controller->left,    controller->right,
                                controller->up,      controller->down);
        
        data->boat3D.draw_coords = apply_waves(data->boat3D.coords, data->waves, 5, time.run_time_s);
    }
    else
    {
        menu_update_active(&data->active, 0, 1, controller->backward, controller->forward);
    }
}

internal void
draw_game_3D(Application *app, Game_Data *data)
{
	Controller *menu_controller = app->input.active_controller;

	app->matrices.view_matrix = get_view(data->camera);
	perspective(data->matrices_ubo, &app->matrices); // 3D
            
	platform_set_capability(PLATFORM_CAPABILITY_DEPTH_TEST, true);
	platform_set_capability(PLATFORM_CAPABILITY_CULL_FACE, true);
            
    draw_water(&app->assets, data->water, app->time.run_time_s, data->waves, 5, data->light, data->camera);
    draw_model(find_shader(&app->assets, "MATERIAL"), &data->tree, data->light, data->camera);
    draw_cube(data->light.position, 0, { 1, 1, 1 }, data->light.color * 255.0f);
    draw_cube(data->boat3D.draw_coords, 0, { 1, 1, 1 }, { 255, 0, 255, 255 });
    
	orthographic(data->matrices_ubo, &app->matrices); // 2D

    platform_set_capability(PLATFORM_CAPABILITY_DEPTH_TEST, false);
    platform_set_capability(PLATFORM_CAPABILITY_CULL_FACE, false);
    
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
            data->active = 0;
        }
    }
}