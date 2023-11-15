#ifndef ASSETS_H
#define ASSETS_H

struct File
{
    u32 size;
    void *memory;
    const char *path;
    
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

enum shader_types
{
    VERTEX_SHADER,                  // 0
    TESSELLATION_CONTROL_SHADER,    // 1
    TESSELLATION_EVALUATION_SHADER, // 2
    GEOMETRY_SHADER,                // 3
    FRAGMENT_SHADER,                // 4

    SHADER_TYPE_AMOUNT              // 5
};

struct Shader
{
    File files[SHADER_TYPE_AMOUNT];

    b32 compiled;
    b32 uniform_buffer_objects_generated;
    u32 handle;
};

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

struct Light
{
    v3 position;
    v3 ambient;
    v3 diffuse;
    v3 specular;
    v4 color;
};

union Camera
{
    struct {
        v3 position;
        v3 target;
        v3 up;
        r32 fov;
        r32 yaw;
        r32 pitch;
    };
    r32 E[12];
};
inline m4x4 get_view(Camera camera) 
{ 
    return look_at(camera.position, camera.position + camera.target, camera.up); 
}

struct Model
{
    Mesh *meshes;
    u32 meshes_count;

    Shader *color_shader;
    Shader *texture_shader;
};

Model load_obj(const char *filename);
void draw_model(Model *model, Camera camera, v3 position, quat rotation);

struct Font_Char
{
    u32 codepoint; // ascii
    u32 glyph_index; // unicode
    
    s32 ax; // advance width
    s32 lsb; // left side bearing

    v2s bb_0; // bounding box coord 0
    v2s bb_1; // bounding box coord 1
};

struct Font_Char_Bitmap
{
    Font_Char *font_char;
    f32 scale;
    Bitmap bitmap;
};

struct Font_String
{
    Font_Char_Bitmap bitmaps[20]; // don't use font cache. just save them here always
    v2 dim;
    f32 pixel_height;
    v4 color;
};

struct Font
{
    File file;
    void *info; // stbtt_fontinfo
    
    v2s bb_0; // font bounding box coord 0
    v2s bb_1; // font bounding box coord 1

    s32 font_chars_cached;
    s32 bitmaps_cached;
    
    Font_Char font_chars[255];
    Font_Char_Bitmap bitmaps[300];
};

v2 get_font_loaded_dim(Font *font, f32 pixel_height);
v2 get_font_dim(Font *font, f32 pixel_height);
v2 get_string_dim(Font *font, const char *string, f32 pixel_height, v4 color);
v2 get_string_dim(Font *font, const char *string, s32 length, f32 pixel_height, v4 color);

Font_Char_Bitmap* load_font_char_bitmap(Font *font, u32 codepoint, f32 scale);
f32 get_scale_for_pixel_height(void *info, f32 pixel_height);
s32 get_codepoint_kern_advance(void *info, s32 ch1, s32 ch2);

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

enum asset_types
{
    ASSET_TYPE_BITMAP,
    ASSET_TYPE_FONT,
    ASSET_TYPE_SHADER,
    ASSET_TYPE_AUDIO,
    ASSET_TYPE_MODEL,
    
    ASSET_TYPE_AMOUNT
};

struct Asset
{
    u32 type;
    const char *tag;
    u32 tag_length;
    union
    {
        Font   font;
        Bitmap bitmap;
        Shader shader;
        Audio  audio;
        Model  model;
        void  *memory;
    };
};

struct Asset_Array
{
    Asset *data;       // points to Assets data array
    u32 num_of_assets;
};

struct Assets
{
    Asset *data; // array of assets
    u32 num_of_assets;
    
    Asset_Array types[ASSET_TYPE_AMOUNT]; // points to same memory as *data
};

internal void*
find_asset(Assets *assets, u32 type, const char *tag)
{
    for (u32 i = 0; i < assets->types[type].num_of_assets; i++) {
        if (equal(tag, assets->types[type].data[i].tag))
            return &assets->types[type].data[i].memory;
    }
    warning(0, "Could not find asset, type: %d, tag: %s", type, tag);
    return 0;
}

inline Bitmap* find_bitmap(Assets *assets, const char *tag) { return (Bitmap*) find_asset(assets, ASSET_TYPE_BITMAP, tag); }
inline Font*   find_font  (Assets *assets, const char *tag) { return (Font*)   find_asset(assets, ASSET_TYPE_FONT,   tag); }
inline Shader* find_shader(Assets *assets, const char *tag) { return (Shader*) find_asset(assets, ASSET_TYPE_SHADER, tag); }
inline Model*  find_model (Assets *assets, const char *tag) { return (Model*)  find_asset(assets, ASSET_TYPE_MODEL,  tag); }

#endif //ASSETS_H
