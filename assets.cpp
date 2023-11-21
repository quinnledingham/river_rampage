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

// returns the file with a 0 at the end of the memory.
// useful if you want to read the file like a string immediately.
function File
read_file_terminated(const char *filename) {
    File result = {};
    File file = read_file(filename);

    //result = file;
    result.size = file.size + 1;
    result.path = filename;
    result.memory = platform_malloc(result.size);
    SDL_memcpy(result.memory, file.memory, file.size);

    char *r = (char*)result.memory;
    r[file.size] = 0; // last byte in result.memory
    
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
    SDL_memset(string, 0, length + 1);
    
    const char *ptr = file->ch - length;
    for (u32 i = 0; i < length; i++)
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
    if (file->memory != 0) SDL_free(file->memory);
    *file = {}; // sets file-memory to 0
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

enum Texture_Parameters
{
    TEXTURE_PARAMETERS_DEFAULT,
    TEXTURE_PARAMETERS_CHAR,
};

function void
init_bitmap_handle(Bitmap *bitmap, u32 texture_parameters)
{
    GLenum target = GL_TEXTURE_2D;
    
    glGenTextures(1, &bitmap->handle);
    glBindTexture(target, bitmap->handle);
    
    GLint internal_format = 0;
    GLenum data_format = 0;
    GLint pixel_unpack_alignment = 0;
    
    switch(bitmap->channels)
    {
        case 1: {
            internal_format = GL_RED,
            data_format = GL_RED,
            pixel_unpack_alignment = 1; 
        } break;

        case 3: {
            internal_format = GL_RGB;
            data_format = GL_RGB;
            pixel_unpack_alignment = 1; // because RGB is weird case unpack alignment can't be 3
        } break;
        
        case 4: {
            internal_format = GL_RGBA;
            data_format = GL_RGBA;
            pixel_unpack_alignment = 4;
        } break;
    }
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, pixel_unpack_alignment);
    glTexImage2D(target, 0, internal_format, bitmap->dim.width, bitmap->dim.height, 0, data_format, GL_UNSIGNED_BYTE, bitmap->memory);
    glGenerateMipmap(target);
    
    switch(texture_parameters)
    {
        case TEXTURE_PARAMETERS_DEFAULT:
        glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        break;

        case TEXTURE_PARAMETERS_CHAR:
        glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        break;
    }
    
    glBindTexture(target, 0);
}

function void init_bitmap_handle(Bitmap *bitmap) { init_bitmap_handle(bitmap, TEXTURE_PARAMETERS_DEFAULT); }

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

// loads the files
void load_shader(Shader *shader)
{
    if (shader->files[0].path == 0) {
        error("load_opengl_shader() must have a vertex shader");
        return;
    }

    printf("loaded shader: ");
    for (u32 i = 0; i < SHADER_TYPE_AMOUNT; i++) {
        if (shader->files[i].memory != 0) platform_free(&shader->files[i].memory);
        shader->files[i].memory = 0;

        if (shader->files[i].path != 0) {
            shader->files[i] = read_file_terminated(shader->files[i].path);
            printf("%s ", shader->files[i].path);
        }
    }
    printf("\n");
}

bool compile_shader(u32 handle, const char *file, int type)
{
    u32 shader =  glCreateShader((GLenum)type);
    glShaderSource(shader, 1, &file, NULL);
    glCompileShader(shader);
    
    GLint compiled_shader = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled_shader);  
    if (!compiled_shader) {
        opengl_debug(GL_SHADER, shader);
    } else {
        glAttachShader(handle, shader);
    }
    
    glDeleteShader(shader);
    
    return compiled_shader;
}

// lines up with enum shader_types
const u32 file_types[5] = { 
    GL_VERTEX_SHADER,
    GL_TESS_CONTROL_SHADER,
    GL_TESS_EVALUATION_SHADER,
    GL_GEOMETRY_SHADER,
    GL_FRAGMENT_SHADER,
};

// compiles the files
void compile_shader(Shader *shader)
{
    shader->uniform_buffer_objects_generated = false;
    shader->compiled = false;
    if (shader->handle != 0) glDeleteProgram(shader->handle);
    shader->handle = glCreateProgram();
    
    if (shader->files[0].memory == 0) {
        error("vertex shader required");
        return;
    }

    for (u32 i = 0; i < SHADER_TYPE_AMOUNT; i++) {
        if (shader->files[i].memory == 0) continue; // file was not loaded

        if (!compile_shader(shader->handle, (char*)shader->files[i].memory, file_types[i])) {
            error("compile_shader() could not compile %s", shader->files[i].path); 
            return;
        }
    }

    // Link
    glLinkProgram(shader->handle);

    GLint linked_program = 0;
    glGetProgramiv(shader->handle, GL_LINK_STATUS, &linked_program);
    if (!linked_program) {
        opengl_debug(GL_PROGRAM, shader->handle);
        error("compile_shader() link failed");
        return;
    }

    shader->compiled = true;
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
    SDL_memset(font.font_chars, 0, sizeof(Font_Char)        * ARRAY_COUNT(font.font_chars));
    SDL_memset(font.bitmaps,    0, sizeof(Font_Char_Bitmap) * ARRAY_COUNT(font.bitmaps));
    font.file = read_file(filename);
    
    return font;
}

function void
init_font(Font *font)
{
    font->info = SDL_malloc(sizeof(stbtt_fontinfo));
    stbtt_fontinfo *info = (stbtt_fontinfo*)font->info;
    *info = {};
    
    stbtt_InitFont(info, (u8*)font->file.memory, stbtt_GetFontOffsetForIndex((u8*)font->file.memory, 0));
    stbtt_GetFontBoundingBox(info, &font->bb_0.x, &font->bb_0.y, &font->bb_1.x, &font->bb_1.y);
}

function Font_Char*
load_font_char(Font *font, u32 codepoint)
{
    stbtt_fontinfo *info = (stbtt_fontinfo*)font->info;

    // search cache for font char
    for (s32 i = 0; i < font->font_chars_cached; i++)
    {
        Font_Char *font_char = &font->font_chars[i];
        if (font_char->codepoint == codepoint)
            return font_char;
    }
    
    // where to cache new font char
    Font_Char *font_char = &font->font_chars[font->font_chars_cached++];
    if (font->font_chars_cached >= ARRAY_COUNT(font->font_chars)) 
        font->font_chars_cached = 0;

    memset(font_char, 0, sizeof(Font_Char));
    font_char->codepoint = codepoint;
    font_char->glyph_index = stbtt_FindGlyphIndex(info, font_char->codepoint);
    
    // how wide is this character
    stbtt_GetGlyphHMetrics(info, font_char->glyph_index, &font_char->ax, &font_char->lsb);
    
    stbtt_GetGlyphBox(info, font_char->glyph_index, &font_char->bb_0.x, &font_char->bb_0.y, &font_char->bb_1.x, &font_char->bb_1.y);

    return font_char;
}

Font_Char_Bitmap*
load_font_char_bitmap(Font *font, u32 codepoint, f32 scale)
{
    if (scale == 0.0f) {
        error("load_font_char_bitmap(): scale can not be zero"); // scale used below
        return 0;
    }

    stbtt_fontinfo *info = (stbtt_fontinfo*)font->info;

    // search cache for font char
    for (s32 i = 0; i < font->bitmaps_cached; i++)
    {
        Font_Char_Bitmap *bitmap = &font->bitmaps[i];
        if (bitmap->font_char->codepoint == codepoint && bitmap->scale == scale)
            return bitmap;
    }

    // where to cache new font char
    Font_Char_Bitmap *bitmap = &font->bitmaps[font->bitmaps_cached++];
    if (font->bitmaps_cached >= ARRAY_COUNT(font->bitmaps)) 
        font->bitmaps_cached = 0;

    // free bitmap if one is being overwritten
    if (bitmap->scale != 0) { 
        stbtt_FreeBitmap(bitmap->bitmap.memory, info->userdata);
        glDeleteTextures(1, &bitmap->bitmap.handle);
        bitmap->bitmap.memory = 0;
    }

    memset(bitmap, 0, sizeof(Font_Char_Bitmap));
    bitmap->font_char = load_font_char(font, codepoint);
    bitmap->scale = scale;

    bitmap->bitmap.memory = stbtt_GetGlyphBitmap(info, 0, bitmap->scale, bitmap->font_char->glyph_index, &bitmap->bitmap.dim.width, &bitmap->bitmap.dim.height, 0, 0);
    bitmap->bitmap.channels = 1;

    init_bitmap_handle(&bitmap->bitmap, TEXTURE_PARAMETERS_CHAR);

    return bitmap;
}

v2 get_font_loaded_dim(Font *font, f32 pixel_height)
{
    f32 scale = stbtt_ScaleForPixelHeight((stbtt_fontinfo*)font->info, pixel_height);
    v2 dim = {};

    for (s32 i = 0; i < font->font_chars_cached; i++)
    {
        Font_Char *font_char = &font->font_chars[i];
        if (dim.y < (f32)font_char->bb_1.y) dim.y = (f32)font_char->bb_1.y;
        if (dim.x < (f32)font_char->bb_1.x) dim.x = (f32)font_char->bb_1.x;
    }

    return dim * scale;
}

v2 get_font_dim(Font *font, f32 pixel_height)
{
    f32 scale = stbtt_ScaleForPixelHeight((stbtt_fontinfo*)font->info, pixel_height);
    v2 dim = cv2(font->bb_1 - font->bb_0);
    return dim * scale;
}

v2 get_string_dim(Font *font, const char *string, s32 length, f32 pixel_height, v4 color)
{
    if (string == 0) return { 0, 0 };
    if (font == 0) {
        error("get_string_dim(): no font");
        return { 0, 0 };
    }

    stbtt_fontinfo *info = (stbtt_fontinfo*)font->info;
    f32 scale = stbtt_ScaleForPixelHeight(info, pixel_height);
    v2 dim = {};
    u32 index = 0;

    while (string[index] != 0)
    {
        if (length != -1)
        {
            // if a length is set then only include in the dim the chars up to that point
            if (index == length) break;
        }

        Font_Char *font_char = load_font_char(font, string[index]);
        
        if (dim.y < (r32)font_char->bb_1.y) 
            dim.y = (r32)font_char->bb_1.y;
        
        int kern = stbtt_GetCodepointKernAdvance(info, string[index], string[index + 1]);
        dim.x += (kern + font_char->ax);
        
        index++;
    }

    dim *= scale;
    
    return dim;
}

v2 get_string_dim(Font *font, const char *string, f32 pixel_height, v4 color)
{
    return get_string_dim(font, string, -1, pixel_height, color);
}

f32 get_scale_for_pixel_height(void *info, f32 pixel_height) {
    return stbtt_ScaleForPixelHeight((stbtt_fontinfo*)info, pixel_height);
}

s32 get_codepoint_kern_advance(void *info, s32 ch1, s32 ch2) {
    return stbtt_GetCodepointKernAdvance((stbtt_fontinfo*)info, ch1, ch2);
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

void draw_model(Model *model, Camera camera, v3 position, quat rotation)
{
    u32 shader_enabled = 0; // 0 no shader, 1 shader, 2 tex_shader
    u32 handle = 0;

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, 0);

    for (u32 i = 0; i < model->meshes_count; i++) {
        if (shader_enabled != 1 && model->meshes[i].material.diffuse_map.memory == 0) {
            shader_enabled = 1;
            handle = use_shader(model->color_shader);
        } else if (shader_enabled != 2 && model->meshes[i].material.diffuse_map.memory != 0) {
            shader_enabled = 2;
            handle = use_shader(model->texture_shader);
        }

        m4x4 model_matrix = create_transform_m4x4(position, rotation, {1, 1, 1});
        glUniformMatrix4fv(glGetUniformLocation(handle, "model"), (GLsizei)1, false, (float*)&model_matrix);
        glUniform3fv(glGetUniformLocation(handle, "viewPos"), (GLsizei)1, (float*)&camera.position);   

        glUniform3fv(glGetUniformLocation(handle, "material.ambient"),  (GLsizei)1, (float*)&model->meshes[i].material.ambient);
        glUniform3fv(glGetUniformLocation(handle, "material.diffuse"),  (GLsizei)1, (float*)&model->meshes[i].material.diffuse);
        glUniform3fv(glGetUniformLocation(handle, "material.specular"), (GLsizei)1, (float*)&model->meshes[i].material.specular);
        glUniform1f (glGetUniformLocation(handle, "material.shininess"), model->meshes[i].material.specular_exponent);

        if (model->meshes[i].material.diffuse_map.memory != 0)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, model->meshes[i].material.diffuse_map.handle);
        }
        
        draw_mesh(&model->meshes[i]);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

internal void
init_model(Model *model) {
    for (u32 mesh_index = 0; mesh_index < model->meshes_count; mesh_index++) {
        Mesh *mesh = &model->meshes[mesh_index];
        init_mesh(mesh);
        init_bitmap_handle(&mesh->material.diffuse_map);
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

// returned from the obj scanner
struct OBJ_Token
{
    s32 type;
    const char *lexeme;
    s32 ch; // char when the token is created. Used for debugging
};

union Face_Vertex {
    struct {
        u32 position_index;
        u32 uv_index;
        u32 normal_index;
    };
    u32 E[3];
};

// stores information taken from a .obj file
struct OBJ {
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
    for (u32 i = 0; i < 3; i++) {
        file->ch = (char*)skip_whitespace(file->ch);
        file->ch = (char*)char_array_to_f32(file->ch, &v->E[i]);
    }
    file->ch = (char*)skip_whitespace(file->ch);
}

internal void
parse_face_vertex(File *file, s32 *line_num, Face_Vertex *f)
{
    for (u32 i = 0; i < 3; i++) {
        file->ch = (char*)skip_whitespace(file->ch);
        file->ch = (char*)char_array_to_u32(file->ch, &f->E[i]);
    }
}

internal void
obj_count(OBJ *obj, File file) {
    char buffer[40];
    OBJ_Token token = {};
    s32 line_num = 1;
    do
    {
        token = scan_obj(&file, &line_num, false, buffer);
        
        switch(token.type)
        {
            case OBJ_TOKEN_VERTEX:        obj->vertices_count++; break;
            case OBJ_TOKEN_NORMAL:        obj->normals_count++;  break;
            case OBJ_TOKEN_TEXTURE_COORD: obj->uvs_count++;      break;
            case OBJ_TOKEN_FACE:          obj->faces_count++;    break;
            case OBJ_TOKEN_LINE:          obj->lines_count++;    break;
            case OBJ_TOKEN_USEMTL:        obj->meshes_count++;   break;
        }
        
    } while(token.type != OBJ_TOKEN_EOF);
}

internal void
obj_fill_arrays(OBJ *obj, File file, Model *model) {
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
                parse_v3_obj(&file, &line_num, &obj->vertices[vertices_index++]);
            }
            else if (*file.ch == 'n') 
            {
                file.ch++;
                parse_v3_obj(&file, &line_num, &obj->normals[normals_index++]);
            }
            else if (*file.ch == 't')
            {
                file.ch++;
                file.ch = (char*)parse_float(file.ch, &obj->uvs[uvs_index].x);
                file.ch = (char*)parse_float(file.ch, &obj->uvs[uvs_index].y);
                uvs_index++;
            }
        }
        else if (*file.ch == 'f')
        {
            file.ch++;
            if (*file.ch == ' ' || *file.ch == '\t')
            {
                file.ch++;
                parse_face_vertex(&file, &line_num, &obj->face_vertices[face_vertices_index++]);
                parse_face_vertex(&file, &line_num, &obj->face_vertices[face_vertices_index++]);
                parse_face_vertex(&file, &line_num, &obj->face_vertices[face_vertices_index++]);
                obj->meshes_face_count[meshes_index]++;
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
                obj->material_filename = string_malloc(token.lexeme);
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
        
        if      (token.type == OBJ_TOKEN_VERTEX) parse_v3_obj(&file, &line_num, &obj->vertices[vertices_index++]);
        else if (token.type == OBJ_TOKEN_NORMAL) parse_v3_obj(&file, &line_num, &obj->normals[normals_index++]);
        else if (token.type == OBJ_TOKEN_TEXTURE_COORD)
        {
            file.ch = (char*)skip_whitespace(file.ch);
            file.ch = (char*)char_array_to_f32(file.ch, &obj->uvs[uvs_index].x);
            file.ch = (char*)skip_whitespace(file.ch);
            file.ch = (char*)char_array_to_f32(file.ch, &obj->uvs[uvs_index].y);
            file.ch = (char*)skip_whitespace(file.ch);
            uvs_index++;
        }
        else if (token.type == OBJ_TOKEN_FACE)
        {
            parse_face_vertex(&file, &line_num, &obj->face_vertices[face_vertices_index++]);
            parse_face_vertex(&file, &line_num, &obj->face_vertices[face_vertices_index++]);
            parse_face_vertex(&file, &line_num, &obj->face_vertices[face_vertices_index++]);
            
            obj->meshes_face_count[meshes_index]++;
        }
        else if (token.type == OBJ_TOKEN_LINE)
        {
            token = scan_obj(&file, &line_num, true, buffer);
            token = scan_obj(&file, &line_num, true, buffer);
        }
        else if (token.type == OBJ_TOKEN_MTLLIB) // get the material file to load
        {
            token = scan_obj(&file, &line_num, true, buffer);
            obj->material_filename = string_malloc(token.lexeme);
        }
        else if (token.type == OBJ_TOKEN_USEMTL) // different mesh every time the material is changed
        {
            meshes_index++;
            token = scan_obj(&file, &line_num, true, buffer);
            model->meshes[meshes_index].material.id = string_malloc(token.lexeme);
        }
    } while(token.type != OBJ_TOKEN_EOF);
}

internal void
model_create_meshes(Model *model, OBJ obj, MTL mtl) {
    s32 lower_range = 0;
    for (u32 mesh_index = 0; mesh_index < model->meshes_count; mesh_index++)
    {
        Mesh *mesh = &model->meshes[mesh_index];        
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
        
        // assign material aka load material
        for (u32 i = 0; i < mtl.materials_count; i++)
        {
            if (equal(mesh->material.id, mtl.materials[i].id)) 
                mesh->material = mtl.materials[i];
        }
                    
        lower_range += mesh_face_vertices_count;
    }
}

Model load_obj(const char *filename)
{
    Model model = {};
    OBJ obj = {};
    
    File file = read_file(filename);

    if (!file.size) { error("load_obj: could not read object file"); return model; }

    // count components
    obj_count(&obj, file);
    
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
    
    obj_fill_arrays(&obj, file, &model);
    
    free_file(&file);
    
    const char *filepath = get_path(filename);
    MTL mtl = load_mtl(filepath, obj.material_filename);
    platform_free((void*)filepath);
    
    // creating the model's meshes
    model_create_meshes(&model, obj, mtl);
    
    platform_free(mtl.materials);
    
    platform_free(obj.vertices);
    platform_free(obj.uvs);
    platform_free(obj.normals);
    platform_free(obj.face_vertices);
    platform_free(obj.meshes_face_count);
    
    return model;
}

//
// Asset File Reading
//

enum Asset_Token_Type
{
    ATT_KEYWORD,
    ATT_ID,
    ATT_SEPERATOR,
    ATT_ERROR,
    ATT_WARNING,
    ATT_END
};

struct Asset_Token
{
    s32 type;
    const char *lexeme;
};

const Pair shader_types[SHADER_TYPE_AMOUNT] = {
    { VERTEX_SHADER,                  "VERTEX"     },
    { TESSELLATION_CONTROL_SHADER,    "CONTROL"    },
    { TESSELLATION_EVALUATION_SHADER, "EVALUATION" },
    { GEOMETRY_SHADER,                "GEOMETRY"   },
    { FRAGMENT_SHADER,                "FRAGMENT"   },
};

const Pair asset_types[ASSET_TYPE_AMOUNT] = {
    { ASSET_TYPE_BITMAP, "BITMAPS" },
    { ASSET_TYPE_FONT,   "FONTS"   },
    { ASSET_TYPE_SHADER, "SHADERS" },
    { ASSET_TYPE_AUDIO,  "AUDIOS"  },
    { ASSET_TYPE_MODEL,  "MODELS"  },
};

function b32
is_asset_keyword(const char *word)
{
    for (u32 i = 0; i < ASSET_TYPE_AMOUNT; i++) {
        if (equal(word, pair_get_value(asset_types, ASSET_TYPE_AMOUNT, i))) 
            return true;
    }
    return false;
}

function b32
is_file_path_ch(s32 ch)
{
    if (ch == '.' || ch == '/' || ch == '-' || ch == '_') return true;
    else return false;
}

function b32
is_valid_body_ch(s32 ch)
{
    if (is_ascii_letter(ch) || is_ascii_digit(ch) || is_file_path_ch(ch)) return true;
    else return false;
}

function b32
is_valid_start_ch(s32 ch)
{
    if (is_ascii_letter(ch) || is_file_path_ch(ch)) return true;
    else return false;
}

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

struct Asset_Load_Info
{
    int type;             // type of asset - what type array to put it in  
    int indexes[ASSET_TYPE_AMOUNT]; // where in the type array the asset should be loaded

    const char *tag;      // name
    const char *filename; // file path or file in case of model
    const char *path;     // path to folder
    
    const char *file_paths[SHADER_TYPE_AMOUNT]; // shader
};

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
        
        if (tok.type == ATT_KEYWORD)
        {
            u32 type = pair_get_key(asset_types, ASSET_TYPE_AMOUNT, tok.lexeme);
            if (type == ASSET_TYPE_AMOUNT) error("parse_asset_file(): could not find asset type for lexeme");

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
                    info.file_paths[shader_type] = tok.lexeme;
                }
                else if (equal(last_token.lexeme, "|")) // shader part needs to be grabbed
                {
                    shader_type = pair_get_key(shader_types, SHADER_TYPE_AMOUNT, tok.lexeme);
                    
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
                        int indexes[ASSET_TYPE_AMOUNT];
                        for (u32 i = 0; i < ASSET_TYPE_AMOUNT; i++) indexes[i] = info.indexes[i];
                        info = {};
                        info.type = ASSET_TYPE_SHADER;
                        for (u32 i = 0; i < ASSET_TYPE_AMOUNT; i++) info.indexes[i] = indexes[i];
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
            else // Fonts, Bitmaps, Audios, Models
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

internal void
load_asset(Asset *asset, Asset_Load_Info *info)
{
    asset->type = info->type;
    asset->tag = info->tag;
    asset->tag_length = get_length(asset->tag);
    //log("loading: %s", asset->tag);
    // how to load the various assets
    switch(asset->type)
    {
        case ASSET_TYPE_FONT:   asset->font   = load_font(info->filename); break;
        case ASSET_TYPE_BITMAP: asset->bitmap = load_bitmap(info->filename); break;
        case ASSET_TYPE_SHADER:
        {
            for (u32 i = 0; i < SHADER_TYPE_AMOUNT; i++) {
                asset->shader.files[i].path = info->file_paths[i];
            }
            load_shader(&asset->shader);
        } break;         
        case ASSET_TYPE_AUDIO:  asset->audio = load_audio(info->filename); break;
        case ASSET_TYPE_MODEL:  asset->model = load_obj(info->filename); break;
    }
}

function void
add_asset(void *data, void *args)
{
    Assets *assets = (Assets*)data;
    Asset_Load_Info *info = (Asset_Load_Info*)args;

    u32 asset_array_index = info->indexes[info->type]++;
    Asset *asset = &assets->types[info->type].data[asset_array_index];
    load_asset(asset, info);
}

function void
count_asset(void *data, void *args)
{
    Assets *assets = (Assets*)data;
    Asset_Load_Info *info = (Asset_Load_Info*)args;

    assets->num_of_assets++;
    assets->types[info->type].num_of_assets++;
}

function void
init_types_array(Asset *data, Asset_Array *types)
{
    u32 running_total_of_assets = 0;
    for (u32 i = 0; i < ASSET_TYPE_AMOUNT; i++) {
        types[i].data = data + (running_total_of_assets);
        running_total_of_assets += types[i].num_of_assets;
    }
}

function u32
load_assets(Assets *assets, const char *filename) // returns 0 on success
{
    File file = read_file(filename);
    if (file.size == 0) { error("load_assets(): could not open file %s", filename); return 1; }
    
    parse_asset_file(assets, &file, count_asset);
    assets->data = ARRAY_MALLOC(Asset, assets->num_of_assets);
    init_types_array(assets->data, assets->types);

    reset_get_char(&file);
    parse_asset_file(assets, &file, add_asset);
    free_file(&file);

    return 0;
}

internal void
init_assets(Assets *assets)
{
    for (u32 i = 0; i < assets->num_of_assets; i++) {
        Asset *asset = &assets->data[i];
        switch(asset->type) {
            case ASSET_TYPE_FONT:   init_font(&asset->font);            break;
            case ASSET_TYPE_BITMAP: init_bitmap_handle(&asset->bitmap); break;
            case ASSET_TYPE_SHADER: compile_shader(&asset->shader);     break;
            case ASSET_TYPE_AUDIO:                                      break;
            case ASSET_TYPE_MODEL:  init_model(&asset->model);          break;
        }
    }
}

internal void
save_bitmap_memory(Bitmap bitmap, FILE *file)
{
    fwrite(bitmap.memory, bitmap.dim.x * bitmap.dim.y * bitmap.channels, 1, file);
}

internal void
load_bitmap_memory(Bitmap *bitmap, FILE *file)
{
    u32 size = bitmap->dim.x * bitmap->dim.y * bitmap->channels;
    bitmap->memory = (u8*)SDL_malloc(size);
    fread(bitmap->memory, size, 1, file);
}

internal void
save_mesh(Mesh mesh, FILE *file)
{
    fwrite((void*)&mesh, sizeof(Mesh), 1, file);
    fwrite(mesh.vertices, sizeof(Vertex), mesh.vertices_count, file);
    fwrite(mesh.indices, sizeof(u32), mesh.indices_count, file);

    if (mesh.material.diffuse_map.channels != 0) save_bitmap_memory(mesh.material.diffuse_map, file);
}

internal Mesh
load_mesh(FILE *file)
{
    Mesh mesh = {};
    fread((void*)&mesh, sizeof(Mesh), 1, file);
    mesh.vertices = ARRAY_MALLOC(Vertex, mesh.vertices_count);
    mesh.indices = ARRAY_MALLOC(u32, mesh.indices_count);
    fread(mesh.vertices, sizeof(Vertex), mesh.vertices_count, file);
    fread(mesh.indices, sizeof(u32), mesh.indices_count, file);

    if (mesh.material.diffuse_map.channels != 0) load_bitmap_memory(&mesh.material.diffuse_map, file);

    return mesh;
}

function void
save_assets(Assets *assets, const char *filename)
{
    u32 i = 0;
    FILE *file = fopen(filename, "wb");
    fwrite(assets, sizeof(Assets), 1, file);
    fwrite(assets->data, sizeof(Asset), assets->num_of_assets, file);
    
    for (i = 0; i < assets->num_of_assets; i++)
    {
        Asset *asset = &assets->data[i];

        fwrite(asset->tag, asset->tag_length + 1, 1, file);

        switch(asset->type)
        {
            case ASSET_TYPE_FONT: fwrite(asset->font.file.memory, asset->font.file.size, 1, file); break;
            case ASSET_TYPE_BITMAP: save_bitmap_memory(asset->bitmap, file); break;
            
            case ASSET_TYPE_SHADER: {
                for (u32 i = 0; i < SHADER_TYPE_AMOUNT; i++) {
                    fwrite(asset->shader.files[i].memory, asset->shader.files[i].size, 1, file);
                }
            } break;
            
            case ASSET_TYPE_AUDIO: fwrite(asset->audio.buffer, asset->audio.length, 1, file); break;

            case ASSET_TYPE_MODEL:{
                for (u32 i = 0; i < asset->model.meshes_count; i++) 
                    save_mesh(asset->model.meshes[i], file);
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

    init_types_array(assets->data, assets->types);
    
    for (u32 i = 0; i < assets->num_of_assets; i++)
    {
        Asset *asset = &assets->data[i];

        asset->tag = (const char*)SDL_malloc(asset->tag_length + 1);
        fread((void*)asset->tag, asset->tag_length + 1, 1, file);
        
        switch(asset->type) {
            case ASSET_TYPE_FONT: {
                asset->font.file.memory = SDL_malloc(asset->font.file.size);
                fread(asset->font.file.memory, asset->font.file.size, 1, file);
            } break;

            case ASSET_TYPE_BITMAP: load_bitmap_memory(&asset->bitmap, file); break;

            case ASSET_TYPE_SHADER: {
                for (u32 i = 0; i < SHADER_TYPE_AMOUNT; i++) {
                    if (asset->shader.files[i].size) {
                        asset->shader.files[i].memory = SDL_malloc(asset->shader.files[i].size);
                        fread((void*)asset->shader.files[i].memory, asset->shader.files[i].size, 1, file);
                    }
                    asset->shader.files[i].path = 0;
                }

                asset->shader.compiled = false;
            } break;
            
            case ASSET_TYPE_AUDIO: {
                asset->audio.buffer = (u8*)SDL_malloc(asset->audio.length);
                fread(asset->audio.buffer, asset->audio.length, 1, file);
            } break;

            case ASSET_TYPE_MODEL: {
                asset->model.meshes = (Mesh*)SDL_malloc(asset->model.meshes_count * sizeof(Mesh));

                for (u32 i = 0; i < asset->model.meshes_count; i++) { 
                    asset->model.meshes[i] = load_mesh(file);
                }
            } break;
        }
    }
    
    fclose(file);
    
    return 0;
}