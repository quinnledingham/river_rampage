function void
draw_shape(Shape shape, m4x4 projection_matrix, m4x4 view_matrix)
{
    u32 handle = 0;
    
    switch(shape.draw_type)
    {
        case SHAPE_COLOR:
        {
            handle = use_shader(&shape_color_shader);
            glUniform4fv(glGetUniformLocation(handle, "user_color"), (GLsizei)1, (float*)&shape.color);
        } break;
        
        case SHAPE_TEXTURE:
        {
            handle = use_shader(&shape_texture_shader);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, shape.bitmap->handle);
            glUniform1i(glGetUniformLocation(handle, "tex0"), 0);
        } break;
        
        default: error("draw_shape(): Not valid shape draw type");
    };
    
    shape.coords += shape.dim / 2.0f; // coords = top left corner
    
    m4x4 model = create_transform_m4x4(shape.coords, shape.rotation, shape.dim);
    glUniformMatrix4fv(glGetUniformLocation(handle, "model"), (GLsizei)1, false, (float*)&model);
    glUniformMatrix4fv(glGetUniformLocation(handle, "projection"), (GLsizei)1, false, (float*)&projection_matrix);
    glUniformMatrix4fv(glGetUniformLocation(handle, "view"), (GLsizei)1, false, (float*)&view_matrix);
    
    switch(shape.type)
    {
        case SHAPE_RECT: draw_mesh(&shape_rect); break;
        case SHAPE_CIRCLE: draw_mesh(&shape_circle); break;
        default: error("draw_shape(): Not valid shape type");
    }
}

//
// rect
//

function void
init_rect_indices(u32 *indices, 
                  u32 top_left, u32 top_right,
                  u32 bottom_left, u32 bottom_right)
{
    indices[0] = top_left;
    indices[1] = bottom_left;
    indices[2] = bottom_right;
    indices[3] = top_left;
    indices[4] = bottom_right;
    indices[5] = top_right;
}

function void
init_rect_mesh(Mesh *rect)
{
    rect->vertices_count = 4;
    rect->vertices = ARRAY_MALLOC(Vertex, rect->vertices_count);
    
    rect->vertices[0] = { {-0.5, -0.5, 0}, {0, 0, 1}, {0, 0} };
    rect->vertices[1] = { {-0.5, 0.5, 0}, {0, 0, 1}, {0, 1} };
    rect->vertices[2] = { {0.5, -0.5, 0}, {0, 0, 1}, {1, 0} };
    rect->vertices[3] = { {0.5, 0.5, 0}, {0, 0, 1}, {1, 1} };
    
    rect->indices_count = 6;
    rect->indices = ARRAY_MALLOC(u32, rect->indices_count);
    init_rect_indices(rect->indices, 1, 3, 0, 2);
    
    init_mesh(rect);
}

function void
center_on(Rect *rect, v2 coords)
{
    rect->coords = coords - (rect->dim / 2.0f);
}

function void
draw_rect(v2 coords, r32 rotation, v2 dim, v4 color)
{
    v3 coords_v3 = { coords.x, coords.y, 0 };
    quat rotation_quat = get_rotation(rotation, { 0, 0, 1 });
    v3 dim_v3 = { dim.x, dim.y, 1 };
    
    Shape shape = {};
    shape.type = SHAPE_RECT;
    shape.coords = coords_v3;
    shape.rotation = rotation_quat;
    shape.dim = dim_v3;
    shape.draw_type = SHAPE_COLOR;
    shape.color = color;
    draw_shape(shape, orthographic_matrix, identity_m4x4());
}

function void
draw_rect(v2 coords, r32 rotation, v2 dim, Bitmap *bitmap)
{
    v3 coords_v3 = { coords.x, coords.y, 0 };
    quat rotation_quat = get_rotation(-rotation, { 0, 0, 1 });
    v3 dim_v3 = { dim.x, dim.y, 1 };
    
    Shape shape = {};
    shape.type = SHAPE_RECT;
    shape.coords = coords_v3;
    shape.rotation = rotation_quat;
    shape.dim = dim_v3;
    shape.draw_type = SHAPE_TEXTURE;
    shape.bitmap = bitmap;
    draw_shape(shape, orthographic_matrix, identity_m4x4());
}

//
// circle
//

/*
6 vertices circle

  ###.###.###
###########
.####.####.
###########
###.###.###
*/

function void
init_circle_mesh(Mesh *mesh)
{
    u32 circle_vertices = 64;
    r32 deg_between_vertices = 360.0f / circle_vertices;
    r32 radius = 0.5f;
    v3 normal = { 0, 0, 1 };
    
    // allocating memory
    mesh->vertices_count = 1 + circle_vertices; // +1 for center
    mesh->vertices = ARRAY_MALLOC(Vertex, mesh->vertices_count);
    
    mesh->indices_count = circle_vertices * 3;
    mesh->indices = ARRAY_MALLOC(u32, mesh->indices_count);
    
    // center vertex
    mesh->vertices[0] = { { 0, 0, 0 }, normal, { 0.5, 0.5 } };
    
    u32 indices_index = 0;
    for (u32 i = 0; i < circle_vertices; i++)
    {
        v3 coords = {};
        v2 texture_coords = {};
        r32 rad = DEG2RAD * (i * deg_between_vertices);
        
        coords.x = radius * cosf(rad);
        coords.y = radius * sinf(rad);
        
        // NOT CORRECT
        texture_coords.x = coords.x;
        texture_coords.y = coords.y;
        
        mesh->vertices[i] = { coords, normal, texture_coords };
        
        if (i == 0) continue;
        
        // Make triangles
        mesh->indices[indices_index++] = 0;
        mesh->indices[indices_index++] = i - 1;
        mesh->indices[indices_index++] = i;
    }
    
    init_mesh(mesh);
}

function void
draw_circle(v2 coords, r32 rotation, v2 dim, v4 color)
{
    v3 coords_v3 = { coords.x, coords.y, 0 };
    quat rotation_quat = get_rotation(rotation, { 0, 0, 1 });
    v3 dim_v3 = { dim.x, dim.y, 1 };
    
    Shape shape = {};
    shape.type = SHAPE_CIRCLE;
    shape.coords = coords_v3;
    shape.rotation = rotation_quat;
    shape.dim = dim_v3;
    shape.draw_type = SHAPE_COLOR;
    shape.color = color;
    draw_shape(shape, orthographic_matrix, identity_m4x4());
}

function void
init_shapes()
{
    init_rect_mesh(&shape_rect);
    init_circle_mesh(&shape_circle);
    
    shape_color_shader.vs_file = basic_vs;
    shape_color_shader.fs_file = color_fs;
    compile_shader(&shape_color_shader);
    
    shape_texture_shader.vs_file = basic_vs;
    shape_texture_shader.fs_file = tex_fs;
    compile_shader(&shape_texture_shader);
}