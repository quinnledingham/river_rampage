internal f32
get_pixel_height(v2 box)
{
    f32 pixel_height = box.y;
    if (box.x < box.y) pixel_height = box.x;
    pixel_height *= 0.8f;
    return pixel_height;
}

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

internal char
get_keyboard_input(Input *input) {
    return input->buffer[input->buffer_index++];
}

// default textbox updates
internal b32
update_textbox(char *buffer, u32 max_length, u32 *cursor_position, s32 input) {
    u32 current_length = get_length(buffer);

    switch(input) {
        
        case 8: { // Backspace: delete char
            if (*cursor_position == 0) return false;

            for(u32 i = *cursor_position; i < current_length; i++) {
                buffer[i - 1] = buffer[i];
            }
            buffer[current_length - 1] = 0;
            (*cursor_position)--;
        } break;

        case 13: { // Enter/Return: return true to tell calling code to do something
            return true;
        } break;

        case 37: { // Left
            if (*cursor_position != 0)
                (*cursor_position)--; // Left
        } break;

        case 39: { // Right
            if (*cursor_position != current_length)
                (*cursor_position)++; // Right
        } break;
        
        default: { // Add char to char_array in textbox
            if (current_length >= max_length) return false;

            for(s32 i = current_length - 1; i >= (s32)(*cursor_position); i--) {
                buffer[i + 1] = buffer[i];
            }
            buffer[*cursor_position] = input;
            buffer[current_length + 1] = 0;
            (*cursor_position)++;
        } break;
    }

    return false;
}

// buffer is the text that is in the textbox
function void
draw_textbox(Draw_Textbox *box)
{
    f32 pixel_height = box->dim.y;
    if (box->dim.x < box->dim.y) pixel_height = box->dim.x;
    pixel_height *= 0.8f;

    //v2 text_dim = get_string_dim(box->font, box->buffer, pixel_height, box->text_color);
    v2 font_dim = get_font_loaded_dim(box->font, pixel_height);
    v2 text_coords = {};
    //text_coords.x = box->coords.x + (box->dim.x / 2.0f) - (font_dim.x / 2.0f);
    text_coords.x = box->coords.x;
    text_coords.y = box->coords.y + (box->dim.y / 2.0f) + (font_dim.y / 2.0f);

    v2 text_dim_cursor = get_string_dim(box->font, box->buffer, box->cursor_position, pixel_height, box->text_color);
    v2 cursor_coords = box->coords;
    cursor_coords.x += text_dim_cursor.x;
    //cursor_coords.y -= box->dim.y;

    draw_rect(box->coords, 0.0f, box->dim, box->back_color);
    if (box->active) draw_rect(cursor_coords, 0.0f, { box->cursor_width, box->dim.y }, box->cursor_color);
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

        true,
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
    ALIGN_CAMERA,
    TOGGLE_CAMERA_MENU,

    CONSOLE_COMMANDS_COUNT
};

global const Pair console_commands[CONSOLE_COMMANDS_COUNT] = {
    { DO_NOTHING,       "/do_nothing"       },
    { HIDE_CONSOLE,     "/hide_console"     },
    { TOGGLE_WIREFRAME, "/toggle_wireframe" },
    { RELOAD_SHADERS,   "/reload_shaders"   },
    { TOGGLE_FPS,       "/toggle_fps"       },
    { ALIGN_CAMERA,     "/align_camera"     },
    { TOGGLE_CAMERA_MENU, "/toggle_camera_menu" },
};

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

internal u32
console_auto_complete(const char *auto_complete, u32 last_auto_complete_index) {
    for (u32 i = 0; i < CONSOLE_COMMANDS_COUNT; i++) {
        const char *temp = pair_get_value(console_commands, CONSOLE_COMMANDS_COUNT, i);
        if (equal_start(temp, auto_complete) && last_auto_complete_index < i) {
            return i;
        }
    }
    return 0;
}

function b32
update_console(Console *console, Input *input)
{
    if (input->buffer[0] == 0) return true;

    u32 max_lines  = ARRAY_COUNT(console->memory)    - 1;
    u32 max_length = ARRAY_COUNT(console->memory[0]) - 1;
    u32 current_length = get_length(console->textbox.buffer);

    s32 ch = 0;
    while((ch = get_keyboard_input(input)) != 0)
    {
        if (!is_ascii(ch)) continue;

        if (ch != 9) console_clear_line(console, console->auto_complete); // any other input than autocomplete clear the search buffer

        switch(ch)
        {
            // Tab: Autocomplete
            case 9:  {
                if (console->auto_complete[0] == 0) copy_char_array(console->auto_complete, console->textbox.buffer);

                // index 0 is do_nothing
                u32             index = console_auto_complete(console->auto_complete, console->last_auto_complete_index);
                if (index == 0) index = console_auto_complete(console->auto_complete, 0); // grab the first option

                if (index != 0) {
                    const char *command = pair_get_value(console_commands, CONSOLE_COMMANDS_COUNT, index);
                    console_clear_line(console, console->textbox.buffer);
                    copy_char_array(console->textbox.buffer, command);
                    console->textbox.cursor_position = get_length(console->textbox.buffer);
                    console->last_auto_complete_index = index;
                }
            } break;

            case 27: // Esc: Close consle
                input->mode = INPUT_MODE_GAME;
                return false;
            break;

            case 38: // Up
            case 40: // Down
                if (ch == 38 && console->lines_up_index != 0)              console->lines_up_index--;
                if (ch == 40 && console->lines_up_index != console->lines) console->lines_up_index++;

                console->textbox.buffer = console->memory[console->lines_up_index];
                console->textbox.cursor_position = get_length(console->textbox.buffer);
            break;

            default: {
                // update_textbox returns true when enter/return is inputed
                if (update_textbox(console->textbox.buffer, max_length, &console->textbox.cursor_position, ch)) {
                    for (u32 i = 0; i < CONSOLE_COMMANDS_COUNT; i++) {
                        const char *command = pair_get_value(console_commands, CONSOLE_COMMANDS_COUNT, i);
                        if (equal(console->textbox.buffer, command)) console->command = i;
                    }

                    if (console->lines < max_lines) // check if there are still lines available
                    {
                        if (console->lines_up_index == console->lines) console->lines++;
                    } else {
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
            } break;
        }
    }

    return true;
}

function void
draw_console(Console *console, v2s window_dim)
{
    console->textbox.coords.y = window_dim.y - console->textbox.dim.y;
    console->textbox.dim.x = (r32)window_dim.x;

    draw_textbox(&console->textbox);
}

internal void
add_onscreen_notification(Onscreen_Notifications *n, const char *not)
{
    if (n->lines == ARRAY_COUNT(n->memory)) {
        log("add_onscreen_notification(): too many notfications");
        return;
    }

    n->times[n->lines] = 1.0f;
    n->colors[n->lines] = n->text_color;

    u32 not_length = get_length(not);
    for (u32 ch = 0; ch < not_length; ch++) {
        n->memory[n->lines][ch] = not[ch];
    }   
    n->lines++;
}

internal void
update_onscreen_notifications(Onscreen_Notifications *n, r32 frame_time_s)
{
    for (u32 i = 0; i < n->lines; i++) {
        n->times[i] -= frame_time_s;

        n->colors[i].a = n->times[i];

        if (n->times[i] <= 0.0f) {
            //for (u32 ch = 0; ch < ARRAY_COUNT(n->memory[i]); ch++) {
            //    n->memory[i][ch] = 0;
            //}
            n->colors[i].a = 0.0f;

            // Last line that was added is dissappearing so you can start adding from the beginning again.
            if (i == n->lines - 1) {
                n->lines = 0;
            }
        }
    }
}

internal void
draw_onscreen_notifications(Onscreen_Notifications *n, v2s window_dim, r32 frame_time_s)
{
    update_onscreen_notifications(n, frame_time_s);

    f32 pixel_height = get_pixel_height(cv2(window_dim) * 0.1f);

    f32 above_text_coord = 0.0f;
    for (u32 i = 0; i < n->lines; i++)
    {
        v2 text_dim = get_string_dim(n->font, n->memory[i], pixel_height, n->text_color);
        v2 text_coords = {};
        text_coords.x = (window_dim.x / 2.0f) - (text_dim.x / 2.0f);
        text_coords.y = above_text_coord + text_dim.y + 10.0f;

        draw_string(n->font, n->memory[i], text_coords, pixel_height, n->colors[i]);

        above_text_coord = text_coords.y;
    }
}


internal void
init_camera_menu(Camera_Menu *menu, Assets *assets) {
    menu->textbox = {
        { 0, 0 },
        { 125, 40 },
        { 50, 50, 50, 0.5f },

        find_font(assets, "CASLON"),
        0,
        4,
        { 255, 255, 255, 1.0f },

        false,
        0,
        2.0f,
        { 200, 200, 200, 1.0f },
    };

    menu->active = -1;
}

internal b8
inside_box(v2 coords, v2 box_coords, v2 box_dim) {
    if (box_coords.x <= coords.x && coords.x <= box_coords.x + box_dim.x &&
        box_coords.y <= coords.y && coords.y <= box_coords.y + box_dim.y)
        return true;

    return false;
}

internal void
get_v3_textbox(Camera_Menu *menu, v2 start, v2 dim, u32 index) {
    menu->coords[index + 0] = start;
    start.x += dim.x;
    menu->coords[index + 1] = start;
    start.x += dim.x;
    menu->coords[index + 2] = start;
}

internal void
update_camera_menu_textbox(char *buffer, u32 max_length, u32 *cursor_position, Input *input) {


}

internal void
update_camera_menu(Camera_Menu *menu, Camera *camera, Input *input, Button mouse_left, v2s mouse_coords) {
    v2 box = menu->textbox.coords;

    get_v3_textbox(menu, box, menu->textbox.dim, 0);
    box.y += menu->textbox.dim.y;
    get_v3_textbox(menu, box, menu->textbox.dim, 3);
    box.y += menu->textbox.dim.y;
    get_v3_textbox(menu, box, menu->textbox.dim, 6);
    box.y += menu->textbox.dim.y;

    menu->coords[9] = box;
    box.y += menu->textbox.dim.y;
    menu->coords[10] = box;
    box.y += menu->textbox.dim.y;
    menu->coords[11] = box;

    if (on_down(mouse_left)) {
        s32 new_active = -1; // default assume that a box was not clicked

        // find if a box had been clicked on
        for (u32 i = 0; i < ARRAY_COUNT(menu->coords); i++) {
            if (inside_box(cv2(mouse_coords), menu->coords[i], menu->textbox.dim)) {
                new_active = i;
                menu->textbox.cursor_position = get_length(menu->memory[i]);
                input->mode = INPUT_MODE_KEYBOARD;
                break;
            }
        }

        if (new_active == -1) // no longer is there a active textbox
            input->mode = INPUT_MODE_GAME;
        if (new_active != menu->active) // switch textboxes or left all textboxes so save written value
            char_array_to_f32(menu->memory[menu->active], &camera->E[menu->active]);

        menu->active = new_active;
        //printf("ACTIVE: %d\n", menu->active);
    }

    if (menu->active != -1) {
        s32 ch = 0;
        while((ch = get_keyboard_input(input)) != 0) {
            if (!is_ascii(ch)) continue;

            switch(ch) {
                case 27: // Esc: leave textbox the same
                    menu->active = -1;
                    input->mode = INPUT_MODE_GAME;
                break;

                default: {
                    if (update_textbox(menu->memory[menu->active], float_digit_size - 1, &menu->textbox.cursor_position, ch)) {
                        char_array_to_f32(menu->memory[menu->active], &camera->E[menu->active]);
                        //menu->active = -1;
                        //input->mode = INPUT_MODE_GAME;
                    }
                } break;
            }
            
        }
    }

    for (u32 i = 0; i < 12; i++) {
        const char *temp = ftos(camera->E[i]);
        if (menu->active == i) continue; // don't update box being written to
        for (u32 j = 0; j < float_digit_size - 1; j++) {
            menu->memory[i][j] = temp[j];
        }
        platform_free((void*)temp);
    }
}

internal void
draw_camera_menu(Camera_Menu *menu, Camera *camera) {
    for (u32 i = 0; i < ARRAY_COUNT(menu->coords); i++) {
        menu->textbox.coords = menu->coords[i];
        menu->textbox.buffer = menu->memory[i];
        if (menu->active == i) menu->textbox.active = true;
        else                   menu->textbox.active = false;
        draw_textbox(&menu->textbox);        
    }

    // back to defaults
    menu->textbox.coords = { 0, 0 }; 
}

internal void
v3_textbox(Textbox_V3 *tb_v3, Input *input, Button mouse_left, v2s mouse_coords) {
    // check if active textbox should change
    if (on_down(mouse_left)) {
        s32 new_active = -1; // default assume that a box was not clicked

        // find if a box had been clicked on
        for (u32 i = 0; i < 3; i++) {
            v2 coords = tb_v3->coords;
            coords.x += tb_v3->individual_dim.x * (f32)i;
            if (inside_box(cv2(mouse_coords), coords, tb_v3->individual_dim)) {
                new_active = i;
                tb_v3->textbox.cursor_position = get_length(tb_v3->memory[i]);
                input->mode = INPUT_MODE_KEYBOARD;
                break;
            }
        }

        if (new_active == -1) // no longer is there a active textbox
            input->mode = INPUT_MODE_GAME;
        if (new_active != tb_v3->active) // switch textboxes or left all textboxes so save written value
            char_array_to_f32(tb_v3->memory[tb_v3->active], &tb_v3->src->E[tb_v3->active]);

        tb_v3->active = new_active;
        printf("ACTIVE: %d\n", tb_v3->active);
    }

    // process input
    if (tb_v3->active != -1) {
        s32 ch = 0;
        while((ch = get_keyboard_input(input)) != 0) {
            if (!is_ascii(ch)) continue;

            switch(ch) {
                case 27: // Esc: leave textbox the same
                    tb_v3->active = -1;
                    input->mode = INPUT_MODE_GAME;
                break;

                default: {
                    if (update_textbox(tb_v3->memory[tb_v3->active], float_digit_size - 1, &tb_v3->textbox.cursor_position, ch)) {
                        char_array_to_f32(tb_v3->memory[tb_v3->active], &tb_v3->src->E[tb_v3->active]);
                    }
                } break;
            }
            
        }
    }

    // load new values from v3
    for (u32 i = 0; i < 3; i++) {
        const char *temp = ftos(tb_v3->src->E[i]);
        if (tb_v3->active == i) continue; // don't update box being written to
        for (u32 j = 0; j < float_digit_size - 1; j++) {
            tb_v3->memory[i][j] = temp[j];
        }
        platform_free((void*)temp);
    }

    // draw textboxes to screen
    for (u32 i = 0; i < 3; i++) {
        tb_v3->textbox.coords = tb_v3->coords;
        tb_v3->textbox.coords.x += tb_v3->individual_dim.x * (f32)i;
        tb_v3->textbox.buffer = tb_v3->memory[i];
        if (tb_v3->active == i) tb_v3->textbox.active = true;
        else                    tb_v3->textbox.active = false;
        draw_textbox(&tb_v3->textbox);
    }
}