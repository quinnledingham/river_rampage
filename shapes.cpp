#include "log.h"
#include "types.h"
#include "types_math.h"
#include "char_array.h"
#include "assets.h"
#include "renderer.h"
#include "data_structures.h"
#include "shapes.h"
#include "application.h"

enum Shape_Types
{
    SHAPE_RECT,
    SHAPE_CIRCLE,
    SHAPE_CUBE,
};

enum Shape_Draw_Types
{
    SHAPE_COLOR,
    SHAPE_TEXTURE,
    SHAPE_TEXT,
};

struct Shape // container for all the information needed to draw the shape
{
    u32 type;
    v3 coords;
    quat rotation;
    v3 dim;
    
    u32 draw_type;
    v4 color;
    Bitmap *bitmap;
};

void init_shapes(Shader *color, Shader *texture, Shader *text);
void draw_shape(Shape shape);

struct Shapes {
    Shader *test_color;
    Shader *test_texture;
    Shader *test_text;

    Shader color;
    Shader texture;
    Shader text;

    Mesh rect;
    Mesh circle;
    Mesh cube;
};

Shapes shapes = {};

const char *basic_vs = "#version 330 core\nlayout (location = 0) in vec3 position;layout (location = 1) in vec3 normal;layout (location = 2) in vec2 texture_coords;out vec2 uv;layout (std140) uniform Matrices{mat4 projection;mat4 view;};uniform mat4 model; void main(void) { gl_Position = projection * view * model * vec4(position, 1.0f); uv = texture_coords;}";
const char *color_fs = "#version 330 core\nuniform vec4 user_color;in vec2 uv; out vec4 FragColor;void main() { FragColor  = vec4(user_color.x/255, user_color.y/255, user_color.z/255, user_color.w);}";
const char *tex_fs   = "#version 330 core\nuniform sampler2D tex0;in vec2 uv;out vec4 FragColor;void main() { vec4 tex = texture(tex0, uv); FragColor = tex;}";
const char *text_fs  = "#version 330 core\nin vec2 uv;out vec4 FragColor;uniform sampler2D tex0;uniform vec4 text_color;void main() { vec3 norm_text_color = vec3(text_color.x/255, text_color.y/255, text_color.z/255);float alpha = texture(tex0, uv).r * text_color.a;vec4 tex = vec4(1.0, 1.0, 1.0, alpha); FragColor = vec4(norm_text_color, 1.0) * tex;}";

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

void draw_rect(v2 coords, r32 rotation, v2 dim, v4 color)
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
    draw_shape(shape);
}

void draw_rect(v2 coords, r32 rotation, v2 dim, Bitmap *bitmap)
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
    draw_shape(shape);
}

void draw_rect(v2 coords, r32 rotation, v2 dim, Bitmap *bitmap, v4 color)
{
    v3 coords_v3 = { coords.x, coords.y, 0 };
    quat rotation_quat = get_rotation(-rotation, { 0, 0, 1 });
    v3 dim_v3 = { dim.x, dim.y, 1 };
    
    Shape shape = {};
    shape.type = SHAPE_RECT;
    shape.coords = coords_v3;
    shape.rotation = rotation_quat;
    shape.dim = dim_v3;
    shape.draw_type = SHAPE_TEXT;
    shape.bitmap = bitmap;
    shape.color = color;
    draw_shape(shape);
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

void draw_circle(v2 coords, r32 rotation, r32 radius, v4 color)
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
    draw_shape(shape);
}

//
// Cube
//
 
Mesh get_cube_mesh(b32 out)
{
    Mesh mesh = {};
    
    mesh.vertices_count = 8;
    mesh.vertices = ARRAY_MALLOC(Vertex, mesh.vertices_count);
    
    // back
    mesh.vertices[0] = { {-0.5, -0.5, -0.5}, {0, 0, 1}, {0, 0} }; // bottom left
    mesh.vertices[1] = { {-0.5,  0.5, -0.5}, {0, 0, 1}, {0, 1} }; // top left
    mesh.vertices[2] = { { 0.5, -0.5, -0.5}, {0, 0, 1}, {1, 0} }; // bottom right
    mesh.vertices[3] = { { 0.5,  0.5, -0.5}, {0, 0, 1}, {1, 1} }; // top right
    
    // forward
    mesh.vertices[4] = { {-0.5, -0.5, 0.5}, {0, 0, 1}, {0, 0} }; // bottom left
    mesh.vertices[5] = { {-0.5,  0.5, 0.5}, {0, 0, 1}, {0, 1} }; // top left
    mesh.vertices[6] = { { 0.5, -0.5, 0.5}, {0, 0, 1}, {1, 0} }; // bottom right
    mesh.vertices[7] = { { 0.5,  0.5, 0.5}, {0, 0, 1}, {1, 1} }; // top right
    /*
    // back
    mesh.vertices[0] = { {-1.0, -1.0, -1.0}, {0, 0, 1}, {0, 0} }; // bottom left
    mesh.vertices[1] = { {-1.0,  1.0, -1.0}, {0, 0, 1}, {0, 1} }; // top left
    mesh.vertices[2] = { { 1.0, -1.0, -1.0}, {0, 0, 1}, {1, 0} }; // bottom right
    mesh.vertices[3] = { { 1.0,  1.0, -1.0}, {0, 0, 1}, {1, 1} }; // top right
    
    // forward
    mesh.vertices[4] = { {-1.0, -1.0, 1.0}, {0, 0, 1}, {0, 0} }; // bottom left
    mesh.vertices[5] = { {-1.0,  1.0, 1.0}, {0, 0, 1}, {0, 1} }; // top left
    mesh.vertices[6] = { { 1.0, -1.0, 1.0}, {0, 0, 1}, {1, 0} }; // bottom right
    mesh.vertices[7] = { { 1.0,  1.0, 1.0}, {0, 0, 1}, {1, 1} }; // top right
    */
    mesh.indices_count = 6 * 6; // 6 indices per side (rects), 6 sides
    mesh.indices = ARRAY_MALLOC(u32, mesh.indices_count);
    
    if (out)
    {
        init_rect_indices(mesh.indices + 0,  3, 1, 2, 0); // back
        init_rect_indices(mesh.indices + 6,  5, 7, 4, 6); // front
        init_rect_indices(mesh.indices + 12, 1, 3, 5, 7); // top
        init_rect_indices(mesh.indices + 18, 4, 6, 0, 2); // bottom
        init_rect_indices(mesh.indices + 24, 1, 5, 0, 4); // left
        init_rect_indices(mesh.indices + 30, 7, 3, 6, 2); // right
    }
    else
    {
        init_rect_indices(mesh.indices + 0,  1, 3, 0, 2); // back
        init_rect_indices(mesh.indices + 6,  7, 5, 6, 4); // front
        init_rect_indices(mesh.indices + 12, 5, 7, 1, 3); // top
        init_rect_indices(mesh.indices + 18, 0, 2, 4, 6); // bottom
        init_rect_indices(mesh.indices + 24, 5, 1, 4, 0); // left
        init_rect_indices(mesh.indices + 30, 3, 7, 2, 6); // right
    }

    init_mesh(&mesh);
    
    return mesh;
}
Mesh get_cube_mesh() { return get_cube_mesh(true); }

void draw_cube(v3 coords, r32 rotation, v3 dim, v4 color)
{
    quat rotation_quat = get_rotation(rotation, { 0, 0, 1 });
    
    Shape shape = {};
    shape.type = SHAPE_CUBE;
    shape.coords = coords;
    shape.rotation = rotation_quat;
    shape.dim = dim;
    shape.draw_type = SHAPE_COLOR;
    shape.color = color;
    draw_shape(shape);
}

void draw_cube(v3 coords, r32 rotation, v3 dim, Bitmap *bitmap)
{
    quat rotation_quat = get_rotation(rotation, { 0, 0, 1 });
    
    Shape shape = {};
    shape.type = SHAPE_CUBE;
    shape.coords = coords;
    shape.rotation = rotation_quat;
    shape.dim = dim;
    shape.draw_type = SHAPE_TEXTURE;
    shape.bitmap = bitmap;
    draw_shape(shape);
}

//
// shapes
//

void init_shapes(Shader *color, Shader *texture, Shader *text) {
    init_rect_mesh(&shapes.rect);
    init_circle_mesh(&shapes.circle);
    shapes.cube = get_cube_mesh();
    
    shapes.test_color   = color;
    shapes.test_texture = texture;
    shapes.test_text    = text;
}

void init_shapes() {
    init_rect_mesh(&shapes.rect);
    init_circle_mesh(&shapes.circle);
    shapes.cube = get_cube_mesh();

    shapes.color.files[VERTEX_SHADER].memory   = (void*)basic_vs;
    shapes.texture.files[VERTEX_SHADER].memory = (void*)basic_vs;
    shapes.text.files[VERTEX_SHADER].memory    = (void*)basic_vs;    

    shapes.color.files[FRAGMENT_SHADER].memory   = (void*)color_fs;
    shapes.texture.files[FRAGMENT_SHADER].memory = (void*)tex_fs;
    shapes.text.files[FRAGMENT_SHADER].memory    = (void*)text_fs;

    compile_shader(&shapes.color);
    compile_shader(&shapes.texture);
    compile_shader(&shapes.text);

    platform_set_uniform_block_binding(shapes.text.handle,    "Matrices", 0);
    platform_set_uniform_block_binding(shapes.texture.handle, "Matrices", 0);
    platform_set_uniform_block_binding(shapes.color.handle,   "Matrices", 0);
}


function void
draw_shape(Shape shape)
{
    u32 handle = 0;
    
    switch(shape.draw_type)
    {
        case SHAPE_COLOR: {
            handle = use_shader(&shapes.color);
            platform_uniform_v4(handle, "user_color", shape.color);
        } break;
        
        case SHAPE_TEXTURE: {
            handle = use_shader(&shapes.texture);
            platform_set_texture(shape.bitmap);
            platform_uniform_s32(handle, "tex0", 0);
        } break;

        case SHAPE_TEXT: {
            handle = use_shader(&shapes.text);
            platform_set_texture(shape.bitmap);
            platform_uniform_s32(handle, "tex0", 0);
            platform_uniform_v4(handle, "text_color", shape.color);
        } break;
        
        default: error("draw_shape(): Not valid shape draw type");
    };
    
    shape.coords += shape.dim / 2.0f; // coords = top left corner
    
    m4x4 model = create_transform_m4x4(shape.coords, shape.rotation, shape.dim);
    platform_uniform_m4x4(handle, "model", &model);
    
    switch(shape.type)
    {
        case SHAPE_RECT:   draw_mesh(&shapes.rect);   break;
        case SHAPE_CIRCLE: draw_mesh(&shapes.circle); break;
        case SHAPE_CUBE:   draw_mesh(&shapes.cube);   break;
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
void draw_string(Font *font, const char *string, v2 coords, f32 pixel_height, v4 color)
{
    //stbtt_fontinfo *info = (stbtt_fontinfo*)font->info;
    f32 scale = get_scale_for_pixel_height(font->info, pixel_height);
    f32 string_x_coord = 0.0f;

    u32 i = 0;
    while (string[i] != 0)
    {
        Font_Char_Bitmap *bitmap = load_font_char_bitmap(font, string[i], scale);
        Font_Char *font_char = bitmap->font_char;
        
        v2 char_coords = { string_x_coord + font_char->lsb, -(r32)font_char->bb_1.y };
        v2 char_dim = cv2(font_char->bb_1 - font_char->bb_0);

        v2 scaled_coords = char_coords * scale;
        v2 scaled_dim    = char_dim    * scale;
        draw_rect(coords + scaled_coords, 0, scaled_dim, &bitmap->bitmap, color);
        
        s32 kern = get_codepoint_kern_advance(font->info, string[i], string[i + 1]);
        string_x_coord += kern + font_char->ax;
        
        i++;
    }
}

