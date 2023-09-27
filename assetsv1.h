#ifndef ASSETS_H
#define ASSETS_H

struct File
{
    u32 size;
    void *memory;
};

struct Bitmap
{
    u32 handle;
    u8 *memory;
    v2s dim;
    s32 pitch;
    s32 channels;
};

struct Shader
{
    const char *vs_filename; //.vs vertex_shader
    const char *tcs_filename; //.tcs tessellation control shader
    const char *tes_filename; //.tes tessellation evaluation shader
    const char *gs_filename; //.gs geometry shader
    const char *fs_filename; //.fs fragment shader
    
    const char *vs_file;
    const char *tcs_file;
    const char *tes_file;
    const char *gs_file;
    const char *fs_file;
    
    b32 compiled;
    u32 handle;
};

const char *basic_vs = "#version 330 core\n layout (location = 0) in vec3 position; layout (location = 1) in vec3 normal; layout (location = 2) in vec2 texture_coords; uniform mat4 model; uniform mat4 projection; uniform mat4 view; out vec2 uv; void main(void) { gl_Position = projection * view * model * vec4(position, 1.0f); uv = texture_coords;}";
const char *color_fs = "#version 330 core\n in vec2 uv; uniform vec4 user_color; out vec4 FragColor; void main() { FragColor  = vec4(user_color.x/255, user_color.y/255, user_color.z/255, user_color.w);}";
const char *tex_fs = "#version 330 core\n uniform sampler2D tex0; in vec2 uv; out vec4 FragColor; void main() { vec4 tex = texture(tex0, uv); FragColor = tex;}";

struct Vertex
{
    v3 position;
    v3 normal;
    v2 texture_coordinate;
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
};

function u32
use_shader(Shader *shader)
{
    glUseProgram(shader->handle);
    return shader->handle;
}

struct Font
{
    
};

//
// Loading Assets
//

enum Asset_Types
{
    ASSET_TYPE_FONT,
    ASSET_TYPE_BITMAP,
    ASSET_TYPE_SHADER,
};

struct Asset
{
    u32 type;
    const char *tag;
    union
    {
        Bitmap bitmap;
        Shader shader;
    };
};

struct Asset_Load_Info
{
    u32 type;
    u32 index;
    const char *tag;
    const char *filename;
};

struct Assets
{
    u32 num_of_assets;
    
    Asset_Load_Info *info;
    u32 num_of_info_loaded;
    
    // Storage of assets
    Asset *bitmaps;
    u32 num_of_bitmaps;
    
    Asset *shaders;
    u32 num_of_shaders;
    
    Asset *fonts;
    u32 num_of_fonts;
};

function Bitmap*
find_bitmap(Assets *assets, const char *tag)
{
    for (u32 i = 0; i < assets->num_of_bitmaps; i++)
        if (equal(tag, assets->bitmaps[i].tag)) return &assets->bitmaps[i].bitmap;
    
    warning(0, "Could not find bitmap with tag: %s", tag);
    
    return 0;
}

enum Asset_Token_Types
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

const char *asset_keywords[3] = { "FONTS", "BITMAPS", "SHADERS" };

function b32
is_asset_keyword(const char *word)
{
    for (u32 i = 0; i < ARRAY_COUNT(asset_keywords); i++) 
        if (equal(word, asset_keywords[i])) return true;
    
    return false;
}

function b32
is_file_path_ch(s32 ch)
{
    switch(ch)
    {
        case '.':
        case '/':
        case '-':
        case '_':
        return true;
        
        default:
        return false;
    }
}

function b32
is_valid_body_ch(s32 ch)
{
    if (isalpha(ch) || isdigit(ch) || is_file_path_ch(ch)) return true;
    else return false;
}

function b32
is_valid_start_ch(s32 ch)
{
    if (isalpha(ch) || is_file_path_ch(ch)) return true;
    else return false;
}

function void
add_asset_load_info(void *data, void *args)
{
    Assets *assets = (Assets*)data;
    Asset_Load_Info *info = (Asset_Load_Info*)args;
    
    switch(info->type)
    {
        case ASSET_TYPE_FONT:   info->index = assets->num_of_fonts;   assets->num_of_fonts++;   break;
        case ASSET_TYPE_BITMAP: info->index = assets->num_of_bitmaps; assets->num_of_bitmaps++; break;
        case ASSET_TYPE_SHADER: info->index = assets->num_of_shaders; assets->num_of_shaders++; break;
    }
    
    assets->info[assets->num_of_info_loaded++] = *info;
}

function void
count_asset(void *data, void *args)
{
    Assets *assets = (Assets*)data;
    assets->num_of_assets++;
}

#endif //ASSETS_H
