#ifndef SHAPES_H
#define SHAPES_H

void init_shapes();

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

internal b8
inside_box(v2 coords, v2 box_coords, v2 box_dim) {
    if (box_coords.x <= coords.x && coords.x <= box_coords.x + box_dim.x &&
        box_coords.y <= coords.y && coords.y <= box_coords.y + box_dim.y)
        return true;

    return false;
}

void draw_rect(v2 coords, r32 rotation, v2 dim, v4 color);
void draw_rect(v2 coords, r32 rotation, v2 dim, Bitmap *bitmap);
void draw_rect(v2 coords, r32 rotation, v2 dim, Bitmap *bitmap, v4 color);
void draw_string(Font *font, const char *string, v2 coords, f32 pixel_height, v4 color);
void draw_string2(Font *font, const char *string, v2 coords, f32 pixel_height, v4 color);

Mesh get_cube_mesh();
Mesh get_cube_mesh(b32 out);
void draw_cube(v3 coords, r32 rotation, v3 dim, v4 color);
void draw_cube(v3 coords, r32 rotation, v3 dim, Bitmap *bitmap);

void draw_sphere(v3 coords, r32 rotation, v3 dim, v4 color);

#endif //SHAPES_H
