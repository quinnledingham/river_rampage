internal void
draw_button(Draw_Button button)
{
    v4 back_color = button.default_back_color;
    v4 text_color = button.default_text_color;
    
    if (button.active)
    {
        back_color = button.active_back_color;
        text_color = button.active_text_color;
    }

    f32 pixel_height = button.dim.y;
    if (button.dim.x < button.dim.y) pixel_height = button.dim.x;
    pixel_height *= 0.8f;

    v2 text_dim = get_string_dim(button.font, button.text, pixel_height, text_color);
    v2 text_coords = {};
    text_coords.x = button.coords.x + (button.dim.x / 2.0f) - (text_dim.x / 2.0f);
    text_coords.y = button.coords.y + (button.dim.y / 2.0f) + (text_dim.y / 2.0f);

    draw_rect(button.coords, 0, button.dim, back_color);
    draw_string(button.font, button.text, text_coords, pixel_height, text_color);
}

// buffer is the text that is in the textbox
function void
draw_textbox(Draw_Textbox *box)
{
    f32 pixel_height = box->dim.y;
    if (box->dim.x < box->dim.y) pixel_height = box->dim.x;
    pixel_height *= 0.8f;

    v2 text_dim = get_string_dim(box->font, box->buffer, pixel_height, box->text_color);
    v2 text_coords = {};
    //text_coords.x = box->coords.x + (box->dim.x / 2.0f) - (text_dim.x / 2.0f);
    text_coords.y = box->coords.y + (box->dim.y / 2.0f) + (text_dim.y / 2.0f);

    v2 text_dim_cursor = get_string_dim(box->font, box->buffer, box->cursor_position, pixel_height, box->text_color);
    v2 cursor_coords = box->coords;
    cursor_coords.x += text_dim_cursor.x;
    //cursor_coords.y -= box->dim.y;

    draw_rect(box->coords, 0.0f, box->dim, box->back_color);
    draw_rect(cursor_coords, 0.0f, { box->cursor_width, box->dim.y }, box->cursor_color);
    if (box->buffer != 0)
        draw_string(box->font, box->buffer, text_coords, pixel_height, box->text_color);
}


// index  = number of button
// active = which index is active
// press  = interact key is pressed
function b8
menu_button(Menu *menu, const char *text, u32 index, u32 active, u32 press)
{
    Draw_Button button = {
        menu->button_style.default_back_color,
        menu->button_style.default_text_color,

        menu->button_style.active_back_color,
        menu->button_style.active_text_color,

        false,

        menu->rect.coords,
        menu->button_style.dim,

        menu->font,
        text
    };

    b8 button_pressed = false;
    if (index == active && press) button_pressed = true;
    if (index == active) button.active = true;

    draw_button(button);
    
    menu->rect.coords.y += menu->button_style.dim.y + menu->padding.y;
    
    return button_pressed;
}

function void
menu_update_active(s32 *active, s32 lower, s32 upper, Button increase, Button decrease)
{
    if (on_down(increase))
    {
        (*active)++;
        if (*active > upper)
            *active = upper;
    }
    if (on_down(decrease))
    {
        (*active)--;
        if (*active < lower)
            *active = lower;
    }
}

function void
init_console(Console *console, Assets *assets)
{
    console->font = find_font(assets, "CASLON");

    console->textbox = {
        { 0, 0 },
        { 0, 60 },
        { 50, 50, 50, 0.5f },

        console->font,
        0,
        4,
        { 255, 255, 255, 1.0f },

        0,
        2.0f,
        { 200, 200, 200, 1.0f },
    };

    console->textbox.buffer = console->memory[0];
    console->textbox.cursor_position = get_length(console->textbox.buffer);
    memset(console->auto_complete, 0, 90);
}

// make sure that the indices match up between the enum and char**
// +2 convert from char** to enum
enum Console_Commands
{
    DO_NOTHING,
    HIDE_CONSOLE,

    TOGGLE_WIREFRAME,
    RELOAD_SHADERS,
    TOGGLE_FPS,
};
global const char *console_commands[3] = { "/toggle_wireframe", "/reload_shaders", "/toggle_fps"};

function b32
console_command(Console *console, u32 looking_for)
{
    if (console->command == looking_for) {
        console->command = 0;
        return true;
    }
    else return false;
}

function void
console_clear_line(Console *console, char *line)
{
    for (u32 i = 0; i < ARRAY_COUNT(console->memory[0]); i++) {
        line[i] = 0;
    }
}

function void
console_clear_line(Console *console, u32 line)
{
    console_clear_line(console, console->memory[line]);
}

function b32
update_console(Console *console, Input *input)
{
    u32 max_lines  = ARRAY_COUNT(console->memory)    - 1;
    u32 max_length = ARRAY_COUNT(console->memory[0]) - 1;

    if (input->buffer[0] != 0)
    {
        u32 current_length = get_length(console->textbox.buffer);

        // scan for special inputs
        const char *ptr = input->buffer;
        while(*ptr != 0)
        {
            if (*ptr == 9) // Tab: autocomplete
            {
                if (console->auto_complete[0] == 0) copy_char_array(console->auto_complete, console->textbox.buffer);

                for (u32 i = 0; i < ARRAY_COUNT(console_commands); i++)
                {
                    if (equal_start(console_commands[i], console->auto_complete) && !equal(console_commands[i], console->textbox.buffer))
                    {
                        console_clear_line(console, console->textbox.buffer);
                        copy_char_array(console->textbox.buffer, console_commands[i]);
                        console->textbox.cursor_position = get_length(console->textbox.buffer);
                        break;
                    }
                }
            }
            else console_clear_line(console, console->auto_complete);

            if      (*ptr == 8 && console->textbox.cursor_position != 0) // Backspace: remove char
            { 
                for(u32 i = console->textbox.cursor_position; i < current_length; i++)
                {
                    console->textbox.buffer[i - 1] = console->textbox.buffer[i];
                }
                console->textbox.buffer[current_length - 1] = 0;
                console->textbox.cursor_position--;
            }
            else if (*ptr == 13) // return: do command/newline
            {
                //if      (equal(console->textbox.buffer, console_commands[0])) console->command = TOGGLE_WIREFRAME;
                //else if (equal(console->textbox.buffer, console_commands[1])) console->command = RELOAD_SHADERS;

                for (u32 i = 0; i < ARRAY_COUNT(console_commands); i++) {
                    if (equal(console->textbox.buffer, console_commands[i])) console->command = i + 2;
                }

                if (console->lines < max_lines) // check if there are still lines available
                {
                    if (console->lines_up_index == console->lines) console->lines++;
                }
                else 
                {
                    log("update_console(): ran out of lines. console reset.");
                    for (u32 i = 0; i < max_lines + 1; i++) console_clear_line(console, i);
                    console->lines = 0;
                }

                console->lines_up_index = console->lines;
                console->textbox.buffer = console->memory[console->lines];
                console->textbox.cursor_position = get_length(console->textbox.buffer);

                input->mode = INPUT_MODE_GAME;
                return false;
            }
            else if (*ptr == 27) // Esc
            {
                input->mode = INPUT_MODE_GAME;
                return false;
            }
            else if (*ptr == 37 && console->textbox.cursor_position != 0) console->textbox.cursor_position--; // Left
            else if (*ptr == 39 && console->textbox.cursor_position != current_length) console->textbox.cursor_position++; // Right
            else if (*ptr == 38 || *ptr == 40) // Up: move thorugh command history
            {
                if (*ptr == 38 && console->lines_up_index != 0)              console->lines_up_index--;
                if (*ptr == 40 && console->lines_up_index != console->lines) console->lines_up_index++;

                console->textbox.buffer = console->memory[console->lines_up_index];
                console->textbox.cursor_position = get_length(console->textbox.buffer);
            }
            else if ((isalpha(*ptr) || *ptr == '/' || *ptr == '_') && current_length < max_length)
            {
                for(s32 i = current_length - 1; i > (s32)console->textbox.cursor_position; i--)
                {
                    console->textbox.buffer[i + 1] = console->textbox.buffer[i];
                }
                console->textbox.buffer[console->textbox.cursor_position] = *ptr;
                console->textbox.buffer[current_length + 1] = 0;
                console->textbox.cursor_position++;
            }
            ptr++;
        }
    }

    return true;
}

function void
draw_console(Console *console, v2s window_dim)
{
    console->textbox.coords.y = window_dim.y - console->textbox.dim.y;
    console->textbox.dim.x = window_dim.x;

    draw_textbox(&console->textbox);
}
