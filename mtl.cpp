struct MTL_Token
{
    s32 type;
    union
    {
        const char *lexeme;
        float float_num;
        s32 int_num;
    };
    s32 ch;
};


struct Mtl
{
    Material *materials; // loaded from the corresponding material file
    u32 materials_count;
};

enum MTL_Token_Type
{
    MTL_TOKEN_KEYWORD,
    MTL_TOKEN_NUMBER,
    MTL_TOKEN_STRING,
    MTL_TOKEN_ERROR,
    MTL_TOKEN_EOF
};


static const s32 mtl_valid_chars[5] = { '-', '.', '_', ':', '/' };

function b8
is_valid_mtl_char(s32 ch)
{
    for (s32 i = 0; i < ARRAY_COUNT(mtl_valid_chars); i++)
    {
        if (ch == mtl_valid_chars[i]) return true;
    }
    return false;
}

function MTL_Token*
create_mtl_token(MTL_Token token)
{
    MTL_Token *mtl = (MTL_Token*)SDL_malloc(sizeof(MTL_Token));
    (*mtl) = token;
    return mtl;
}

function void*
scan_mtl(File *file, s32 *line_num)
{
    X:
    
    s32 ch;
    while((ch = get_char(file)) != EOF && (ch == 9 || ch == 13)); // remove tabs
    //log("%c %d", ch, file->size);
    switch(ch)
    {
        case EOF: { return (void*)create_mtl_token({ MTL_TOKEN_EOF, 0, ch }); } break;
        
        case '\n':
        {
            (*line_num)++;
            goto X;
        } break;
        
        case '#':
        {
            while((ch = get_char(file)) != EOF && (ch != '\n'));
            unget_char(file);
            goto X;
        } break;
        
        case ' ':
        {
            goto X;
        } break;
        
        default:
        {
            if (isalpha(ch) || isdigit(ch) || is_valid_mtl_char(ch))
            {
                int length = 0;
                do
                {
                    ch = get_char(file);
                    length++;
                } while((isalpha(ch) || isdigit(ch) || is_valid_mtl_char(ch)) && ch != ' ' && ch != EOF);
                
                unget_char(file);
                const char *sequence = copy_last_num_of_chars(file, length);
                
                if (isalpha(sequence[0])) return (void*)create_mtl_token({ MTL_TOKEN_KEYWORD, sequence });
                return (void*)create_mtl_token({ MTL_TOKEN_NUMBER, sequence });
            }
            
            error(*line_num, "not a valid ch (%d)", ch);
        } break;
    }
    
    return (void*)create_mtl_token({ MTL_TOKEN_ERROR, 0, ch });
}

function v3
parse_v3(Lexer *lexer)
{
    v3 result = {};
    MTL_Token *token = 0;
    token = (MTL_Token*)lex(lexer);
    result.x = std::stof(token->lexeme);
    token = (MTL_Token*)lex(lexer);
    result.y = std::stof(token->lexeme);
    token = (MTL_Token*)lex(lexer);
    result.z = std::stof(token->lexeme);
    return result;
}

function Mtl
load_mtl(const char *path, const char *filename)
{
    Mtl mtl = {};
    
    //char filepath[80];
    //memset(filepath, 0, 80);
    //strcat(strcat(filepath, path), filename);
    const char *filepath = char_array_insert(path, get_length(path), filename);

    Lexer lexer = {};
    lexer.file = read_file(filepath);

    if (!lexer.file.size) { error("load_mtl: could not read material file"); return mtl; }
    lexer.scan = &scan_mtl;
    lexer.token_size = sizeof(MTL_Token);
    
    // count new materials
    MTL_Token *token = (MTL_Token*)lex(&lexer);
    while (token->type != MTL_TOKEN_EOF)
    {
        if (equal(token->lexeme, "newmtl")) mtl.materials_count++;
        token = (MTL_Token*)lex(&lexer);
    }
    
    //print_ll(&lexer.tokens);
    
    mtl.materials = ARRAY_MALLOC(Material, mtl.materials_count);
    reset_lex(&lexer);
    
    u32 material_index = 0;
    Material material = {};
    token = 0;
    do
    {
        token = (MTL_Token*)lex(&lexer);
        
        if (equal(token->lexeme, "newmtl"))
        {
            if (material.id != 0)
            {
                mtl.materials[material_index++] = material;
            }
            
            material = {};
            token = (MTL_Token*)lex(&lexer);
            material.id = token->lexeme;
        }
        else if (equal(token->lexeme, "Ns"))
        {
            token = (MTL_Token*)lex(&lexer);
            material.specular_exponent = std::stof(token->lexeme, 0);
        }
        else if (equal(token->lexeme, "Ka"))
        {
            material.ambient = parse_v3(&lexer);
        }
        else if (equal(token->lexeme, "Kd"))
        {
            material.diffuse = parse_v3(&lexer);
        }
        else if (equal(token->lexeme, "Ks"))
        {
            material.specular = parse_v3(&lexer);
        }
        else if (equal(token->lexeme, "map_Kd"))
        {
            token = (MTL_Token*)lex(&lexer);
            const char *diffuse_map_filepath = char_array_insert(path, get_length(path), token->lexeme);
            material.diffuse_map = load_bitmap(diffuse_map_filepath);
        }
    } while(token->type != MTL_TOKEN_EOF);
    
    if (material.id != 0)
    {
        mtl.materials[material_index++] = material;
    }
    
    platform_free((void*)filepath);
    free_file(&lexer.file);
    
    return mtl;
}
