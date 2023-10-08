enum MTL_Token_Type
{
    MTL_TOKEN_KEYWORD,
    MTL_TOKEN_NUMBER,
    MTL_TOKEN_STRING,
    MTL_TOKEN_ERROR,
    MTL_TOKEN_EOF
};

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

function void*
scan_mtl_v2(File *file, s32 *line_num)
{
    return 0;
}

function MTL_Token
scan_mtl(File *file, s32 *line_num)
{
    X:
    
    s32 ch;
    while((ch = get_char(file)) != EOF && (ch == 9 || ch == 13)); // remove tabs
    //log("%c %d", ch, file->size);
    switch(ch)
    {
        case EOF: { return { MTL_TOKEN_EOF, 0, ch }; } break;
        
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
                
                if (isalpha(sequence[0])) return { OBJ_TOKEN_ID, sequence };
                return { OBJ_TOKEN_NUMBER, sequence };
            }
            
            error(*line_num, "not a valid ch (%d)", ch);
        } break;
    }
    
    return { OBJ_TOKEN_ERROR, 0, ch };
}



function MTL_Token
mtl_lex(Lexer *lexer)
{
    if (lexer->cursor == 0 || lexer->cursor->next == 0) 
    {
        MTL_Token token = scan_mtl(&lexer->file, &lexer->line_num);
        MTL_Token *tok = (MTL_Token*)SDL_malloc(sizeof(MTL_Token));
        (*tok) = token;
        LL_Node *new_node = create_ll_node(tok);
        ll_add(&lexer->tokens, new_node);
        lexer->cursor = new_node;
    }
    else
    {
        lexer->cursor = lexer->cursor->next;
    }
    
    return *(MTL_Token*)lexer->cursor->data;
}

function void
mtl_unlex(Lexer *lexer)
{
    lexer->cursor = lexer->cursor->previous;
}

function MTL_Token
mtl_peek(Lexer *lexer)
{
    MTL_Token token = mtl_lex(lexer);
    mtl_unlex(lexer);
    return token;
}

enum MTL_Type
{
    MTL_STRING,
    MTL_FLOAT,
    MTL_INT,
};

struct MTL_Node_Info
{
    u32 type;
    
    union
    {
        const char *lexeme;
        f32 float_number;
        s32 int_number;
    };
    
};

function AST_Node*
ast_create_mtl_node(MTL_Node_Info mtl_info)
{
    MTL_Node_Info *info = (MTL_Node_Info*)SDL_malloc(sizeof(MTL_Node_Info));
    (*info) = mtl_info;
    return create_ast_node(info, sizeof(MTL_Node_Info));
}


function AST_Node*
mtl_parse(Lexer *lexer, AST_Node *head)
{
    if (head == 0)
    {
        MTL_Node_Info info = {};
        info.type = MTL_STRING;
        info.lexeme = "mtl_file";
        AST_Node *next = ast_create_mtl_node(info);
        
        while(equal(mtl_peek(lexer).lexeme, "newmtl"))
        {
            ast_add_child(next, mtl_parse(lexer, next));
        }
        return next;
    }
    
    MTL_Token token = mtl_lex(lexer);
    MTL_Node_Info *mtl_head = (MTL_Node_Info*)head->data;
    
    if (token.type == OBJ_TOKEN_ID)
    {
        if (equal(token.lexeme, "newmtl"))
        {
            if (equal(mtl_head->lexeme, "newmtl"))
            {
                
            }
            
            MTL_Node_Info info = {};
            info.type = MTL_STRING;
            info.lexeme = "newmtl";
            
            AST_Node *next = ast_create_mtl_node(info);
            
            MTL_Token next_token = mtl_lex(lexer);
            info = {};
            info.type = MTL_STRING;
            info.lexeme = next_token.lexeme;
            AST_Node *file_name = ast_create_mtl_node(info);
            ast_add_child(next, file_name);
            
            while(!equal(mtl_peek(lexer).lexeme, "newmtl") && mtl_peek(lexer).type != MTL_TOKEN_EOF)
                ast_add_child(next, mtl_parse(lexer, next));
            
            return next;
        }
        else if (equal(mtl_head->lexeme, "newmtl"))
        {
            if (equal(token.lexeme, "Ns") || equal(token.lexeme, "Ni") || 
                equal(token.lexeme, "d") || equal(token.lexeme, "illum") || equal(token.lexeme, "map_Kd"))
            {
                MTL_Node_Info info = {};
                info.type = MTL_STRING;
                info.lexeme = token.lexeme;
                
                AST_Node *next = ast_create_mtl_node(info);
                
                ast_add_child(next, mtl_parse(lexer, next));
                return next;
            }
            else if (equal(token.lexeme, "Ka") || equal(token.lexeme, "Ks") || equal(token.lexeme, "Ke"))
            {
                MTL_Node_Info info = {};
                info.type = MTL_STRING;
                info.lexeme = token.lexeme;
                
                AST_Node *next = ast_create_mtl_node(info);
                
                ast_add_child(next, mtl_parse(lexer, next));
                ast_add_child(next, mtl_parse(lexer, next));
                ast_add_child(next, mtl_parse(lexer, next));
                
                return next;
            }
        }
        else
        {
            MTL_Node_Info info = {};
            info.type = MTL_STRING;
            info.lexeme = token.lexeme;
            AST_Node *next = ast_create_mtl_node(info);
            return next;
        }
        
    }
    else if (!equal(mtl_head->lexeme, "newmtl") && token.type == OBJ_TOKEN_NUMBER)
    {
        MTL_Node_Info info = {};
        info.type = MTL_FLOAT;
        info.float_number = std::stof(token.lexeme, 0);
        AST_Node *next = ast_create_mtl_node(info);
        return next;
    }
    
    MTL_Node_Info info = {};
    info.type = MTL_STRING;
    info.lexeme = "yo";
    AST_Node *yo = ast_create_mtl_node(info);
    ast_add_child(head, yo);
    log("here");
    return yo;
}

function AST
mtl_parser(Lexer *lexer)
{
    AST ast = {};
    ast.head = mtl_parse(lexer, 0);
    return ast;
}