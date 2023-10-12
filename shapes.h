#ifndef SHAPES_H
#define SHAPES_H

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

function void draw_shape(Shape shape, m4x4 projection_matrix, m4x4 view_matrix);

// update on window resize
global_variable m4x4 orthographic_matrix = {};

// update on load
global_variable Shader shape_color_shader = {};
global_variable Shader shape_texture_shader = {};
global_variable Mesh shape_rect = {};
global_variable Mesh shape_circle = {};
global_variable Mesh shape_cube = {};

function void
set_orthographic_matrix(v2s window_dim)
{
    orthographic_matrix = orthographic_projection(0.0f, (r32)window_dim.width, (r32)window_dim.height, 0.0f, -3.0f, 3.0f);
}

function void draw_rect(v2 coords, r32 rotation, v2 dim, v4 color);
function void draw_rect(v2 coords, r32 rotation, v2 dim, Bitmap *bitmap);

// Shapes for the game code to use
struct Circle
{
    v2 coords;
    r32 radius;
};

struct Rect
{
    v2 coords;
    v2 dim;
    r32 rotation; // radians
};

function void draw_rect(Rect rect, v4 color) { draw_rect(rect.coords, 0.0f, rect.dim, color); }
function void draw_rect(Rect rect, Bitmap *bitmap) { draw_rect(rect.coords, 0.0f, rect.dim, bitmap); }
function void draw_rect(Rect rect, r32 rotation, Bitmap *bitmap) { draw_rect(rect.coords, rotation, rect.dim, bitmap); }

#endif //SHAPES_H
