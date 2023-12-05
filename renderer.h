#ifndef RENDERER_H
#define RENDERER_H

struct Matrices // for rendering
{
    b32 update;

    m4x4 perspective_matrix;
    m4x4 orthographic_matrix;
    m4x4 view_matrix;

    f32 p_near, p_far; // perspective matrix settings
    f32 window_width; 
    f32 window_height;
};
global v2s renderer_window_dim; // used for scissor box in menu.cpp

// functions to set matrices in uniform buffer
void orthographic(u32 ubo, Matrices *matrices);
void perspective(u32 ubo, Matrices *matrices);

void uniform_m4x4(u32 shader_handle, const char *tag, m4x4 *m);
void uniform_f32 (u32 shader_handle, const char *tag, f32   f);
void uniform_s32 (u32 shader_handle, const char *tag, s32   i);
void uniform_v3  (u32 shader_handle, const char *tag, v3    v);
void uniform_v4  (u32 shader_handle, const char *tag, v4    v);

void platform_set_texture(Bitmap *bitmap);
void platform_set_texture(Bitmap *bitmap, u32 index);
void platform_set_texture(u32 handle, u32 index);

void copy_buffers_to_textures(u32 depth_handle, u32 color_handle, v2s window_dim);
void platform_set_texture_cube_map(Cubemap *cubemap, u32 shader);

enum
{
    UNIFORM_BUFFER,
};

u32 init_uniform_buffer_object(u32 block_size, u32 block_index);
void platform_set_uniform_block_binding(u32 shader_handle, const char *tag, u32 index);

u32 gl_get_buffer_target(u32 target);
void bind_buffer(u32 target, u32 ubo);
void unbind_buffer(u32 target);
u32 buffer_sub_data(u32 target, u32 offset, u32 size, void *data);

#define BUFFER_SUB_DATA(target, offset, n)       buffer_sub_data(target, offset,                     sizeof(n), (void *)&n)
#define BUFFER_SUB_DATA_ARRAY(target, offset, n) buffer_sub_data(target, offset, ARRAY_COUNT(n) * sizeof(n[0]), (void *)&n)

#define UNIFORM_BUFFER_SUB_DATA(offset, n) buffer_sub_data(gl_get_buffer_target(UNIFORM_BUFFER), offset, sizeof(n), (void *)&n)

enum
{
    POLYGON_MODE_POINT,
    POLYGON_MODE_LINE,
    POLYGON_MODE_FILL,

    PLATFORM_CAPABILITY_SCISSOR_TEST,
    PLATFORM_CAPABILITY_DEPTH_TEST,
    PLATFORM_CAPABILITY_CULL_FACE,
};

void set_blend_function(u32 source_factor, u32 destination_factor);
void set_polygon_mode(u32 mode);
void platform_set_capability(u32 capability, b32 state);
b32 platform_capability_enabled(u32 capability);
void platform_set_depth_mask(b32 state);
void platform_set_scissor_box(v2s bottom_left, v2s dim);

#endif // RENDERER_H