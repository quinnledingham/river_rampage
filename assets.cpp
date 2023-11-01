//
// File
//

function File
read_file(const char *filename)
{
    File result = {};
    
    FILE *in = fopen(filename, "rb");
    if(in) 
    {
        fseek(in, 0, SEEK_END);
        result.size = ftell(in);
        fseek(in, 0, SEEK_SET);
        
        result.memory = SDL_malloc(result.size);
        fread(result.memory, result.size, 1, in);
        fclose(in);
    }
    else error(0, "Cannot open file %s", filename);
    
    return result;
}

function s32
get_char(File *file)
{
    if (file->ch == 0) file->ch = (const char*)file->memory;
    if (file->ch == (const char*)file->memory + file->size) return EOF;
    s32 ch = *file->ch;
    file->ch++;
    return ch;
}

function s32
previous_char(File *file)
{
    if (file->ch == 0) file->ch = (const char*)file->memory;
    const char *ptr = file->ch - 2;
    return *ptr;
}

function s32
peek_char(File *file)
{
    const char *ptr = file->ch;
    return *ptr;
}

function void
unget_char(File *file)
{
    file->ch--;
}

function void
reset_get_char(File *file)
{
    file->ch = (char*)file->memory;
}

function const char*
copy_last_num_of_chars(File *file, u32 length)
{
    char *string = (char*)SDL_malloc(length + 1);
    memset(string, 0, length + 1);
    
    const char *ptr = file->ch - length;
    for (int i = 0; i < length; i++)
    {
        int ch = *ptr++;
        if (ch == EOF)
        {
            warning(0, "copy_from_file hit the EOF");
            break;
        }
        string[i] = ch;
    }
    
    return string;
}

function void
free_file(File *file)
{
    SDL_free(file->memory);
    *file = {};
}

function void
write_file(File *file, const char *filename)
{
    FILE *in = fopen(filename, "wb");
    if(in) 
    {
        fwrite(file->memory, file->size, 1, in);
    }
    else error(0, "Cannot open file %s", filename);
    fclose(in);
}

//
// Bitmap
//

internal Bitmap
load_bitmap(const char *filename, b32 flip_on_load)
{
    if (flip_on_load) stbi_set_flip_vertically_on_load(true);
    else              stbi_set_flip_vertically_on_load(false);
    Bitmap bitmap = {};
    bitmap.memory = stbi_load(filename, &bitmap.dim.width, &bitmap.dim.height, &bitmap.channels, 0);
    if (bitmap.memory == 0) error("load_bitmap() could not load bitmap %s", filename);
    //log("file: %s %d %d %d\n", filename, bitmap.dim.width, bitmap.dim.height, bitmap.channels);
    bitmap.pitch = bitmap.dim.width * bitmap.channels;
    return bitmap;
}

internal Bitmap
load_bitmap(const char *filename)
{
    return load_bitmap(filename, true);
}

internal void
free_bitmap(Bitmap bitmap)
{
    stbi_image_free(bitmap.memory);
}

function void
init_bitmap_handle(Bitmap *bitmap)
{
    GLenum target = GL_TEXTURE_2D;
    
    glGenTextures(1, &bitmap->handle);
    glBindTexture(target, bitmap->handle);
    
    GLint internal_format = 0;
    GLenum data_format = 0;
    GLint pixel_unpack_alignment = 0;
    
    switch(bitmap->channels)
    {
        case 3:
        internal_format = GL_RGB;
        data_format = GL_RGB;
        pixel_unpack_alignment = 1;
        break;
        
        case 4:
        internal_format = GL_RGBA;
        data_format = GL_RGBA;
        pixel_unpack_alignment = 0;
        break;
    }
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, pixel_unpack_alignment);
    glTexImage2D(target, 0, internal_format, bitmap->dim.width, bitmap->dim.height, 0, data_format, GL_UNSIGNED_BYTE, bitmap->memory);
    glGenerateMipmap(target);
    
    // Tile
    glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    //glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    //glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    glBindTexture(target, 0);
}

function Bitmap
load_and_init_bitmap(const char *filename)
{
    Bitmap bitmap = load_bitmap(filename);
    init_bitmap_handle(&bitmap);
    return bitmap;
}

/*
GL_TEXTURE_CUBE_MAP_POSITIVE_X  Right
GL_TEXTURE_CUBE_MAP_NEGATIVE_X  Left
GL_TEXTURE_CUBE_MAP_POSITIVE_Y  Top
GL_TEXTURE_CUBE_MAP_NEGATIVE_Y  Bottom
GL_TEXTURE_CUBE_MAP_POSITIVE_Z  Back
GL_TEXTURE_CUBE_MAP_NEGATIVE_Z  Front
*/
Cubemap
load_cubemap()
{
    Cubemap cubemap = {};

    GLenum target = GL_TEXTURE_CUBE_MAP;
    GLint internal_format = GL_RGB;
    GLenum data_format = GL_RGB;
    GLint pixel_unpack_alignment = 1;

    glGenTextures(1, &cubemap.handle);
    glBindTexture(target, cubemap.handle);

    for (u32 i = 0; i < 6; i++)
    {
        cubemap.bitmaps[i] = load_bitmap(cubemap.filenames[i], false);
        Bitmap *bitmap = &cubemap.bitmaps[i];

        if (bitmap->memory == 0)
        {
            error("load_cubemap(): could not load %s", cubemap.filenames[i]);
            free_bitmap(*bitmap);
            continue;
        }
        //glPixelStorei(GL_UNPACK_ALIGNMENT, pixel_unpack_alignment);
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, internal_format, bitmap->dim.width, bitmap->dim.height, 0, data_format, GL_UNSIGNED_BYTE, bitmap->memory);
    }

    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return cubemap;
}

//
// Shader
//

function const char*
load_shader_file(const char* filename, u32 *file_size)
{
    FILE *file = fopen(filename, "rb");
    if (file == 0)
    {
        error("load_shader_file() could not open file");
        return 0;
    }
    
    fseek(file, 0, SEEK_END);
    u32 size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    char* shader_file = (char*)malloc(size + 1);
    fread(shader_file, size, 1, file);
    shader_file[size] = 0;
    fclose(file);
    
    *file_size = size + 1; // file_size includes the terminator
    
    return shader_file;
}

// loads the files
void load_shader(Shader *shader)
{
    if (shader->vs_filename == 0)
    {
        error("load_opengl_shader() must have a vertex shader");
        return;
    }
    
    // Free all files
    if (shader->vs_file  != 0) free((void*)shader->vs_file);
    if (shader->tcs_file != 0) free((void*)shader->tcs_file);
    if (shader->tes_file != 0) free((void*)shader->tes_file);
    if (shader->gs_file  != 0) free((void*)shader->gs_file);
    if (shader->fs_file  != 0) free((void*)shader->fs_file);
    
    // Load files
    if (shader->vs_filename  != 0) shader->vs_file  = load_shader_file(shader->vs_filename,  &shader->file_sizes[0]);
    if (shader->tcs_filename != 0) shader->tcs_file = load_shader_file(shader->tcs_filename, &shader->file_sizes[1]);
    if (shader->tes_filename != 0) shader->tes_file = load_shader_file(shader->tes_filename, &shader->file_sizes[2]);
    if (shader->gs_filename  != 0) shader->gs_file  = load_shader_file(shader->gs_filename,  &shader->file_sizes[3]);
    if (shader->fs_filename  != 0) shader->fs_file  = load_shader_file(shader->fs_filename,  &shader->file_sizes[4]);
}

bool compile_shader(u32 handle, const char *file, int type)
{
    u32 s =  glCreateShader((GLenum)type);
    glShaderSource(s, 1, &file, NULL);
    glCompileShader(s);
    
    GLint compiled_s = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &compiled_s);  
    if (!compiled_s)
    {
        opengl_debug(GL_SHADER, s);
    }
    else
    {
        glAttachShader(handle, s);
    }
    
    glDeleteShader(s);
    
    return compiled_s;
}

// compiles the files
void compile_shader(Shader *shader)
{
    shader->uniform_buffer_objects_generated = false;
    shader->compiled = false;
    if (shader->handle != 0) glDeleteProgram(shader->handle);
    shader->handle = glCreateProgram();
    
    if (shader->vs_file == 0) error("vertex shader required");
    
    if (shader->vs_file  != 0) if (!compile_shader(shader->handle, shader->vs_file,  GL_VERTEX_SHADER))          { error("compile_shader() could not compile vertex shader");          return; }
    if (shader->tcs_file != 0) if (!compile_shader(shader->handle, shader->tcs_file, GL_TESS_CONTROL_SHADER))    { error("compile_shader() could not compile tess controll shader");   return; }
    if (shader->tes_file != 0) if (!compile_shader(shader->handle, shader->tes_file, GL_TESS_EVALUATION_SHADER)) { error("compile_shader() could not compile tess evaluation shader"); return; }
    if (shader->gs_file  != 0) if (!compile_shader(shader->handle, shader->gs_file,  GL_GEOMETRY_SHADER))        { error("compile_shader() could not compile geometry shader");        return; }
    if (shader->fs_file  != 0) if (!compile_shader(shader->handle, shader->fs_file,  GL_FRAGMENT_SHADER))        { error("compile_shader() could not compile fragment shader");        return; }
    
    // Link
    glLinkProgram(shader->handle);
    GLint linked_program = 0;
    glGetProgramiv(shader->handle, GL_LINK_STATUS, &linked_program);
    if (!linked_program)
    {
        opengl_debug(GL_PROGRAM, shader->handle);
        error("compile_shader() link failed");
        return;
    }
    
    shader->compiled = true;
    log("compiled shader with vs file %s", shader->vs_filename);
}

u32 use_shader(Shader *shader)
{
    glUseProgram(shader->handle);
    return shader->handle;
}

//
// Mesh
//

void init_mesh(Mesh *mesh)
{
    glGenVertexArrays(1, &mesh->vao);
    glGenBuffers(1, &mesh->vbo);
    glGenBuffers(1, &mesh->ebo);
    
    glBindVertexArray(mesh->vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBufferData(GL_ARRAY_BUFFER, mesh->vertices_count * sizeof(Vertex), &mesh->vertices[0], GL_STATIC_DRAW);  
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->indices_count * sizeof(u32), &mesh->indices[0], GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0); // vertex positions
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(1); // vertex normals
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(2); // vertex texture coords
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texture_coordinate));
    
    glBindVertexArray(0);
}

void draw_mesh(Mesh *mesh)
{
    glBindVertexArray(mesh->vao);
    glDrawElements(GL_TRIANGLES, mesh->indices_count, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void draw_mesh_patches(Mesh *mesh)
{
    glBindVertexArray(mesh->vao);
    glDrawElements(GL_PATCHES, mesh->indices_count, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void draw_mesh_instanced(Mesh *mesh)
{
    glBindVertexArray(mesh->vao);
    glDrawElementsInstanced(GL_TRIANGLES, mesh->indices_count, GL_UNSIGNED_INT, 0, 10);
    glBindVertexArray(0);
}

//
// Font
//

function Font
load_font(const char *filename)
{
    Font font = {};
    SDL_memset(font.font_scales,  0, sizeof(Font_Scale)  * ARRAY_COUNT(font.font_scales));
    SDL_memset(font.font_chars,   0, sizeof(Font_Char)   * ARRAY_COUNT(font.font_chars));
    SDL_memset(font.font_strings, 0, sizeof(Font_String) * ARRAY_COUNT(font.font_strings));
    font.file = read_file(filename);
    
    font.info = SDL_malloc(sizeof(stbtt_fontinfo));
    stbtt_fontinfo *info = (stbtt_fontinfo*)font.info;
    *info = {};
    
    stbtt_InitFont(info, (u8*)font.file.memory, stbtt_GetFontOffsetForIndex((u8*)font.file.memory, 0));
    return font;
}

function Font_Char*
load_font_char(Font *font, u32 codepoint, f32 scale, v4 color)
{
    stbtt_fontinfo *info = (stbtt_fontinfo*)font->info;

    // search cache for font char
    for (s32 i = 0; i < font->font_chars_cached; i++)
    {
        Font_Char *font_char = &font->font_chars[i];
        if (font_char->codepoint == codepoint && font_char->color == color && font_char->scale == scale)
            return font_char;
    }
    
    // where to cache new font char
    Font_Char *font_char = &font->font_chars[font->font_chars_cached];
    memset(font_char, 0, sizeof(Font_Char));
    if (font->font_chars_cached + 1 < (s32)ARRAY_COUNT(font->font_chars))
        font->font_chars_cached++;
    else
        font->font_chars_cached = 0;
    
    font_char->codepoint = codepoint;
    font_char->scale = scale;
    font_char->color = color;
    
    // how wide is this character
    stbtt_GetCodepointHMetrics(info, font_char->codepoint, &font_char->ax, &font_char->lsb);
    
    // get bounding box for character (may be offset to account for chars that dip above or below the line
    stbtt_GetCodepointBitmapBox(info, font_char->codepoint, font_char->scale, font_char->scale, 
                                &font_char->c_x1, &font_char->c_y1, &font_char->c_x2, &font_char->c_y2);
    
    u8 *mono_bitmap = stbtt_GetCodepointBitmap(info, 0, scale, codepoint, 
                                               &font_char->bitmap.dim.width, &font_char->bitmap.dim.height,
                                               0, 0);
    font_char->bitmap.channels = 4;
    font_char->bitmap.memory = (u8*)SDL_malloc(font_char->bitmap.dim.width * 
                                               font_char->bitmap.dim.height * 
                                               font_char->bitmap.channels);
    u32 *dest = (u32*)font_char->bitmap.memory;
    for (s32 x = 0; x < font_char->bitmap.dim.width; x++)
    {
        for (s32 y = 0; y < font_char->bitmap.dim.height; y++)
        {
            u8 alpha = *mono_bitmap++;
            u32 real_alpha = u32((f32)alpha * color.a);
            *dest++ = ((real_alpha << 24) | ((u32)color.r << 16) | ((u32)color.g <<  8) | ((u32)color.b <<  0));
        }
    }
    
    //free(mono_bitmap);
    
    init_bitmap_handle(&font_char->bitmap);
    
    return font_char;
}

v2 get_string_dim(Font *font, const char *string, f32 pixel_height, v4 color)
{
    stbtt_fontinfo *info = (stbtt_fontinfo*)font->info;

    v2 dim = {};
    f32 scale = stbtt_ScaleForPixelHeight(info, pixel_height);
    
    u32 i = 0;
    while (string[i] != 0)
    {
        Font_Char *font_char = load_font_char(font, string[i], scale, color);
        
        f32 y = -1.0f * (r32)font_char->c_y1;
        if (dim.y < y) dim.y = y;
        
        int kern = stbtt_GetCodepointKernAdvance(info, string[i], string[i + 1]);
        dim.x += ((kern + font_char->ax) * scale);
        
        i++;
    }
    
    return dim;
}

//
// Audio
//

function void
print_audio_spec(SDL_AudioSpec *audio_spec)
{
    log("freq %d",     audio_spec->freq);
    log("format %d %d", SDL_AUDIO_BITSIZE(audio_spec->format), SDL_AUDIO_ISSIGNED(audio_spec->format));
    log("channels %d", audio_spec->channels);
    log("silence %d",  audio_spec->silence);
    log("samples %d",  audio_spec->samples);
    log("size %d",     audio_spec->size);
}

function void
print_audio_device_status(SDL_AudioDeviceID dev)
{
    printf("audio device id: %d, ", dev);
    switch (SDL_GetAudioDeviceStatus(dev))
    {
        case SDL_AUDIO_STOPPED: printf("stopped\n"); break;
        case SDL_AUDIO_PLAYING: printf("playing\n"); break;
        case SDL_AUDIO_PAUSED:  printf("paused\n");  break;
        default: printf("???"); break;
    }
}

function void
init_audio_player(Audio_Player *player)
{
    // printing all drivers and devices
    for (s32 i = 0; i < SDL_GetNumAudioDrivers(); i++) log("Audio driver %d: %s", i, SDL_GetAudioDriver(i));
    const char* driver_name = SDL_GetCurrentAudioDriver();
    if (driver_name) log("Audio subsystem initialized; driver = %s.", driver_name);
    else             log("Audio subsystem not initialized.");
    
    u32 num_audio_devices = SDL_GetNumAudioDevices(0);
    for (u32 i = 0; i < num_audio_devices; i++) log("Audio device %d: %s", i, SDL_GetAudioDeviceName(i, 0));
    
    // opening audio device
    SDL_AudioSpec desired;
    SDL_AudioSpec obtained;
    desired.freq = 48000;
    desired.format = AUDIO_S16;
    desired.callback = 0;
    
#if WINDOWS
    SDL_AudioSpec spec;
    char *device_name = 0;
    SDL_GetDefaultAudioInfo(&device_name, &spec, 0);
    SDL_AudioDeviceID device_id = SDL_OpenAudioDevice(device_name, 0, &desired, &obtained, 0);
    if (device_name) log("Audio device selected = %s.", device_name);
    else             log("Audio device not selected.");
    log("device audio spec:");
    print_audio_spec(&spec);
#elif LINUX
    SDL_AudioDeviceID device_id = SDL_OpenAudioDevice(NULL, 0, &desired, &obtained, 0);
#endif
    SDL_PauseAudioDevice(device_id, 0);
    
    log("obtained spec:");
    print_audio_spec(&obtained);
    
    player->max_length = 10000;
    player->buffer = (u8*)SDL_malloc(player->max_length);
    SDL_memset(player->buffer, 0, player->max_length);
    
    player->audios_count = 10;
    player->sound_volume = 0.5f;
    player->music_volume = 0.5f;
}

function Audio
load_audio(const char *filename)
{
    Audio audio = {};
    SDL_AudioSpec spec = {};
    SDL_LoadWAV(filename, &spec, &audio.buffer, &audio.length);
    //print_audio_spec(&audio.spec);
    return audio;
}

function void
play_audio(Audio_Player *player, Audio *audio, u32 type)
{
    Playing_Audio *playing_audio = 0;
    
    switch(type)
    {
        case AUDIO_SOUND:
        {
            for (u32 i = 1; i < player->audios_count; i++)
            {
                if (player->audios[i].length_remaining <= 0) { playing_audio = &player->audios[i]; break; }
            }
        } break;
        
        case AUDIO_MUSIC: playing_audio = &player->audios[0]; break;
    }
    
    if (playing_audio == 0) { error("play_audio(): trying to play too many"); return; }
    
    playing_audio->position = audio->buffer;
    playing_audio->length_remaining = audio->length;
    playing_audio->type = type;
}

function void
mix_audio(Audio_Player *player, r32 frame_time_s)
{
    // Delays in audio playing that I experienced before were probably caused by queueing too much audio
    // so that the audio with the new sound in it wouldnt play until what was queued has been played.
    r32 samples_per_second = 48000.0f; // frequency
    u32 sample_count = (int)floor(frame_time_s * samples_per_second);
    u32 bytes_to_copy = sample_count * 4; // 2 bytes per 2 channels for each sample
    
    player->length = 0;
    for (u32 i = 0; i < player->audios_count; i++)
    {
        Playing_Audio *audio = &player->audios[i];
        if (audio->length_remaining <= 0) continue;
        if (audio->length_remaining < bytes_to_copy) bytes_to_copy = audio->length_remaining;
        if (bytes_to_copy > player->max_length && player->audios_count != 0) 
        { 
            error("queue_audio buffer not big enough for %d bytes", bytes_to_copy); 
            return; 
        }
        
        // Set volume
        r32 volume = 0.5f;
        switch(audio->type)
        {
            case AUDIO_SOUND: volume = player->sound_volume; break;
            case AUDIO_MUSIC: volume = player->music_volume; break;
        }
        
        // Mix into buffer
        u32 buffer_index = 0;
        while(buffer_index < bytes_to_copy)
        {
            s16 *buffer = (s16*)&player->buffer[buffer_index];
            s16 *source = (s16*)&audio->position[buffer_index];
            *buffer += s16((r32)*source * volume);
            buffer_index += 2; // move two bytes
        }
        
        audio->position += bytes_to_copy;
        audio->length_remaining -= bytes_to_copy;
        if (bytes_to_copy > player->length) player->length = bytes_to_copy;
        //log("%d %d %d %d", bytes_to_copy, audio->length_remaining, player->audios_count, u8((r32)audio->position[0] * 1.0f));
    }
}

//
// Model
//

void draw_model(Shader *shader, Shader *tex_shader, Model *model, Light_Source light, Camera camera, v3 position, quat rotation)
{
    v3 light_ambient = { 0.2, 0.2, 0.2 };
    v3 light_diffuse = { 0.9, 0.9, 0.9 };
    v3 light_specular = { 1, 1, 1 };

    u32 shader_enabled = 0; // 0 no shader, 1 shader, 2 tex_shader
    u32 handle = 0;
    //u32 handle = use_shader(shader);

    //m4x4 model_matrix = create_transform_m4x4(position, rotation, {1, 1, 1});
    //glUniformMatrix4fv(glGetUniformLocation(handle, "model"), (GLsizei)1, false, (float*)&model_matrix);
    //glUniform3fv(glGetUniformLocation(handle, "viewPos"), (GLsizei)1, (float*)&camera.position);    
    glBindTexture(GL_TEXTURE_2D, 0);

    for (s32 i = 0; i < model->meshes_count; i++)
    {
        if (shader_enabled != 1 && model->meshes[i].material.diffuse_map.memory == 0)
        {
            shader_enabled = 1;
            handle = use_shader(shader);

            m4x4 model_matrix = create_transform_m4x4(position, rotation, {1, 1, 1});
            glUniformMatrix4fv(glGetUniformLocation(handle, "model"), (GLsizei)1, false, (float*)&model_matrix);
            glUniform3fv(glGetUniformLocation(handle, "viewPos"), (GLsizei)1, (float*)&camera.position);  
        }
        else if (shader_enabled != 2 && model->meshes[i].material.diffuse_map.memory != 0)
        {
            shader_enabled = 2;
            handle = use_shader(tex_shader);

            m4x4 model_matrix = create_transform_m4x4(position, rotation, {1, 1, 1});
            glUniformMatrix4fv(glGetUniformLocation(handle, "model"), (GLsizei)1, false, (float*)&model_matrix);
            glUniform3fv(glGetUniformLocation(handle, "viewPos"), (GLsizei)1, (float*)&camera.position);    
        }

        glUniform3fv(glGetUniformLocation(handle, "material.ambient"), (GLsizei)1, (float*)&model->meshes[i].material.ambient);
        glUniform3fv(glGetUniformLocation(handle, "material.diffuse"), (GLsizei)1, (float*)&model->meshes[i].material.diffuse);
        glUniform3fv(glGetUniformLocation(handle, "material.specular"), (GLsizei)1, (float*)&model->meshes[i].material.specular);
        glUniform1f(glGetUniformLocation(handle, "material.shininess"), model->meshes[i].material.specular_exponent);
        
        glUniform3fv(glGetUniformLocation(handle, "light.position"), (GLsizei)1, (float*)&light.position);
        glUniform3fv(glGetUniformLocation(handle, "light.ambient"), (GLsizei)1, (float*)&light_ambient);
        glUniform3fv(glGetUniformLocation(handle, "light.diffuse"), (GLsizei)1, (float*)&light_diffuse);
        glUniform3fv(glGetUniformLocation(handle, "light.specular"), (GLsizei)1, (float*)&light_specular);
        
        if (model->meshes[i].material.diffuse_map.memory != 0)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, model->meshes[i].material.diffuse_map.handle);
        }
        
        draw_mesh(&model->meshes[i]);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    
}

// OBJ

#include "mtl.cpp"

enum OBJ_Token_Type
{
    OBJ_TOKEN_ID,
    OBJ_TOKEN_NUMBER,
    
    OBJ_TOKEN_VERTEX,
    OBJ_TOKEN_NORMAL,
    OBJ_TOKEN_TEXTURE_COORD,
    OBJ_TOKEN_FACE,
    OBJ_TOKEN_LINE,
    OBJ_TOKEN_USEMTL,
    OBJ_TOKEN_MTLLIB,
    
    OBJ_TOKEN_ERROR,
    OBJ_TOKEN_EOF
};

struct OBJ_Token
{
    s32 type;
    const char *lexeme;
    s32 ch; // char when the token is created. Used for debugging
};

struct Face_Vertex
{
    u32 position_index;
    u32 uv_index;
    u32 normal_index;
};

struct Obj
{
    u32 vertices_count;
    u32 uvs_count;
    u32 normals_count;
    u32 faces_count;
    u32 lines_count;
    u32 meshes_count;
    
    v3 *vertices;
    v2 *uvs;
    v3 *normals;
    Face_Vertex *face_vertices; // array of all the faces vertexs
    u32 *meshes_face_count; // the amount of faces in each mesh
    
    const char *material_filename;
};

s32 obj_valid_chars[5] = { '-', '.', '_', ':', '\\' };

function b8
is_valid_char(s32 ch)
{
    for (s32 i = 0; i < ARRAY_COUNT(obj_valid_chars); i++)
    {
        if (ch == obj_valid_chars[i]) return true;
    }
    return false;
}

function b8
scan_is_string(File *file, s32 *ch, const char *str, u32 str_length)
{
    b8 is_string = true;
    for (u32 ch_index = 0; ch_index < str_length; ch_index++)
    {
        if (*ch != str[ch_index]) is_string = false;
        *ch = get_char(file);
    }
    
    return is_string;
}

// strings = true if you want it to return filenames and numbers
// doesn't malloc any memory unless strings = true
// buffer max length = 40
function OBJ_Token
scan_obj(File *file, s32 *line_num, b8 strings, char *buffer)
{
    X:
    s32 ch;
    while((ch = get_char(file)) != EOF && (ch == 9 || ch == 13 || ch == ' ' || ch == '/' || ch == '\n')); // remove tabs
    
    switch(ch)
    {
        case EOF: { return { OBJ_TOKEN_EOF, 0, ch }; } break;
        
        /*
        case '\n':
        {
            (*line_num)++;
            goto X;
        } break;
        */

        case '#':
        {
            while((ch = get_char(file)) != EOF && (ch != '\n'));
            unget_char(file);
            goto X;
        } break;
        
        default:
        {
            s32 last_ch = previous_char(file);

            if (isalpha(ch) && last_ch == '\n') // only after a newline
            {
                s32 next_ch = peek_char(file);

                if (ch == 'v')
                {
                    ch = get_char(file);
                    next_ch = peek_char(file);

                    if      (ch == 'n' && next_ch == ' ') return { OBJ_TOKEN_NORMAL, "vn", ch };
                    else if (ch == 't' && next_ch == ' ') return { OBJ_TOKEN_TEXTURE_COORD, "vt", ch };
                    else if (ch == ' ') return { OBJ_TOKEN_VERTEX, "v", ch };
                }
                else if (ch == 'f' && next_ch == ' ') return { OBJ_TOKEN_FACE, "f", ch };
                else if (ch == 'l' && next_ch == ' ') return { OBJ_TOKEN_LINE, "l", ch };
                else if (ch == 'm') { if (scan_is_string(file, &ch, "mtllib", 6)) return { OBJ_TOKEN_MTLLIB, "mtllib", ch }; }
                else if (ch == 'u') { if (scan_is_string(file, &ch, "usemtl", 6)) return { OBJ_TOKEN_USEMTL, "usemtl", ch }; }
            }
            
            if (strings)
            {
                int length = 0;
                buffer[length] = ch;
                do
                {
                    if (length >= 40) return { OBJ_TOKEN_ERROR, "error", ch };
                    buffer[length++] = ch;
                    ch = get_char(file);
                } while((isalpha(ch) || isdigit(ch) || is_valid_char(ch)) && ch != ' ' && ch != '/' && ch != EOF);
                buffer[length] = 0;
                return { OBJ_TOKEN_ID, buffer, ch };
            }
            
            goto X;
        } break;
    }
    
    return { OBJ_TOKEN_ERROR, "error", ch };
}

function const char*
skip_whitespace(const char *ptr)
{
    X:
    while(*ptr != EOF && (*ptr == 9 || *ptr == 13 || *ptr == ' ' || *ptr == '/' || *ptr == '\n'))
    {
        ptr++;
    }

    if (*ptr == '#')
    {
        while(*ptr != EOF && (*ptr != '\n')) { ptr++; }
        goto X;
    }

    return ptr;
}

internal void
parse_v3_obj(File *file, s32 *line_num, v3 *v)
{    
    file->ch = (char*)skip_whitespace(file->ch);
    file->ch = (char*)char_array_to_f32(file->ch, &v->x);
    file->ch = (char*)skip_whitespace(file->ch);
    file->ch = (char*)char_array_to_f32(file->ch, &v->y);
    file->ch = (char*)skip_whitespace(file->ch);
    file->ch = (char*)char_array_to_f32(file->ch, &v->z);
    file->ch = (char*)skip_whitespace(file->ch);
}

internal void
parse_face_vertex(File *file, s32 *line_num, Face_Vertex *f)
{
    file->ch = (char*)skip_whitespace(file->ch);
    file->ch = (char*)char_array_to_u32(file->ch, &f->position_index);
    file->ch = (char*)skip_whitespace(file->ch);
    file->ch = (char*)char_array_to_u32(file->ch, &f->uv_index);
    file->ch = (char*)skip_whitespace(file->ch);
    file->ch = (char*)char_array_to_u32(file->ch, &f->normal_index);
}

Model load_obj(const char *path, const char *filename)
{
    Model model = {};
    Obj obj = {};
    
    char filepath[80];
    memset(filepath, 0, 80);
    strcat(strcat(filepath, path), filename);
    
    File file = read_file(filepath);
    if (!file.size) { error("load_obj: could not read object file"); return model; }

    // count components
    {
        char buffer[40];
        OBJ_Token token = {};
        s32 line_num = 1;
        do
        {
            token = scan_obj(&file, &line_num, false, buffer);
            
            switch(token.type)
            {
                case OBJ_TOKEN_VERTEX:        obj.vertices_count++; break;
                case OBJ_TOKEN_NORMAL:        obj.normals_count++;  break;
                case OBJ_TOKEN_TEXTURE_COORD: obj.uvs_count++;      break;
                case OBJ_TOKEN_FACE:          obj.faces_count++;    break;
                case OBJ_TOKEN_LINE:          obj.lines_count++;    break;
                case OBJ_TOKEN_USEMTL:        obj.meshes_count++;   break;
            }
            
        } while(token.type != OBJ_TOKEN_EOF);
    }
    
    // allocate space for the components
    obj.vertices          = ARRAY_MALLOC(v3, obj.vertices_count);
    obj.uvs               = ARRAY_MALLOC(v2, obj.uvs_count);
    obj.normals           = ARRAY_MALLOC(v3, obj.normals_count);
    obj.face_vertices     = ARRAY_MALLOC(Face_Vertex, obj.faces_count * 3);
    obj.meshes_face_count = ARRAY_MALLOC(u32, obj.meshes_count);
    memset(obj.meshes_face_count, 0, sizeof(u32) * obj.meshes_count); // ++ to count so need it to start at zero
    
    model.meshes_count = obj.meshes_count;
    model.meshes = ARRAY_MALLOC(Mesh, model.meshes_count);
    
    reset_get_char(&file);
    
    // fill in component arrays
    {
        char buffer[40];
        OBJ_Token last_token = {};
        OBJ_Token token = {};
        s32 line_num = 1;
        
        u32 meshes_index        = -1;
        u32 vertices_index      = 0;
        u32 uvs_index           = 0;
        u32 normals_index       = 0;
        u32 face_vertices_index = 0;
        /*
        while(*file.ch != EOF)
        {
            file.ch = (char*)skip_whitespace(file.ch);
            if (*file.ch == 'v')
            {
                file.ch++;
                if (*file.ch == ' ' || *file.ch == '\t')
                {
                    file.ch++;
                    parse_v3_obj(&file, &line_num, &obj.vertices[vertices_index++]);
                }
                else if (*file.ch == 'n') 
                {
                    file.ch++;
                    parse_v3_obj(&file, &line_num, &obj.normals[normals_index++]);
                }
                else if (*file.ch == 't')
                {
                    file.ch++;
                    file.ch = (char*)parse_float(file.ch, &obj.uvs[uvs_index].x);
                    file.ch = (char*)parse_float(file.ch, &obj.uvs[uvs_index].y);
                    uvs_index++;
                }
            }
            else if (*file.ch == 'f')
            {
                file.ch++;
                if (*file.ch == ' ' || *file.ch == '\t')
                {
                    file.ch++;
                    parse_face_vertex(&file, &line_num, &obj.face_vertices[face_vertices_index++]);
                    parse_face_vertex(&file, &line_num, &obj.face_vertices[face_vertices_index++]);
                    parse_face_vertex(&file, &line_num, &obj.face_vertices[face_vertices_index++]);
                    obj.meshes_face_count[meshes_index]++;
                }
            }
            else if (*file.ch == 'l')
            {
                scan_obj(&file, &line_num, true, buffer);
                scan_obj(&file, &line_num, true, buffer);
            }
            else if (*file.ch == 'm')
            {
                file.ch++;
                if (file.ch[0] == 't' && file.ch[1] == 'l' && file.ch[2] == 'l' && 
                    file.ch[3] == 'i' && file.ch[4] == 'b' && file.ch[5] == ' ')
                {
                    file.ch += 5;

                    token = scan_obj(&file, &line_num, true, buffer);
                    obj.material_filename = string_malloc(token.lexeme);
                }
            }
            else if (*file.ch == 'u')
            {
                file.ch++;
                if (file.ch[0] == 's' && file.ch[1] == 'e' && file.ch[2] == 'm' && 
                    file.ch[3] == 't' && file.ch[4] == 'l' && file.ch[5] == ' ')
                {
                    file.ch += 5;

                    meshes_index++;
                    token = scan_obj(&file, &line_num, true, buffer);
                    model.meshes[meshes_index].material.id = string_malloc(token.lexeme);
                }
            }

            while(*file.ch != EOF && *file.ch != '\n')
            {
                file.ch++;
            }
        }
        */
        
        do
        {
            last_token = token;
            token = scan_obj(&file, &line_num, false, buffer);
            
            if      (token.type == OBJ_TOKEN_VERTEX) parse_v3_obj(&file, &line_num, &obj.vertices[vertices_index++]);
            else if (token.type == OBJ_TOKEN_NORMAL) parse_v3_obj(&file, &line_num, &obj.normals[normals_index++]);
            else if (token.type == OBJ_TOKEN_TEXTURE_COORD)
            {
                file.ch = (char*)skip_whitespace(file.ch);
                file.ch = (char*)char_array_to_f32(file.ch, &obj.uvs[uvs_index].x);
                file.ch = (char*)skip_whitespace(file.ch);
                file.ch = (char*)char_array_to_f32(file.ch, &obj.uvs[uvs_index].y);
                file.ch = (char*)skip_whitespace(file.ch);
                uvs_index++;
            }
            else if (token.type == OBJ_TOKEN_FACE)
            {
                parse_face_vertex(&file, &line_num, &obj.face_vertices[face_vertices_index++]);
                parse_face_vertex(&file, &line_num, &obj.face_vertices[face_vertices_index++]);
                parse_face_vertex(&file, &line_num, &obj.face_vertices[face_vertices_index++]);
                
                obj.meshes_face_count[meshes_index]++;
            }
            else if (token.type == OBJ_TOKEN_LINE)
            {
                token = scan_obj(&file, &line_num, true, buffer);
                token = scan_obj(&file, &line_num, true, buffer);
            }
            else if (token.type == OBJ_TOKEN_MTLLIB) // get the material file to load
            {
                token = scan_obj(&file, &line_num, true, buffer);
                obj.material_filename = string_malloc(token.lexeme);
            }
            else if (token.type == OBJ_TOKEN_USEMTL) // different mesh every time the material is changed
            {
                meshes_index++;
                token = scan_obj(&file, &line_num, true, buffer);
                model.meshes[meshes_index].material.id = string_malloc(token.lexeme);
            }
        } while(token.type != OBJ_TOKEN_EOF);
        
    }
    
    free_file(&file);
    
    Mtl mtl = load_mtl(path, obj.material_filename);
    
    // creating the model's meshes
    {
        s32 lower_range = 0;
        for (s32 mesh_index = 0; mesh_index < model.meshes_count; mesh_index++)
        {
            Mesh *mesh = &model.meshes[mesh_index];
            mesh->vertices_count = 0;
            
            s32 mesh_face_vertices_count = obj.meshes_face_count[mesh_index] * 3;
            
            mesh->indices_count = mesh_face_vertices_count;
            mesh->indices = ARRAY_MALLOC(u32, mesh->indices_count);
            
            mesh->vertices_count = mesh_face_vertices_count;
            mesh->vertices = ARRAY_MALLOC(Vertex, mesh->vertices_count);
            
            // put unique face vertices in mesh vertices array
            u32 vertices_index = 0;
            u32 indices_index = 0;
            for (s32 i = lower_range; i < mesh_face_vertices_count + lower_range; i++)
            {
                mesh->indices[indices_index++] = vertices_index;
                
                Vertex *vertex = &mesh->vertices[vertices_index++];
                vertex->position           = obj.vertices[obj.face_vertices[i].position_index - 1];
                vertex->normal             = obj.normals [obj.face_vertices[i].normal_index   - 1];
                vertex->texture_coordinate = obj.uvs     [obj.face_vertices[i].uv_index       - 1];
            }
            
            // assign material
            for (s32 i = 0; i < mtl.materials_count; i++)
            {
                if (equal(mesh->material.id, mtl.materials[i].id)) 
                    mesh->material = mtl.materials[i];
            }
            
            init_mesh(mesh);
            
            lower_range += mesh_face_vertices_count;
        }
    }
    
    SDL_free(mtl.materials);
    
    SDL_free(obj.vertices);
    SDL_free(obj.uvs);
    SDL_free(obj.normals);
    SDL_free(obj.face_vertices);
    SDL_free(obj.meshes_face_count);
    
    return model;
}

//
// Asset File Reading
//

function Asset_Token
scan_asset_file(File *file, s32 *line_num, Asset_Token last_token)
{
    X:
    
    s32 ch;
    while((ch = get_char(file)) != EOF && (ch == 32 || ch == 9 || ch == 13)); // remove whitespace
    
    switch(ch)
    {
        case EOF:
        {
            return { -1, 0 };
        } break;
        
        case '\n':
        {
            (*line_num)++;
            goto X;
        } break;
        
        case ':':
        case ',':
        case '|':
        {
            return { ATT_SEPERATOR, chtos(1, ch) };
        } break;
        
        default:
        {
            if (is_valid_start_ch(ch)) // must start with valid start ch
            {
                int length = 0;
                do
                {
                    ch = get_char(file);
                    length++;
                } while(is_valid_body_ch(ch));
                unget_char(file);
                const char *sequence = copy_last_num_of_chars(file, length);
                if (is_asset_keyword(sequence)) return { ATT_KEYWORD, sequence };
                return { ATT_ID, sequence };
            }
            
            error(*line_num, "not a valid ch");
        } break;
    }
    
    return { ATT_ERROR, 0 };
}

// action is what happens when all the parts of an asset are found
function void
parse_asset_file(Assets *assets, File *file, void (action)(void *data, void *args))
{
    Asset_Load_Info info = {};
    
    u32 shader_type = 0;
    const char *shader_tag = 0;
    
    Asset_Token last_token = {};
    Asset_Token tok = {};
    s32 line_num = 1;
    while (tok.type != -1)
    {
        last_token = tok;
        tok = scan_asset_file(file, &line_num, tok);
        //printf("%d, %s\n", tok.type, tok.lexeme);
        
        if (tok.type == ATT_KEYWORD)
        {
            u32 type = 0;
            if      (equal(tok.lexeme, "FONTS"))   type = ASSET_TYPE_FONT;
            else if (equal(tok.lexeme, "BITMAPS")) type = ASSET_TYPE_BITMAP;
            else if (equal(tok.lexeme, "SHADERS")) type = ASSET_TYPE_SHADER;
            else if (equal(tok.lexeme, "AUDIOS"))  type = ASSET_TYPE_AUDIO;
            
            // add last shader
            if (shader_tag != 0 && type != ASSET_TYPE_SHADER)
            {
                action((void*)assets, (void*)&info);
                shader_tag = 0;
            }
            
            info.type = type;
            
            tok = scan_asset_file(file, &line_num, tok);
            if (!equal(tok.lexeme, ":")) 
            {
                error(line_num, "expected ':' (got %c)", tok.lexeme);
                break;
            }
        }
        else if (tok.type == ATT_ID)
        {
            if (info.type == ASSET_TYPE_SHADER)
            {
                if (equal(last_token.lexeme, ",")) // all that is left is the filename
                {
                    switch(shader_type)
                    {
                        case VERTEX_SHADER:                  info.filename     = tok.lexeme; break;
                        case TESSELLATION_CONTROL_SHADER:    info.tcs_filename = tok.lexeme; break;
                        case TESSELLATION_EVALUATION_SHADER: info.tes_filename = tok.lexeme; break;
                        case GEOMETRY_SHADER:                info.gs_filename  = tok.lexeme; break;
                        case FRAGMENT_SHADER:                info.fs_filename  = tok.lexeme; break;
                    }
                }
                else if (equal(last_token.lexeme, "|")) // shader part needs to be grabbed
                {
                    if      (equal(tok.lexeme, "VERTEX"))     shader_type = VERTEX_SHADER;
                    else if (equal(tok.lexeme, "CONTROL"))    shader_type = TESSELLATION_CONTROL_SHADER;
                    else if (equal(tok.lexeme, "EVALUATION")) shader_type = TESSELLATION_EVALUATION_SHADER;
                    else if (equal(tok.lexeme, "GEOMETRY"))   shader_type = GEOMETRY_SHADER;
                    else if (equal(tok.lexeme, "FRAGMENT"))   shader_type = FRAGMENT_SHADER;
                    
                    // check that comma comes after
                    tok = scan_asset_file(file, &line_num, tok);
                    if (!equal(tok.lexeme, ",")) 
                    {
                        error(line_num, "expected ','");
                        break;
                    }
                }
                else // first element
                {
                    info.tag = tok.lexeme;
                    
                    if (shader_tag == 0) shader_tag = info.tag;
                    else if (!equal(shader_tag, info.tag)) 
                    {
                        const char *new_tag = info.tag;
                        info.tag = shader_tag;
                        action((void*)assets, (void*)&info);
                        info = {};
                        info.type = ASSET_TYPE_SHADER;
                        info.tag = new_tag;
                        shader_tag = info.tag;
                    }
                    
                    // check that | comes after
                    tok = scan_asset_file(file, &line_num, tok);
                    if (!equal(tok.lexeme, "|")) 
                    {
                        error(line_num, "expected '|'");
                        break;
                    }
                }
            }
            else // Fonts, Bitmaps, Audios
            {
                if (!equal(last_token.lexeme, ","))
                {
                    info.tag = tok.lexeme;
                    
                    // check thst comma comes after
                    tok = scan_asset_file(file, &line_num, tok);
                    if (!equal(tok.lexeme, ",")) 
                    {
                        error(line_num, "expected ','");
                        break;
                    }
                }
                else
                {
                    info.filename = tok.lexeme;
                    action((void*)assets, (void*)&info);
                }
            }
        }
        else if (tok.type == ATT_SEPERATOR)
        {
            error(line_num, "unexpected seperator");
        }
    }
}

function void
init_types_array(Assets *assets)
{
    u32 running_total_of_assets = 0;
    for (u32 i = 0; i < ASSET_TYPE_AMOUNT; i++) 
    {
        assets->types[i].data = assets->data + (running_total_of_assets);
        running_total_of_assets += assets->types[i].num_of_assets;
    }
}

function u32
load_assets(Assets *assets, const char *filename) // returns 0 on success
{
    File file = read_file(filename);
    if (file.size == 0) { error("load_assets(): could not open file %s", filename); return 1; }
    parse_asset_file(assets, &file, count_asset);
    assets->info = ARRAY_MALLOC(Asset_Load_Info, assets->num_of_assets);
    reset_get_char(&file);
    parse_asset_file(assets, &file, add_asset);
    free_file(&file);
    
    //for (u32 i = 0; i < ASSET_TYPE_AMOUNT; i++) assets->num_of_assets += assets->types[i].num_of_assets;
    assets->data = ARRAY_MALLOC(Asset, assets->num_of_assets);
    init_types_array(assets);
    
    for (u32 i = 0; i < assets->num_of_assets; i++)
    {
        Asset_Load_Info *info = &assets->info[i];
        //printf("asset: %d, %s, %s\n", info->type, info->tag, info->filename);
        
        Asset asset = {};
        asset.type = info->type;
        asset.tag = info->tag;
        asset.tag_length = get_length(asset.tag);
        
        // how to load the various assets
        switch(asset.type)
        {
            case ASSET_TYPE_FONT: 
            {
                asset.font = load_font(info->filename); 
                assets->types[ASSET_TYPE_FONT].data[info->index] = asset;
            } break;
            
            case ASSET_TYPE_BITMAP: 
            {
                asset.bitmap = load_and_init_bitmap(info->filename); 
                assets->types[ASSET_TYPE_BITMAP].data[info->index] = asset;
            } break;
            
            case ASSET_TYPE_SHADER:
            {
                asset.shader.vs_filename  = info->filename;
                asset.shader.tcs_filename = info->tcs_filename;
                asset.shader.tes_filename = info->tes_filename;
                asset.shader.gs_filename  = info->gs_filename;
                asset.shader.fs_filename  = info->fs_filename;
                load_shader(&asset.shader);
                compile_shader(&asset.shader);
                assets->types[ASSET_TYPE_SHADER].data[info->index] = asset;
            } break;
            
            case ASSET_TYPE_AUDIO:
            {
                asset.audio = load_audio(info->filename);
                assets->types[ASSET_TYPE_AUDIO].data[info->index] = asset;
            } break;
        }
    }
    
    return 0;
}

function void
save_assets(Assets *assets, const char *filename)
{
    u32 i = 0;
    FILE *file = fopen(filename, "wb");
    fwrite(assets, sizeof(Assets), 1, file);
    fwrite(assets->data, sizeof(Asset), assets->num_of_assets, file);
    for (i = 0; i < assets->num_of_assets; i++) fwrite(assets->data[i].tag, assets->data[i].tag_length + 1, 1, file);
    
    for (i = 0; i < assets->num_of_assets; i++)
    {
        Asset *asset = &assets->data[i];
        switch(asset->type)
        {
            case ASSET_TYPE_FONT: 
            {
                fwrite(asset->font.file.memory, asset->font.file.size, 1, file);
            } break;
            
            case ASSET_TYPE_BITMAP: 
            {
                fwrite(asset->bitmap.memory, asset->bitmap.dim.x * asset->bitmap.dim.y * asset->bitmap.channels, 1, file);
            } break;
            
            case ASSET_TYPE_SHADER: 
            {
                fwrite(asset->shader.vs_file,  asset->shader.file_sizes[0], 1, file);
                fwrite(asset->shader.tcs_file, asset->shader.file_sizes[1], 1, file);
                fwrite(asset->shader.tes_file, asset->shader.file_sizes[2], 1, file);
                fwrite(asset->shader.gs_file,  asset->shader.file_sizes[3], 1, file);
                fwrite(asset->shader.fs_file,  asset->shader.file_sizes[4], 1, file);
            } break;
            
            case ASSET_TYPE_AUDIO:
            {
                fwrite(asset->audio.buffer, asset->audio.length, 1, file);
            } break;
        }
    }
    
    fclose(file);
}

function u32
load_saved_assets(Assets *assets, const char *filename) // returns 0 on success
{
    FILE *file = fopen(filename, "rb");
    if (file == 0) { error("load_saved_assets(): could not open file %s", filename); return 1; }
    fread(assets, sizeof(Assets), 1, file);
    assets->data = ARRAY_MALLOC(Asset, assets->num_of_assets);
    fread(assets->data, sizeof(Asset), assets->num_of_assets, file);
    for (u32 i = 0; i < assets->num_of_assets; i++)
    {
        assets->data[i].tag = (const char*)SDL_malloc(assets->data[i].tag_length + 1);
        fread((void*)assets->data[i].tag, assets->data[i].tag_length + 1, 1, file);
    }
    
    init_types_array(assets);
    
    u32 fonts_index = 0;
    u32 bitmaps_index = 0;
    u32 shaders_index = 0;
    u32 audios_index = 0;
    
    for (u32 i = 0; i < assets->num_of_assets; i++)
    {
        Asset *all_asset = &assets->data[i];
        Asset asset = {};
        asset.type = all_asset->type;
        asset.tag = all_asset->tag;
        
        switch(asset.type)
        {
            case ASSET_TYPE_FONT: 
            {
                asset.font = all_asset->font;
                asset.font.file.memory = SDL_malloc(asset.font.file.size);
                fread(asset.font.file.memory, asset.font.file.size, 1, file);

                asset.font.info = SDL_malloc(sizeof(stbtt_fontinfo));
                stbtt_fontinfo *info = (stbtt_fontinfo*)&asset.font.info;
                *info = {};

                stbtt_InitFont(info, (u8*)asset.font.file.memory, stbtt_GetFontOffsetForIndex((u8*)asset.font.file.memory, 0));
                assets->types[ASSET_TYPE_FONT].data[fonts_index++] = asset;
            } break;
            
            case ASSET_TYPE_BITMAP: 
            {
                asset.bitmap = all_asset->bitmap;
                u32 size = asset.bitmap.dim.x * asset.bitmap.dim.y * asset.bitmap.channels;
                asset.bitmap.memory = (u8*)SDL_malloc(size);
                fread(asset.bitmap.memory, size, 1, file);
                init_bitmap_handle(&asset.bitmap);
                assets->types[ASSET_TYPE_BITMAP].data[bitmaps_index++] = asset;
            } break;
            
            case ASSET_TYPE_SHADER: 
            {
                asset.shader = all_asset->shader;
                if (asset.shader.file_sizes[0]) asset.shader.vs_file  = (const char*)SDL_malloc(asset.shader.file_sizes[0]);
                if (asset.shader.file_sizes[1]) asset.shader.tcs_file = (const char*)SDL_malloc(asset.shader.file_sizes[1]);
                if (asset.shader.file_sizes[2]) asset.shader.tes_file = (const char*)SDL_malloc(asset.shader.file_sizes[2]);
                if (asset.shader.file_sizes[3]) asset.shader.gs_file  = (const char*)SDL_malloc(asset.shader.file_sizes[3]);
                if (asset.shader.file_sizes[4]) asset.shader.fs_file  = (const char*)SDL_malloc(asset.shader.file_sizes[4]);
                fread((void*)asset.shader.vs_file,  asset.shader.file_sizes[0], 1, file);
                fread((void*)asset.shader.tcs_file, asset.shader.file_sizes[1], 1, file);
                fread((void*)asset.shader.tes_file, asset.shader.file_sizes[2], 1, file);
                fread((void*)asset.shader.gs_file,  asset.shader.file_sizes[3], 1, file);
                fread((void*)asset.shader.fs_file,  asset.shader.file_sizes[4], 1, file);
                asset.shader.compiled = false;
                compile_shader(&asset.shader);
                assets->types[ASSET_TYPE_SHADER].data[shaders_index++] = asset;
            } break;
            
            case ASSET_TYPE_AUDIO:
            {
                asset.audio = all_asset->audio;
                asset.audio.buffer = (u8*)SDL_malloc(asset.audio.length);
                fread(asset.audio.buffer, asset.audio.length, 1, file);
                assets->types[ASSET_TYPE_AUDIO].data[audios_index++] = asset;
            } break;
        }
    }
    
    fclose(file);
    
    return 0;
}