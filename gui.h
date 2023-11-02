struct Draw_Button
{
    v4 default_back_color;
    v4 default_text_color;

    v4 active_back_color;
    v4 active_text_color;

    b32 active;

    v2 coords;
    v2 dim;

    Font *font;
    const char *text;
};

struct Draw_Textbox
{
    v2 coords;
    v2 dim;
    v4 back_color;

    Font *font;
    char *buffer; // edit
    u32 buffer_size;
    v4 text_color;

    u32 cursor_position; // edit
    r32 cursor_width;
    v4 cursor_color;
};

struct Menu_Button_Style
{
    v2 dim;
    
    v4 default_back_color;
    v4 active_back_color;
    v4 default_text_color;
    v4 active_text_color;
};


struct Menu_Component
{
    v2 coords;
    v2 dim;
};

struct Menu
{
    Menu_Button_Style button_style;
    Font *font;
    
    v2 padding;

    Rect rect; // coords and dim of menu
};


struct Console
{
    char auto_complete[90];

    char memory[100][90]; // 100 lines, 90 chars per line
    u32 lines;
    u32 lines_up_index;

    Font *font;

    Draw_Textbox textbox;

    u32 command; // the command to execute
};

struct Onscreen_Notifications
{
    char memory[10][90];
    u32 lines; // number of lines of memory used
    f32 times[10]; // how long left on displaying the line
    v4 colors[10]; // the color for each line

    Font *font;
    v4 text_color;
};