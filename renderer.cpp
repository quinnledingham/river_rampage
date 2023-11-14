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

// functions to set matrices in uniform buffer
void orthographic(u32 ubo, Matrices *matrices)
{
    glBindBuffer(GL_UNIFORM_BUFFER, ubo);
    glBufferSubData(GL_UNIFORM_BUFFER, 0,            sizeof(m4x4), (void*)&matrices->orthographic_matrix);
    glBufferSubData(GL_UNIFORM_BUFFER, sizeof(m4x4), sizeof(m4x4), (void*)&identity_m4x4());
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void perspective(u32 ubo, Matrices *matrices)
{
    GLenum target = GL_UNIFORM_BUFFER;
    glBindBuffer(target, ubo);
    glBufferSubData(target, 0,            sizeof(m4x4), (void*)&matrices->perspective_matrix);
    glBufferSubData(target, sizeof(m4x4), sizeof(m4x4), (void*)&matrices->view_matrix);
    glBindBuffer(target, 0);
}

void platform_uniform_m4x4(u32 shader_handle, const char *tag, m4x4 *m) { glUniformMatrix4fv(glGetUniformLocation(shader_handle, tag), (GLsizei)1, false, (float*) m); }
void platform_uniform_f32 (u32 shader_handle, const char *tag, f32   f) { glUniform1f       (glGetUniformLocation(shader_handle, tag),                             f); }
void platform_uniform_s32 (u32 shader_handle, const char *tag, s32   i) { glUniform1i       (glGetUniformLocation(shader_handle, tag),                             i); }
void platform_uniform_v3  (u32 shader_handle, const char *tag, v3    v) { glUniform3fv      (glGetUniformLocation(shader_handle, tag), (GLsizei)1,        (float*)&v); }
void platform_uniform_v4  (u32 shader_handle, const char *tag, v4    v) { glUniform4fv      (glGetUniformLocation(shader_handle, tag), (GLsizei)1,        (float*)&v); }

void platform_set_texture(Bitmap *bitmap)
{
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, bitmap->handle);
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

void opengl_set_capability(GLenum capability, b32 state)
{
    if (state) glEnable(capability);
    else       glDisable(capability);
}

void platform_set_capability(u32 capability, b32 state)
{
    switch(capability)
    {
        case PLATFORM_CAPABILITY_DEPTH_TEST: opengl_set_capability(GL_DEPTH_TEST, state); break;
        case PLATFORM_CAPABILITY_CULL_FACE:  opengl_set_capability(GL_CULL_FACE, state);  break;
    }
}

void platform_set_depth_mask(b32 state)
{
    if (state) glDepthMask(GL_TRUE);
    else       glDepthMask(GL_FALSE);
}