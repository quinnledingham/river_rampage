#ifndef RECT_H
#define RECT_H

struct Rect
{
    v2 coords;
    v2 dim;
};

global_variable m4x4 orthographic_matrix = {};

function void
set_orthographic_matrix(v2s window_dim)
{
    orthographic_matrix = orthographic_projection(0.0f, (r32)window_dim.width,
                                                  (r32)window_dim.height, 0.0f,
                                                  -3.0f, 3.0f);
}

#endif //RECT_H
