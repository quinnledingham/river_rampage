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
draw_rect(v3 coords, quat rotation, v3 dim, u32 handle, m4x4 projection_matrix, m4x4 view_matrix)
{
    coords += dim / 2.0f;
    
    m4x4 model = create_transform_m4x4(coords, rotation, dim);
    glUniformMatrix4fv(glGetUniformLocation(handle, "model"), (GLsizei)1, false, (float*)&model);
    glUniformMatrix4fv(glGetUniformLocation(handle, "projection"), (GLsizei)1, false, (float*)&projection_matrix);
    glUniformMatrix4fv(glGetUniformLocation(handle, "view"), (GLsizei)1, false, (float*)&view_matrix);
    
    local_persist Mesh rect = {};
    if (rect.vertices_count == 0) init_rect_mesh(&rect);
    
    draw_mesh(&rect);
}

function void
draw_rect(v3 coords, quat rotation, v3 dim, v4 color, m4x4 projection_matrix, m4x4 view_matrix)
{
    // it makes sense to have the shader local because these functions are tailored to
    // this shaders.
    local_persist Shader shader = {};
    if (!shader.compiled)
    {
        shader.vs_file = basic_vs;
        shader.fs_file = color_fs;
        compile_shader(&shader);
    }
    
    u32 handle = use_shader(&shader);
    
    glUniform4fv(glGetUniformLocation(handle, "user_color"), (GLsizei)1, (float*)&color);
    
    draw_rect(coords, rotation, dim, handle, projection_matrix, view_matrix);
}

function void
draw_rect(v3 coords, quat rotation, v3 dim, Bitmap *bitmap, m4x4 projection_matrix, m4x4 view_matrix)
{
    local_persist Shader shader = {};
    if (!shader.compiled)
    {
        shader.vs_file = basic_vs;
        shader.fs_file = tex_fs;
        compile_shader(&shader);
    }
    
    u32 handle = use_shader(&shader);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, bitmap->handle);
    glUniform1i(glGetUniformLocation(handle, "tex0"), 0);
    
    draw_rect(coords, rotation, dim, handle, projection_matrix, view_matrix);
}

function void
draw_rect(v2 coords, r32 rotation, v2 dim, v4 color)
{
    v3 coords_v3 = { coords.x, coords.y, 0 };
    quat rotation_quat = get_rotation(rotation, { 0, 0, 1 });
    v3 dim_v3 = { dim.x, dim.y, 1 };
    draw_rect(coords_v3, rotation_quat, dim_v3, color, orthographic_matrix, identity_m4x4());
}

function void
draw_rect(v2 coords, r32 rotation, v2 dim, Bitmap *bitmap)
{
    v3 coords_v3 = { coords.x, coords.y, 0 };
    quat rotation_quat = get_rotation(-rotation, { 0, 0, 1 });
    v3 dim_v3 = { dim.x, dim.y, 1 };
    draw_rect(coords_v3, rotation_quat, dim_v3, bitmap, orthographic_matrix, identity_m4x4());
}

function void 
draw_rect(Rect rect, v4 color) 
{
    draw_rect(rect.coords, 0.0f, rect.dim, color); 
}

function void 
draw_rect(Rect rect, Bitmap *bitmap) 
{ 
    draw_rect(rect.coords, 0.0f, rect.dim, bitmap);
}

function void
center_on(Rect *rect, v2 coords)
{
    rect->coords = coords - (rect->dim / 2.0f);
}
