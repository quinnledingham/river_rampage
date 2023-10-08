//
// Linked List
//

function LL_Node*
create_ll_node(void *data)
{
    LL_Node *node = (LL_Node*)SDL_malloc(sizeof(LL_Node));
    *node = {};
    node->data = data;
    return node;
}

function void
ll_add(LL *list, LL_Node *new_node)
{
    if (list->size == 0) 
    {
        list->head = create_ll_node(0);
        list->head->next = new_node;
        new_node->previous = list->head;
    }
    else
    {
        LL_Node *node = list->head;
        for (u32 i = 0; i < list->size - 1; i++) node = node->next;
        node->next = new_node;
        new_node->previous = node;
    }
    list->size++;
}

//
// Abstract Syntax Tree
//

function AST_Node*
create_ast_node(void *data, u32 data_size)
{
    AST_Node *node = (AST_Node*)SDL_malloc(sizeof(AST_Node));
    *node = {};
    node->data = data;
    node->data_size = data_size;
    return node;
}

function void
ast_add_child(AST_Node *node, AST_Node *child)
{
    AST_Node **new_children = ARRAY_MALLOC(AST_Node*, (node->num_of_children + 1));
    memset(new_children, 0, sizeof(AST_Node*) * (node->num_of_children + 1));
    
    if (node->num_of_children != 0)
    {
        memcpy(new_children, node->children, sizeof(AST_Node*) * node->num_of_children);
        SDL_free(node->children);
    }
    
    node->children = new_children;
    node->num_of_children++;
    
    node->children[node->num_of_children - 1] = child;
    
    //printf("%p\n", child);
}

function void
ast_traverse_left_to_right(s32 level, void *args, AST_Node *node, void (action)(s32 level, void *data, void *args))
{
    action(level, node, args);
    
    for (s32 i = 0; i < node->num_of_children; i++)
    {
        ast_traverse_left_to_right(level + 1, args, node->children[i], action);
    }
}

//
// Lexer
//

function void*
lex(Lexer *lexer)
{
    if (lexer->cursor == 0 || lexer->cursor->next == 0) 
    {
        void *token = lexer->scan(&lexer->file, &lexer->line_num);
        void *tok = SDL_malloc(lexer->token_size);
        SDL_memcpy(tok, token, lexer->token_size);
        LL_Node *new_node = create_ll_node(tok);
        ll_add(&lexer->tokens, new_node);
        lexer->cursor = new_node;
    }
    else
    {
        lexer->cursor = lexer->cursor->next;
    }
    
    return lexer->cursor->data;
}

function void
unlex(Lexer *lexer)
{
    lexer->cursor = lexer->cursor->previous;
}

function void*
peek(Lexer *lexer)
{
    void *token = lex(lexer);
    unlex(lexer);
    return token;
}