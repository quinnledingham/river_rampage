//
// drawing
// 

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
    rect->vertices[1] = { {-0.5,  0.5, 0}, {0, 0, 1}, {0, 1} };
    rect->vertices[2] = { { 0.5, -0.5, 0}, {0, 0, 1}, {1, 0} };
    rect->vertices[3] = { { 0.5,  0.5, 0}, {0, 0, 1}, {1, 1} };
    
    rect->indices_count = 6;
    rect->indices = ARRAY_MALLOC(u32, rect->indices_count);
    init_rect_indices(rect->indices, 1, 3, 0, 2);
    
    init_mesh(rect);
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
    
    mesh->indices_count = circle_vertices * 3 + 6;
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
        
        mesh->vertices[i + 1] = { coords, normal, texture_coords };
        
        if (i == 0) continue;
        
        // Make triangles
        mesh->indices[indices_index++] = 0;
        mesh->indices[indices_index++] = i - 1;
        mesh->indices[indices_index++] = i;
    }
    
    mesh->indices[indices_index++] = 0;
    mesh->indices[indices_index++] = circle_vertices - 1;
    mesh->indices[indices_index++] = circle_vertices;
    
    mesh->indices[indices_index++] = 0;
    mesh->indices[indices_index++] = 1;
    mesh->indices[indices_index++] = circle_vertices;
    
    init_mesh(mesh);
}

function void
draw_circle(v2 coords, r32 rotation, r32 radius, v4 color)
{
    v3 coords_v3 = { coords.x, coords.y, 0 };
    quat rotation_quat = get_rotation(rotation, { 0, 0, 1 });
    v3 dim_v3 = { radius, radius, 1 };
    
    Shape shape = {};
    shape.type = SHAPE_CIRCLE;
    shape.coords = coords_v3;
    shape.rotation = rotation_quat;
    shape.dim = dim_v3;
    shape.draw_type = SHAPE_COLOR;
    shape.color = color;
    draw_shape(shape, orthographic_matrix, identity_m4x4());
}

//
// Cube
//

function Mesh
get_cube_mesh()
{
    Mesh mesh = {};
    
    mesh.vertices_count = 8;
    mesh.vertices = ARRAY_MALLOC(Vertex, mesh.vertices_count);
    
    mesh.vertices[0] = { {-0.5, -0.5, -0.5}, {0, 0, 1}, {0, 0} }; // bottom left
    mesh.vertices[1] = { {-0.5,  0.5, -0.5}, {0, 0, 1}, {0, 1} }; // top left
    mesh.vertices[2] = { { 0.5, -0.5, -0.5}, {0, 0, 1}, {1, 0} }; // bottom right
    mesh.vertices[3] = { { 0.5,  0.5, -0.5}, {0, 0, 1}, {1, 1} }; // top right
    
    mesh.vertices[4] = { {-0.5, -0.5, 0.5}, {0, 0, 1}, {0, 0} }; // bottom left
    mesh.vertices[5] = { {-0.5,  0.5, 0.5}, {0, 0, 1}, {0, 1} }; // top left
    mesh.vertices[6] = { { 0.5, -0.5, 0.5}, {0, 0, 1}, {1, 0} }; // bottom right
    mesh.vertices[7] = { { 0.5,  0.5, 0.5}, {0, 0, 1}, {1, 1} }; // top right
    
    mesh.indices_count = 6 * 6; // 6 indices per side (rects), 6 sides
    mesh.indices = ARRAY_MALLOC(u32, mesh.indices_count);
    
    init_rect_indices(mesh.indices + 0,  3, 1, 2, 0); // back
    init_rect_indices(mesh.indices + 6,  5, 7, 4, 6); // front
    init_rect_indices(mesh.indices + 12, 1, 3, 5, 7); // top
    init_rect_indices(mesh.indices + 18, 4, 6, 0, 2); // bottom
    init_rect_indices(mesh.indices + 24, 1, 5, 0, 4); // left
    init_rect_indices(mesh.indices + 30, 7, 3, 6, 2); // right
    
    init_mesh(&mesh);
    
    return mesh;
}

function void
draw_cube(m4x4 perspective_matrix, m4x4 view_matrix, v3 coords, r32 rotation, v3 dim, v4 color)
{
    quat rotation_quat = get_rotation(rotation, { 0, 0, 1 });
    
    Shape shape = {};
    shape.type = SHAPE_CUBE;
    shape.coords = coords;
    shape.rotation = rotation_quat;
    shape.dim = dim;
    shape.draw_type = SHAPE_COLOR;
    shape.color = color;
    draw_shape(shape, perspective_matrix, view_matrix);
}

//
// shapes
//

function void
init_shapes()
{
    init_rect_mesh(&shape_rect);
    init_circle_mesh(&shape_circle);
    shape_cube = get_cube_mesh();
    
    shape_color_shader.vs_file = basic_vs;
    shape_color_shader.fs_file = color_fs;
    compile_shader(&shape_color_shader);
    
    shape_texture_shader.vs_file = basic_vs;
    shape_texture_shader.fs_file = tex_fs;
    compile_shader(&shape_texture_shader);
}

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
    glUniformMatrix4fv(glGetUniformLocation(handle, "model"),      (GLsizei)1, false, (float*)&model);
    glUniformMatrix4fv(glGetUniformLocation(handle, "projection"), (GLsizei)1, false, (float*)&projection_matrix);
    glUniformMatrix4fv(glGetUniformLocation(handle, "view"),       (GLsizei)1, false, (float*)&view_matrix);
    
    switch(shape.type)
    {
        case SHAPE_RECT:   draw_mesh(&shape_rect);   break;
        case SHAPE_CIRCLE: draw_mesh(&shape_circle); break;
        case SHAPE_CUBE:   draw_mesh(&shape_cube); break;
        default: error("draw_shape(): Not valid shape type");
    }
}

//
// gameplay
//

//
// rect
//

// uses draw_rect
function void
draw_string(Font *font, const char *string, v2 coords, f32 pixel_height, v4 color)
{
    f32 scale = stbtt_ScaleForPixelHeight(&font->info, pixel_height);
    f32 string_x_coord = coords.x;
    
    u32 i = 0;
    while (string[i] != 0)
    {
        Font_Char *font_char = load_font_char(font, string[i], scale, color);
        
        f32 y = coords.y + font_char->c_y1;
        f32 x = string_x_coord + (font_char->lsb * scale);
        v2 dim = { f32(font_char->c_x2 - font_char->c_x1), f32(font_char->c_y2 - font_char->c_y1) };
        
        draw_rect({ x, y }, 0, {dim.x, dim.y}, &font_char->bitmap);
        
        int kern = stbtt_GetCodepointKernAdvance(&font->info, string[i], string[i + 1]);
        string_x_coord += ((kern + font_char->ax) * scale);
        
        i++;
    }
}

// returns the coords at the center of rect
function v2 get_center(Rect rect) { return rect.coords + (rect.dim / 2.0f); }

function void
center_on(Rect *rect, v2 coords)
{
    rect->coords = coords - (rect->dim / 2.0f);
}

// returns the coords of where in would be placed to
// be centered in out
function v2
get_centered(Rect in, Rect out)
{
    return
    { 
        (out.dim.x/2.0f) - (in.dim.x / 2.0f), 
        (out.dim.y/2.0f) - (in.dim.y / 2.0f)
    };
}

function Rect
get_centered_rect(Rect og, r32 x_percent, r32 y_percent)
{
    Rect rect = {};
    rect.dim.x = og.dim.x * x_percent;
    rect.dim.y = og.dim.y * y_percent;
    rect.coords = get_centered(rect, og);
    rect.coords += og.coords;
    return rect;
}

function Rect
get_centered_rect_pad(Rect og, v2 pad)
{
    Rect rect = {};
    rect.dim.x = og.dim.x + pad.x;
    rect.dim.y = og.dim.y + pad.y;
    rect.coords = get_centered(rect, og);
    rect.coords += og.coords;
    return rect;
}

function Rect
get_centered_square(Rect og, r32 percent)
{
    Rect rect = {};
    if (og.dim.x <= og.dim.y)
    {
        rect.dim.x = og.dim.x * percent;
        rect.dim.y = rect.dim.x;
    }
    else
    {
        rect.dim.y = og.dim.y * percent;
        rect.dim.x = rect.dim.y;
    }
    
    //log("%f %f\n", rect.dim.x, rect.dim.y);
    rect.coords = get_centered(rect, og);
    rect.coords += og.coords;
    return rect;
}