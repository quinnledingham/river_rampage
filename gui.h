//
// Draw_<> structs are meant to be structs to pass arguments for the draw functions.
// Try not use them to store information about a button or textbox. This is because they
// are meant to be shared between many draw calls, changing only what needs to be changed
// between calls (coords).
//

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
    char *buffer;
    u32 buffer_size;
    v4 text_color;

    // Cursor
    u32 active; // if active draw cursor
    u32 cursor_position;
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

enum Console_Commands
{
    DO_NOTHING,
    HIDE_CONSOLE,
    TOGGLE_WIREFRAME,
    RELOAD_SHADERS,
    TOGGLE_FPS,
    ALIGN_CAMERA,
    TOGGLE_CAMERA_MENU,

    CONSOLE_COMMANDS_COUNT
};

// make sure that the indices match up between the enum and char**
// +2 convert from char** to enum
global const Pair console_commands[CONSOLE_COMMANDS_COUNT] = {
    { DO_NOTHING,         "/do_nothing"       },
    { HIDE_CONSOLE,       "/hide_console"     },
    { TOGGLE_WIREFRAME,   "/toggle_wireframe" },
    { RELOAD_SHADERS,     "/reload_shaders"   },
    { TOGGLE_FPS,         "/toggle_fps"       },
    { ALIGN_CAMERA,       "/align_camera"     },
    { TOGGLE_CAMERA_MENU, "/toggle_camera_menu" },
};

struct Console
{
    char auto_complete[90];
    u32 last_auto_complete_index;

    char memory[100][90]; // 100 lines, 90 chars per line
    u32 lines;
    u32 lines_up_index;

    char *edit; // points to the line in memory
    u32 cursor_position; // cursor to edit

    Draw_Textbox textbox; // console textbox style

    u32 command; // the command to execute
};

inline b32
console_command(Console *console, u32 looking_for)
{
    if (console->command == looking_for) {
        console->command = 0;
        return true;
    }
    else return false;
}

struct Onscreen_Notifications
{
    char memory[10][90];
    u32 lines; // number of lines of memory used
    f32 times[10]; // how long left on displaying the line
    v4 colors[10]; // the color for each line

    Font *font;
    v4 text_color;
};

global const u32 float_digit_size = 15;

struct Float_Textbox {
    void *src;
    u32 src_elements;
    u32 element_size;

    v2 coords;
    v2 dim;

    char memory[4][float_digit_size]; // 4 to fit up to a v4 type

    s32 active = -1;
};

struct Edit_Float_Textbox {
    char buffer[float_digit_size];
    u32 cursor_position;
};

struct Easy_Textboxs {
    Float_Textbox boxs[10];
    u32 num_of_boxs;

    Edit_Float_Textbox edit;
    Draw_Textbox draw;
};

global const Draw_Textbox default_draw_textbox = {
    { 0, 0 },
    { 125, 40 },
    { 50, 50, 50, 0.5f },

    0,
    0,
    4,
    { 255, 255, 255, 1.0f },

    false,
    0,
    2.0f,
    { 200, 200, 200, 1.0f },
};

struct Camera_Menu {
    Float_Textbox boxs[6];

    char buffer[float_digit_size];
    u32 cursor_position;

    Draw_Textbox draw; // the textbox design
};
