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

function void draw_shape(Shape shape);

// update on load
global_variable Shader *shape_color_shader;
global_variable Shader *shape_texture_shader;
global_variable Mesh shape_rect = {};
global_variable Mesh shape_circle = {};
global_variable Mesh shape_cube = {};

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


#endif //SHAPES_H
