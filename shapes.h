#ifndef SHAPES_H
#define SHAPES_H

// update on load
global Shader *shape_color_shader;
global Shader *shape_texture_shader;
global Mesh shape_rect = {};
global Mesh shape_circle = {};
global Mesh shape_cube = {};

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

void init_shapes(Shader *color, Shader *texture);
void draw_shape(Shape shape);

// Shapes for the game code to use
struct Circle
{
    v2 coords;
    r32 radius;
};

void draw_circle(v2 coords, r32 rotation, r32 radius, v4 color);

struct Rect
{
    v2 coords;
    v2 dim;
    r32 rotation; // radians
};

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

void draw_rect(v2 coords, r32 rotation, v2 dim, v4 color);
void draw_rect(v2 coords, r32 rotation, v2 dim, Bitmap *bitmap);
void draw_string(Font *font, const char *string, v2 coords, f32 pixel_height, v4 color);

Mesh get_cube_mesh();
Mesh get_cube_mesh(b32 out);
void draw_cube(v3 coords, r32 rotation, v3 dim, v4 color);
void draw_cube(v3 coords, r32 rotation, v3 dim, Bitmap *bitmap);

#endif //SHAPES_H
