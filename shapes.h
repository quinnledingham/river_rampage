#ifndef SHAPES_H
#define SHAPES_H

enum Shape_Types
{
    SHAPE_RECT,
    SHAPE_CIRCLE,
};

enum Shape_Draw_Types
{
    SHAPE_COLOR,
    SHAPE_TEXTURE,
};

struct Shape
{
    u32 type;
    v3 coords;
    quat rotation;
    v3 dim;
    
    u32 draw_type;
    v4 color;
    Bitmap *bitmap;
};

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

// update on window resize
global_variable m4x4 orthographic_matrix = {};

// update on load
global_variable Shader shape_color_shader = {};
global_variable Shader shape_texture_shader = {};
global_variable Mesh shape_rect = {};
global_variable Mesh shape_circle = {};

function void
set_orthographic_matrix(v2s window_dim)
{
    orthographic_matrix = orthographic_projection(0.0f, (r32)window_dim.width, (r32)window_dim.height, 0.0f, -3.0f, 3.0f);
}

function void draw_rect(v2 coords, r32 rotation, v2 dim, v4 color);
function void draw_rect(v2 coords, r32 rotation, v2 dim, Bitmap *bitmap);
function void draw_rect(Rect rect, v4 color);
function void draw_rect(Rect rect, Bitmap *bitmap);

#endif //SHAPES_H
