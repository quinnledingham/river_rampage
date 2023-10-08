#ifndef DATA_STRUCTURES_H
#define DATA_STRUCTURES_H

struct LL_Node
{
    void *data;
    LL_Node *next;
    LL_Node *previous;
};

struct LL
{
    LL_Node *head;
    u32 size;
    
    u32 data_size;
};

struct AST_Node
{
    void *data;
    u32 data_size;
    
    AST_Node *parent;
    
    AST_Node **children;
    u32 num_of_children;
};

struct AST
{
    AST_Node *head;
};

struct Lexer
{
    File file;
    LL tokens;
    LL_Node *cursor;
    s32 line_num = 1;
    
    void *(*scan)(File *file, s32 *line_num);
    u32 token_size;
};

#endif //DATA_STRUCTURES_H
