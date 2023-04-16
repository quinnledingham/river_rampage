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
    else 
        error(0, "Cannot open file %s", filename);
    
    return result;
}

// copies the next n = length of chars from the input file
// and returns them in a string
function const char*
copy_from_file(FILE *input_file, u32 length)
{
    char *string = (char*)malloc(length + 1);
    memset(string, 0, length + 1);
    
    for (u32 i = 0; i < length; i++)
    {
        int ch = fgetc(input_file);
        if (ch == EOF)
        {
            warning(0, "copy_from_file hit the EOF");
            break;
        }
        string[i] = ch;
    }
    return string;
}

//
// Bitmap
//

function Bitmap
load_bitmap(const char *filename)
{
    Bitmap bitmap = {};
    bitmap.memory = stbi_load(filename, &bitmap.dim.width, &bitmap.dim.height, &bitmap.channels, 0);
    if (bitmap.memory == 0) error("load_bitmap() could not load bitmap %s", filename);
    return bitmap;
}

function void
init_bitmap_handle(Bitmap *bitmap)
{
    glGenTextures(1, &bitmap->handle);
    glBindTexture(GL_TEXTURE_2D, bitmap->handle);
    
    if (bitmap->channels == 3)
    {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, bitmap->dim.width, bitmap->dim.height, 0, GL_RGB, GL_UNSIGNED_BYTE, bitmap->memory);
    }
    else if (bitmap->channels == 4)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bitmap->dim.width, bitmap->dim.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, bitmap->memory);
    }
    glGenerateMipmap(GL_TEXTURE_2D);
    
    // Tile
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glBindTexture(GL_TEXTURE_2D, 0);
}

function Bitmap
load_and_init_bitmap(const char *filename)
{
    Bitmap bitmap = load_bitmap(filename);
    init_bitmap_handle(&bitmap);
    return bitmap;
}

//
// Shader
//

function const char*
load_shader_file(const char* filename)
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
    
    return shader_file;
}

function void
load_shader(Shader *shader)
{
    if (shader->vs_filename == 0)
    {
        error("load_opengl_shader() must have a vertex shader");
        return;
    }
    
    // Free all files
    if (shader->vs_file != 0)  free((void*)shader->vs_file);
    if (shader->tcs_file != 0) free((void*)shader->tcs_file);
    if (shader->tes_file != 0) free((void*)shader->tes_file);
    if (shader->gs_file != 0)  free((void*)shader->gs_file);
    if (shader->fs_file != 0)  free((void*)shader->fs_file);
    
    // Load files
    if (shader->vs_filename != 0)  shader->vs_file = load_shader_file(shader->vs_filename);
    if (shader->tcs_filename != 0) shader->tcs_file = load_shader_file(shader->tcs_filename);
    if (shader->tes_filename != 0) shader->tes_file = load_shader_file(shader->tes_filename);
    if (shader->gs_filename != 0)  shader->gs_file = load_shader_file(shader->gs_filename);
    if (shader->fs_filename != 0)  shader->fs_file = load_shader_file(shader->fs_filename);
}

function bool
compile_shader(u32 handle, const char *file, int type)
{
    u32 s =  glCreateShader((GLenum)type);
    glShaderSource(s, 1, &file, NULL);
    glCompileShader(s);
    
    GLint compiled_s = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &compiled_s);
    if (!compiled_s)
    {
        opengl_debug(GL_SHADER, s);
        error("compile_shader() could not compile %s", glGetString(type));
    }
    else
        glAttachShader(handle, s);
    
    glDeleteShader(s);
    
    return compiled_s;
}

function void
compile_shader(Shader *shader)
{
    shader->compiled = false;
    if (shader->handle != 0) glDeleteProgram(shader->handle);
    shader->handle = glCreateProgram();
    
    if (shader->vs_file != 0)  compile_shader(shader->handle, shader->vs_file, GL_VERTEX_SHADER);
    if (shader->tcs_file != 0) compile_shader(shader->handle, shader->tcs_file, GL_TESS_CONTROL_SHADER);
    if (shader->tes_file != 0) compile_shader(shader->handle, shader->tes_file, GL_TESS_EVALUATION_SHADER);
    if (shader->gs_file != 0)  compile_shader(shader->handle, shader->gs_file, GL_GEOMETRY_SHADER);
    if (shader->fs_file != 0)  compile_shader(shader->handle, shader->fs_file,GL_FRAGMENT_SHADER);
    
    // Link
    glLinkProgram(shader->handle);
    GLint linked_program = 0;
    glGetProgramiv(shader->handle, GL_LINK_STATUS, &linked_program);
    if (!linked_program)
    {
        opengl_debug(GL_PROGRAM, shader->handle);
        error("load_opengl_shader() link failed");
        return;
    }
    
    shader->compiled = true;
}

//
// Mesh
//

function void
init_mesh(Mesh *mesh)
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

function void
draw_mesh(Mesh *mesh)
{
    glBindVertexArray(mesh->vao);
    glDrawElements(GL_TRIANGLES, mesh->indices_count, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

//
// Asset File Reading
//

function Asset_Token
scan_asset_file(FILE *file, s32 *line_num, Asset_Token last_token)
{
    X:
    
    s32 ch;
    while((ch = fgetc(file)) != EOF && (ch == 32 || ch == 9 || ch == 13)); // remove whitespace
    
    switch(ch)
    {
        case EOF: return { -1, 0 }; break;
        
        case '\n':
        {
            (*line_num)++; 
            goto X;
        } break;
        
        case ':':
        case ',':
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
                    ch = fgetc(file);
                    length++;
                } while(is_valid_body_ch(ch));
                ungetc(ch, file);
                
                fseek(file, -length, SEEK_CUR);
                const char *sequence = copy_from_file(file, length);
                
                if (is_asset_keyword(sequence))
                    return { ATT_KEYWORD, sequence };
                
                return { ATT_ID, sequence };
            }
            
            error(*line_num, "not a valid ch");
        } break;
    }
    
    return { ATT_ERROR, 0 };
}

// action is what happens when all the parts of an asset are found
function void
parse_asset_file(Assets *assets, FILE *file, void (action)(void *data, void *args))
{
    u32 type = 0;
    const char *tag;
    const char *filename;
    
    Asset_Token last_token = {};
    Asset_Token tok = {};
    s32 line_num = 1;
    while (tok.type != -1)
    {
        last_token = tok;
        tok = scan_asset_file(file, &line_num, tok);
        //printf("%d, %s\n", tok.type, tok.lexeme);
        
        switch(tok.type)
        {
            case ATT_KEYWORD:
            {
                if (equal(tok.lexeme, "FONTS")) type = ASSET_TYPE_FONT;
                else if (equal(tok.lexeme, "BITMAPS")) type = ASSET_TYPE_BITMAP;
                else if (equal(tok.lexeme, "SHADERS")) type = ASSET_TYPE_SHADER;
                
                tok = scan_asset_file(file, &line_num, tok);
                if (!equal(tok.lexeme, ":")) 
                {
                    error(line_num, "expected ':'");
                    return;
                }
            } break;
            
            case ATT_ID:
            {
                if (!equal(last_token.lexeme, ","))
                {
                    tag = tok.lexeme;
                    
                    tok = scan_asset_file(file, &line_num, tok);
                    if (!equal(tok.lexeme, ",")) 
                    {
                        error(line_num, "expected ','");
                        return;
                    }
                }
                else
                {
                    filename = tok.lexeme;
                    
                    Asset_Load_Info info = { type, 0,  tag, filename };
                    action((void*)assets, (void*)&info);
                }
            } break;
            
            case ATT_SEPERATOR:
            {
                error(line_num, "unexpected seperator");
            } break;
        }
    }
}

function void
load_assets(Assets *assets, const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file == 0) error("load_assets(): failed to open file %s", filename);
    
    parse_asset_file(assets, file, count_asset);
    assets->info = ARRAY_MALLOC(Asset_Load_Info, assets->num_of_assets);
    fseek(file, 0, SEEK_SET);
    parse_asset_file(assets, file, add_asset_load_info);
    fclose(file);
    
    assets->fonts = ARRAY_MALLOC(Asset, assets->num_of_fonts);
    assets->bitmaps = ARRAY_MALLOC(Asset, assets->num_of_bitmaps);
    assets->shaders = ARRAY_MALLOC(Asset, assets->num_of_shaders);
    
    for (u32 i = 0; i < assets->num_of_assets; i++)
    {
        Asset_Load_Info *info = &assets->info[i];
        //printf("asset: %d, %s, %s\n", info->type, info->tag, info->filename);
        
        Asset asset = {};
        asset.type = info->type;
        asset.tag = info->tag;
        
        switch(asset.type)
        {
            case ASSET_TYPE_FONT: 
            {
                //asset.font = load_font(info->filename); 
                assets->fonts[info->index] = asset;
            } break;
            
            case ASSET_TYPE_BITMAP: 
            {
                asset.bitmap = load_and_init_bitmap(info->filename); 
                assets->bitmaps[info->index] = asset;
            } break;
        }
    }
}