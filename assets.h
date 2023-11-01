#ifndef ASSETS_H
#define ASSETS_H

struct File
{
    u32 size;
    void *memory;
    
    const char *ch; // for functions like get_char(File *file);
};

struct Bitmap
{
    u8 *memory;
    v2s dim;
    s32 pitch;
    s32 channels;
    
    u32 handle; // opengl handle
};

struct Cubemap
{
    const char *filenames[6] = { 
        "../assets/skybox/right.jpg", 
        "../assets/skybox/left.jpg", 
        "../assets/skybox/top.jpg",
        "../assets/skybox/bottom.jpg",
        "../assets/skybox/front.jpg",
        "../assets/skybox/back.jpg" 
    };
    Bitmap bitmaps[6];

    u32 handle; // opengl handle
};

Cubemap load_cubemap();

struct Shader
{
    const char *vs_filename;  //.vs vertex_shader
    const char *tcs_filename; //.tcs tessellation control shader
    const char *tes_filename; //.tes tessellation evaluation shader
    const char *gs_filename;  //.gs geometry shader
    const char *fs_filename;  //.fs fragment shader
    
    const char *vs_file;
    const char *tcs_file;
    const char *tes_file;
    const char *gs_file;
    const char *fs_file;
    
    u32 file_sizes[5]; // includes the file terminator. this is for saving it to a asset file
    
    b32 compiled;
    b32 uniform_buffer_objects_generated;
    u32 handle;
};
global const char *basic_vs = "#version 330 core\n layout (location = 0) in vec3 position; layout (location = 1) in vec3 normal; layout (location = 2) in vec2 texture_coords; uniform mat4 model; uniform mat4 projection; uniform mat4 view; out vec2 uv; void main(void) { gl_Position = projection * view * model * vec4(position, 1.0f); uv = texture_coords;}";
global const char *color_fs = "#version 330 core\n in vec2 uv; uniform vec4 user_color; out vec4 FragColor; void main() { FragColor  = vec4(user_color.x/255, user_color.y/255, user_color.z/255, user_color.w);}";
global const char *tex_fs = "#version 330 core\n uniform sampler2D tex0; in vec2 uv; out vec4 FragColor; void main() { vec4 tex = texture(tex0, uv); FragColor = tex;}";

u32 use_shader(Shader *shader); // returns the handle of a shader
void load_shader(Shader *shader);
void compile_shader(Shader *shader);

struct Vertex
{
    v3 position;
    v3 normal;
    v2 texture_coordinate;
};

struct Material
{
    v3 ambient;            // Ka
    v3 diffuse;            // Kd
    v3 specular;           // Ks
    f32 specular_exponent; // Ns
    
    Bitmap ambient_map;
    Bitmap diffuse_map;    // map_Kd
    
    const char *id; // id from mtl file
};

struct Mesh
{
    Vertex *vertices;
    u32 vertices_count;
    
    u32 *indices;
    u32 indices_count;
    
    u32 vao;
    u32 vbo;
    u32 ebo;
    
    Material material;
};

void init_mesh(Mesh *mesh);
void draw_mesh(Mesh *mesh);
void draw_mesh_patches(Mesh *mesh);

struct Light_Source
{
    v3 position;
    v4 color;
};

struct Camera
{
    v3 position;
    v3 target;
    v3 up;
    r32 fov;
    r32 yaw;
    r32 pitch;
};
inline m4x4 get_view(Camera camera) 
{ 
    return look_at(camera.position, camera.position + camera.target, camera.up); 
}

struct Model
{
    Mesh *meshes;
    u32 meshes_count;
};

Model load_obj(const char *path, const char *filename);
void draw_model(Shader *shader, Shader *tex_shader, Model *model, Light_Source light, Camera camera, v3 position, quat rotation);

struct Font_Scale
{
    f32 pixel_height;
    
    f32 scale;
    s32 ascent;
    s32 descent;
    s32 line_gap;
    f32 scaled_ascent;
    f32 scaled_descent;
};

struct Font_Char
{
    u32 codepoint;
    f32 scale;
    v4 color;
    
    s32 ax;
    s32 lsb;
    s32 c_x1;
    s32 c_y1;
    s32 c_x2;
    s32 c_y2;
    
    Bitmap bitmap;
};

struct Font_String
{
    char *memory;
    v2 dim;
    f32 pixel_height;
    v4 color;
};

struct Font
{
    File file;
    void *info; // stbtt_fontinfo
    
    s32 font_scales_cached;
    s32 font_chars_cached;
    s32 strings_cached;
    Font_Scale font_scales[10];
    Font_Char font_chars[300];
    Font_String font_strings[10];
};

v2 get_string_dim(Font *font, const char *string, f32 pixel_height, v4 color);

struct Audio
{
    //SDL_AudioSpec spec;
    u8 *buffer;
    u32 length;
};

// different volumes for different kinds of audio
enum
{
    AUDIO_SOUND,
    AUDIO_MUSIC,
};

struct Playing_Audio
{
    u8 *position;
    u32 length_remaining;
    u32 type; // type of sound to control the volume by type
};

struct Audio_Player
{
    b32 playing;
    Playing_Audio audios[10]; // let index 0 be for music
    u32 audios_count;
    
    u8 *buffer; // points to one byte
    u32 length; // in bytes largest amount copied
    u32 max_length; // in bytes amount available
    
    r32 music_volume;
    r32 sound_volume;
};

//
// Storing Assets
//

enum shader_types
{
    VERTEX_SHADER,
    TESSELLATION_CONTROL_SHADER,
    TESSELLATION_EVALUATION_SHADER,
    GEOMETRY_SHADER,
    FRAGMENT_SHADER,
};

enum asset_types
{
    ASSET_TYPE_BITMAP,
    ASSET_TYPE_FONT,
    ASSET_TYPE_SHADER,
    ASSET_TYPE_AUDIO,
    
    ASSET_TYPE_AMOUNT
};

struct Asset
{
    u32 type;
    const char *tag;
    u32 tag_length;
    union
    {
        Font font;
        Bitmap bitmap;
        Shader shader;
        Audio audio;
    };
};

struct Asset_Load_Info
{
    int type;
    int index;
    const char *tag;
    const char *filename;     //.vs vertex_shader
    
    const char *tcs_filename; //.tcs tessellation control shader
    const char *tes_filename; //.tes tessellation evaluation shader
    const char *gs_filename;  //.gs geometry shader
    const char *fs_filename;  //.fs fragment shader
};

struct Asset_Array
{
    Asset *data;
    u32 num_of_assets;
};

struct Assets
{
    Asset *data; // array of assets
    u32 num_of_assets;
    
    Asset_Load_Info *info;
    u32 num_of_info_loaded;
    
    Asset_Array types[4]; // 0 = bitmap, 1 = font ...
};

function Bitmap*
find_bitmap(Assets *assets, const char *tag)
{
    for (u32 i = 0; i < assets->types[ASSET_TYPE_BITMAP].num_of_assets; i++)
    {
        if (equal(tag, assets->types[ASSET_TYPE_BITMAP].data[i].tag)) 
            return &assets->types[ASSET_TYPE_BITMAP].data[i].bitmap;
    }
    warning(0, "Could not find bitmap with tag: %s", tag);
    return 0;
}

function Font*
find_font(Assets *assets, const char *tag)
{
    for (u32 i = 0; i < assets->types[ASSET_TYPE_FONT].num_of_assets; i++)
    {
        if (equal(tag, assets->types[ASSET_TYPE_FONT].data[i].tag)) 
            return &assets->types[ASSET_TYPE_FONT].data[i].font;
    }
    warning(0, "Could not find font with tag: %s", tag);
    return 0;
}

function Shader*
find_shader(Assets *assets, const char *tag)
{
    for (u32 i = 0; i < assets->types[ASSET_TYPE_SHADER].num_of_assets; i++)
    {
        if (equal(tag, assets->types[ASSET_TYPE_SHADER].data[i].tag)) 
            return &assets->types[ASSET_TYPE_SHADER].data[i].shader;
    }
    warning(0, "Could not find shader with tag: %s", tag);
    return 0;
}


//
// Parsing Asset File
//

function void
add_asset(void *data, void *args)
{
    Assets *assets = (Assets*)data;
    Asset_Load_Info *info = (Asset_Load_Info*)args;
    
    info->index = assets->types[info->type].num_of_assets;
    assets->types[info->type].num_of_assets++;
    
    assets->info[assets->num_of_info_loaded++] = *info;
}

function void
count_asset(void *data, void *args)
{
    Assets *assets = (Assets*)data;
    assets->num_of_assets++;
}

function bool
is_ascii_digit(int ch)
{
    if (isdigit(ch)) return true;
    return false;
}

function bool
is_ascii_letter(int ch)
{
    if (ch >= 'A' && ch <= 'Z') return true; // uppercase
    else if (ch >= 'a' && ch <= 'z') return true; // lowercase
    else return false;
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

global const char *asset_keywords[4] = { "FONTS", "BITMAPS", "SHADERS", "AUDIOS" };

function b32
is_asset_keyword(const char *word)
{
    for (u32 i = 0; i < ARRAY_COUNT(asset_keywords); i++) 
        if (equal(word, asset_keywords[i])) return true;
    return false;
}

#endif //ASSETS_H
