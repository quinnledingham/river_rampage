// block index is from glUniformBlockBinding or binding == #
u32 init_uniform_buffer_object(u32 block_size, u32 block_index)
{
    u32 uniform_buffer_object;
    glGenBuffers(1, &uniform_buffer_object);
    
    glBindBuffer(GL_UNIFORM_BUFFER, uniform_buffer_object);
    glBufferData(GL_UNIFORM_BUFFER, block_size, NULL, GL_STATIC_DRAW);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
    
    glBindBufferBase(GL_UNIFORM_BUFFER, block_index, uniform_buffer_object);
    
    return uniform_buffer_object;
}

void platform_set_uniform_block_binding(u32 shader_handle, const char *tag, u32 index)
{
    u32 tag_uniform_block_index = glGetUniformBlockIndex(shader_handle, tag);
    glUniformBlockBinding(shader_handle, tag_uniform_block_index, index);
}

void platform_set_uniform_buffer_data(u32 ubo, u32 size, void *data)
{
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0, size, data);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

// returns the new offset
inline u32
buffer_sub_data(u32 target, u32 offset, u32 size, void *data) {
    glBufferSubData(target, offset, size, data);
    return (offset + size);
}

#define BUFFER_SUB_DATA(target, offset, n) buffer_sub_data(target, offset, sizeof(n), (void *)&n)

// functions to set matrices in uniform buffer
void orthographic(u32 ubo, Matrices *matrices)
{
    GLenum target = GL_UNIFORM_BUFFER;
    u32 offset = 0;

    glBindBuffer   (target, ubo);
    offset = BUFFER_SUB_DATA(target, offset, matrices->orthographic_matrix);
    offset = BUFFER_SUB_DATA(target, offset, identity_m4x4());
    glBindBuffer   (target, 0);
}

void perspective(u32 ubo, Matrices *matrices)
{
    GLenum target = GL_UNIFORM_BUFFER;
    u32 offset = 0;

    glBindBuffer   (target, ubo);
    offset = BUFFER_SUB_DATA(target, offset, matrices->perspective_matrix);
    offset = BUFFER_SUB_DATA(target, offset, matrices->view_matrix);
    offset = BUFFER_SUB_DATA(target, offset, matrices->p_near);
    offset = BUFFER_SUB_DATA(target, offset, matrices->p_far);
    offset = BUFFER_SUB_DATA(target, offset, matrices->window_width);
    offset = BUFFER_SUB_DATA(target, offset, matrices->window_height);
    glBindBuffer   (target, 0);
}

void platform_uniform_m4x4(u32 shader_handle, const char *tag, m4x4 *m) { glUniformMatrix4fv(glGetUniformLocation(shader_handle, tag), (GLsizei)1, false, (float*) m); }
void platform_uniform_f32 (u32 shader_handle, const char *tag, f32   f) { glUniform1f       (glGetUniformLocation(shader_handle, tag),                             f); }
void platform_uniform_s32 (u32 shader_handle, const char *tag, s32   i) { glUniform1i       (glGetUniformLocation(shader_handle, tag),                             i); }
void platform_uniform_v3  (u32 shader_handle, const char *tag, v3    v) { glUniform3fv      (glGetUniformLocation(shader_handle, tag), (GLsizei)1,        (float*)&v); }
void platform_uniform_v4  (u32 shader_handle, const char *tag, v4    v) { glUniform4fv      (glGetUniformLocation(shader_handle, tag), (GLsizei)1,        (float*)&v); }

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

void copy_buffers(u32 depth_handle, u32 color_handle, v2s window_dim) {
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

void platform_set_texture_cube_map(Cubemap *cubemap, u32 shader)
{
    //glDepthFunc(GL_LEQUAL);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap->handle);
    glUniform1i(glGetUniformLocation(shader, "skybox"), 0);
}

void platform_blend_function(u32 source_factor, u32 destination_factor)
{
    glBlendFunc(source_factor, destination_factor);
}

void platform_set_polygon_mode(u32 mode)
{
    switch(mode)
    {
        case PLATFORM_POLYGON_MODE_POINT: glPolygonMode(GL_FRONT_AND_BACK, GL_POINT); break;
        case PLATFORM_POLYGON_MODE_LINE:  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); break;
        case PLATFORM_POLYGON_MODE_FILL:  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); break;
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

void platform_bind_framebuffer(u32 handle) {
    glBindFramebuffer(GL_FRAMEBUFFER, handle);
}