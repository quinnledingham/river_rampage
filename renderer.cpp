// block index is from glUniformBlockBinding or binding == #
u32 init_uniform_buffer_object(u32 block_size, u32 block_index)
{
    u32 uniform_buffer_object;
    glGenBuffers(1, &uniform_buffer_object);
    
    // clearing buffer
    glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer_object);
    glBufferData(GL_UNIFORM_BUFFER, block_size, NULL, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    
    glBindBufferBase(GL_UNIFORM_BUFFER, block_index, uniform_buffer_object);
    
    return uniform_buffer_object;
}

void platform_set_uniform_block_binding(u32 shader_handle, const char *tag, u32 index)
{
    u32 tag_uniform_block_index = glGetUniformBlockIndex(shader_handle, tag);
    glUniformBlockBinding(shader_handle, tag_uniform_block_index, index);
}

u32 gl_get_buffer_target(u32 target) {
    switch(target) {
        case UNIFORM_BUFFER: return GL_UNIFORM_BUFFER; break;
        default: {
            error("gl_get_buffer_target(): Not a valid target");
            return 0;
        };
    }
}

void bind_buffer(u32 target, u32 ubo) {
    glBindBuffer(target, ubo);
}

void unbind_buffer(u32 target) {
    glBindBuffer(target, 0);
}

// returns the new offset
u32 buffer_sub_data(u32 target, u32 offset, u32 size, void *data) {
    glBufferSubData(target, offset, size, data);
    return (offset + size);
}

// functions to set matrices in uniform buffer
void orthographic(u32 ubo, Matrices *matrices)
{
    GLenum target = GL_UNIFORM_BUFFER;
    u32 offset = 0;

    glBindBuffer(target, ubo);
    offset = BUFFER_SUB_DATA(target, offset, matrices->orthographic_matrix);
    offset = BUFFER_SUB_DATA(target, offset, identity_m4x4());
    glBindBuffer(target, 0);
}

void perspective(u32 ubo, Matrices *matrices)
{
    GLenum target = GL_UNIFORM_BUFFER;
    u32 offset = 0;

    glBindBuffer(target, ubo);
    offset = BUFFER_SUB_DATA(target, offset, matrices->perspective_matrix);
    offset = BUFFER_SUB_DATA(target, offset, matrices->view_matrix);
    offset = BUFFER_SUB_DATA(target, offset, matrices->p_near);
    offset = BUFFER_SUB_DATA(target, offset, matrices->p_far);
    offset = BUFFER_SUB_DATA(target, offset, matrices->window_width);
    offset = BUFFER_SUB_DATA(target, offset, matrices->window_height);
    glBindBuffer(target, 0);
}

inline GLint
get_uniform_location(u32 handle, const char *tag) {
    return glGetUniformLocation(handle, tag);
}

void uniform_m4x4(u32 shader_handle, const char *tag, m4x4 *m) { glUniformMatrix4fv(get_uniform_location(shader_handle, tag), (GLsizei)1, false, (float*) m); }
void uniform_f32 (u32 shader_handle, const char *tag, f32   f) { glUniform1f       (get_uniform_location(shader_handle, tag),                             f); }
void uniform_s32 (u32 shader_handle, const char *tag, s32   i) { glUniform1i       (get_uniform_location(shader_handle, tag),                             i); }
void uniform_v3  (u32 shader_handle, const char *tag, v3    v) { glUniform3fv      (get_uniform_location(shader_handle, tag), (GLsizei)1,        (float*)&v); }
void uniform_v4  (u32 shader_handle, const char *tag, v4    v) { glUniform4fv      (get_uniform_location(shader_handle, tag), (GLsizei)1,        (float*)&v); }

//
// Textures
//

void platform_set_texture(u32 handle, u32 index) {
    glActiveTexture(GL_TEXTURE0 + index);
    glBindTexture(GL_TEXTURE_2D, handle);
}

void platform_set_texture(Bitmap *bitmap, u32 index) {
    platform_set_texture(bitmap->handle, index);
}

void platform_set_texture(Bitmap *bitmap) {
    platform_set_texture(bitmap, 0);
}

void platform_set_texture_cube_map(Cubemap *cubemap, u32 shader)
{
    //glDepthFunc(GL_LEQUAL);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap->handle);
    glUniform1i(glGetUniformLocation(shader, "skybox"), 0);
}


//
// Depth/Color Buffers
//

internal u32
get_depth_buffer_texture(v2s window_dim) {
    u32 handle;

    glGenTextures(1, &handle);
    glBindTexture(GL_TEXTURE_2D, handle);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, window_dim.x, window_dim.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    return handle;
}

internal u32
get_color_buffer_texture(v2s window_dim) {
    u32 handle;

    glGenTextures(1, &handle);
    glBindTexture(GL_TEXTURE_2D, handle);
  
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, window_dim.x, window_dim.y, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 

    return handle;
}

// copys the current depth buffer and the color buffer to textures
// the handles are handles to gl textures
void copy_buffers_to_textures(u32 depth_handle, u32 color_handle, v2s window_dim) {
    glDrawBuffer(GL_BACK);
    glReadBuffer(GL_BACK); // Ensure we are reading from the back buffer.

    if (depth_handle != 0) {
        glBindTexture(GL_TEXTURE_2D, depth_handle);
        glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 0, 0, window_dim.x, window_dim.y, 0);
    }

    if (color_handle != 0) {
        glBindTexture(GL_TEXTURE_2D, color_handle);
        glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 0, 0, window_dim.x, window_dim.y, 0);
    }
}

//
// Options
//
 
void set_blend_function(u32 source_factor, u32 destination_factor)
{
    glBlendFunc(source_factor, destination_factor);
}

void set_polygon_mode(u32 mode)
{
    switch(mode)
    {
        case POLYGON_MODE_POINT: glPolygonMode(GL_FRONT_AND_BACK, GL_POINT); break;
        case POLYGON_MODE_LINE:  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);  break;
        case POLYGON_MODE_FILL:  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);  break;
    }
}

u32 capability_to_gl(u32 capability) {
    switch(capability)
    {
        case PLATFORM_CAPABILITY_DEPTH_TEST:   return GL_DEPTH_TEST;
        case PLATFORM_CAPABILITY_CULL_FACE:    return GL_CULL_FACE;
        case PLATFORM_CAPABILITY_SCISSOR_TEST: return GL_SCISSOR_TEST;
    }
    error("capability_to_gl(): No conversion available");
    return 0;
}

void opengl_set_capability(GLenum capability, b32 state)
{
    if (state) glEnable(capability);
    else       glDisable(capability);
}

void platform_set_capability(u32 capability, b32 state) {
    return opengl_set_capability(capability_to_gl(capability), state);
}

b32 platform_capability_enabled(u32 capability) {
    return glIsEnabled(capability_to_gl(capability));
}

void platform_set_depth_mask(b32 state)
{
    if (state) glDepthMask(GL_TRUE);
    else       glDepthMask(GL_FALSE);
}

void platform_set_scissor_box(v2s bottom_left, v2s dim) {
    glScissor(bottom_left.x, bottom_left.y, dim.x, dim.y);
}