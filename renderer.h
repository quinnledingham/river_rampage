#ifndef RENDERER_H
#define RENDERER_H

struct Matrices // for rendering
{
    b32 update;

    m4x4 perspective_matrix;
    m4x4 orthographic_matrix;
    m4x4 view_matrix;
};
global v2s renderer_window_dim;

// functions to set matrices in uniform buffer
void orthographic(u32 ubo, Matrices *matrices);
void perspective(u32 ubo, Matrices *matrices);

u32 init_uniform_buffer_object(u32 block_size, u32 block_index);
void platform_set_uniform_block_binding(u32 shader_handle, const char *tag, u32 index);
void platform_set_uniform_buffer_data(u32 ubo, u32 size, void *data);

void platform_uniform_m4x4(u32 shader_handle, const char *tag, m4x4 *m);
void platform_uniform_f32(u32 shader_handle, const char *tag, f32 f);
void platform_uniform_s32(u32 shader_handle, const char *tag, s32 i);
void platform_uniform_v3(u32 shader_handle, const char *tag, v3 v);
void platform_uniform_v4(u32 shader_handle, const char *tag, v4 v);
void platform_set_texture(Bitmap *bitmap);
void platform_set_texture_cube_map(Cubemap *cubemap, u32 shader);

void platform_blend_function(u32 source_factor, u32 destination_factor);

enum
{
    PLATFORM_POLYGON_MODE_POINT,
    PLATFORM_POLYGON_MODE_LINE,
    PLATFORM_POLYGON_MODE_FILL,

    PLATFORM_CAPABILITY_SCISSOR_TEST,
    PLATFORM_CAPABILITY_DEPTH_TEST,
    PLATFORM_CAPABILITY_CULL_FACE,
};

void platform_set_polygon_mode(u32 mode);
void platform_set_capability(u32 capability, b32 state);
b32 platform_capability_enabled(u32 capability);
void platform_set_depth_mask(b32 state);
void platform_set_scissor_box(v2s bottom_left, v2s dim);

#endif // RENDERER_H