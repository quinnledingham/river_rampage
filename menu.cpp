internal void
draw_button(const Draw_Button button)
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

    draw_rect(button.coords, 0, button.dim, back_color);                                           // back
    if (button.text) draw_string(button.font, button.text, text_coords, pixel_height, text_color); // text
}

// buffer is the text that is in the textbox
function void
draw_textbox(const Draw_Textbox draw)
{
    platform_set_capability(PLATFORM_CAPABILITY_SCISSOR_TEST, true);
    v2 scissor_box_coords = draw.coords;
    scissor_box_coords.y += draw.dim.y;
    scissor_box_coords.y = (r32)renderer_window_dim.y - scissor_box_coords.y;
    platform_set_scissor_box(cv2(scissor_box_coords), cv2(draw.dim));

    f32 pixel_height = draw.dim.y;
    if (draw.dim.x < draw.dim.y) pixel_height = draw.dim.x;
    pixel_height *= 0.8f;

    //v2 text_dim = get_string_dim(draw.font, draw.buffer, pixel_height, draw.text_color);
    v2 font_dim = get_font_loaded_dim(draw.font, pixel_height);
    v2 text_coords = {};
    text_coords.x = draw.coords.x;
    text_coords.y = draw.coords.y + (draw.dim.y / 2.0f) + (font_dim.y / 2.0f);

    v2 text_dim_cursor = get_string_dim(draw.font, draw.buffer, draw.cursor_position, pixel_height, draw.text_color);
    v2 cursor_coords = draw.coords;
    cursor_coords.x += text_dim_cursor.x;

    draw_rect(draw.coords, 0.0f, draw.dim, draw.back_color);                                  // back
    if (draw.active) // clicked on
        draw_rect(cursor_coords, 0.0f, { draw.cursor_width, draw.dim.y }, draw.cursor_color); // cursor
    if (draw.buffer) // contains text
        draw_string(draw.font, draw.buffer, text_coords, pixel_height, draw.text_color);      // text

    platform_set_capability(PLATFORM_CAPABILITY_SCISSOR_TEST, false);
}

internal f32
get_pixel_height(v2 box)
{
    f32 pixel_height = box.y;
    if (box.x < box.y) pixel_height = box.x;
    pixel_height *= 0.8f;
    return pixel_height;
}


internal char
get_keyboard_input(Input *input) {
    return input->buffer[input->buffer_index++];
}

// default textbox updates
// input is a ascii value
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

// move cursor to mouse based on how the textbox will be drawn
internal u32
get_textbox_cursor_position(const Draw_Textbox *box, Button mouse_left, v2s mouse_coords) {
    u32 max_length = get_length(box->buffer);
    u32 cursor_pos = 0;
    while(1) {
        v2 cursor_dim = get_string_dim(box->font, box->buffer, cursor_pos, box->dim.y, box->text_color);
        if (mouse_coords.x <= (s32)cursor_dim.x + box->coords.x || cursor_pos >= max_length)
            break;
        else
            cursor_pos++;
    }
    return cursor_pos;
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
init_console(Console *console, Font *font)
{
    console->textbox = {
        { 0, 0 },
        { 0, 60 },
        { 50, 50, 50, 0.5f },

        font,
        0,
        4,
        { 255, 255, 255, 1.0f },

        true,
        0,
        2.0f,
        { 200, 200, 200, 1.0f },
    };

    console->edit = console->memory[0];
}

function void
console_clear_line(Console *console, char *line) {
    for (u32 i = 0; i < ARRAY_COUNT(console->memory[0]); i++) {
        line[i] = 0;
    }
}

inline void
console_clear_line(Console *console, u32 line) {
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
update_console(Console *console, Input *input) {
    if (input->buffer[0] == 0) 
        return true;

    u32 max_lines  = ARRAY_COUNT(console->memory)    - 1;
    u32 max_length = ARRAY_COUNT(console->memory[0]) - 1;
    u32 current_length = get_length(console->edit);

    s32 ch = 0;
    while((ch = get_keyboard_input(input)) != 0)
    {
        if (!is_ascii(ch)) 
            continue;

        if (ch != 9) 
            // any other input than autocomplete clear the search buffer
            console_clear_line(console, console->auto_complete); 

        switch(ch)
        {
            // Tab: Autocomplete
            case 9:  {
                if (console->auto_complete[0] == 0) copy_char_array(console->auto_complete, console->edit);

                // index 0 is do_nothing
                u32             index = console_auto_complete(console->auto_complete, console->last_auto_complete_index);
                if (index == 0) index = console_auto_complete(console->auto_complete, 0); // grab the first option

                if (index != 0) {
                    const char *command = pair_get_value(console_commands, CONSOLE_COMMANDS_COUNT, index);
                    console_clear_line(console, console->edit);
                    copy_char_array(console->edit, command);
                    console->cursor_position = get_length(console->edit);
                    console->last_auto_complete_index = index;
                }
            } break;

            case 27: // Esc: Close consle
                return false;
            break;

            case 38: // Up
            case 40: // Down
                if (ch == 38 && console->lines_up_index != 0)              console->lines_up_index--;
                if (ch == 40 && console->lines_up_index != console->lines) console->lines_up_index++;

                console->edit = console->memory[console->lines_up_index];
                console->cursor_position = get_length(console->edit);
            break;

            default: {
                // update_textbox returns true when enter/return is inputed
                if (update_textbox(console->edit, max_length, &console->cursor_position, ch)) {
                    for (u32 i = 0; i < CONSOLE_COMMANDS_COUNT; i++) {
                        const char *command = pair_get_value(console_commands, CONSOLE_COMMANDS_COUNT, i);
                        if (equal(console->edit, command)) console->command = i;
                    }

                    // check if there are still lines available
                    if (console->lines < max_lines) {
                        if (console->lines_up_index == console->lines) 
                            console->lines++;
                    } else {
                        log("update_console(): ran out of lines. console reset.");
                        for (u32 i = 0; i < max_lines + 1; i++)
                            console_clear_line(console, i);
                        console->lines = 0;
                    }

                    console->lines_up_index = console->lines;
                    console->edit = console->memory[console->lines];
                    console->cursor_position = get_length(console->edit);

                    return false;
                }
            } break;
        }
    }

    return true;
}

function void
draw_console(Console *console, v2s window_dim, Button mouse_left, v2s mouse_coords)
{
    console->textbox.coords.y = window_dim.y - console->textbox.dim.y;
    console->textbox.dim.x = (r32)window_dim.x;
    console->textbox.buffer = console->edit;
    if (is_down(mouse_left))
        console->cursor_position = get_textbox_cursor_position(&console->textbox, mouse_left, mouse_coords);
    console->textbox.cursor_position = console->cursor_position;
    draw_textbox(console->textbox);
}

internal void
add_onscreen_notification(Onscreen_Notifications *n, const char *not) {
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
update_onscreen_notifications(Onscreen_Notifications *n, r32 frame_time_s) {
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
draw_onscreen_notifications(Onscreen_Notifications *n, v2s window_dim, r32 frame_time_s) {
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

inline v2
float_textbox_get_dim(const Float_Textbox *box) {
    v2 dim = box->dim;
    dim.x = box->dim.x / (r32)box->src_elements;
    return dim;
}

inline v2
float_textbox_get_coords(const Float_Textbox *box, u32 index) {
    v2 dim = float_textbox_get_dim(box);
    v2 coords = box->coords;
    coords.x += index * dim.x;
    return coords;
}

inline r32*
float_textbox_get_element(const Float_Textbox *box, u32 index) {
    char *src = (char*)box->src + (index * box->element_size);
    return (r32*)src;
}

// finds what textbox the mouse clicked on
// if it is a new textbox it sets new_active to true to load in the number
// to the edit buffer
// returns true if a box was clicked on
internal b32
float_textbox_update_mouse(Float_Textbox *box, Button mouse_left, v2s mouse_coords) {
    b32 active_changed = false;
    s32 new_active = -1;

    if (!on_down(mouse_left)) 
        return active_changed;

    // find if a box had been clicked on
    for (u32 i = 0; i < box->src_elements; i++) {
        v2 coords = float_textbox_get_coords(box, i);
        v2 dim    = float_textbox_get_dim(box);
        if (inside_box(cv2(mouse_coords), coords, dim)) {
            new_active = i;
            break;
        }
    }

    if (new_active != box->active)
        active_changed = true;
    box->active = new_active;

    return active_changed;
}

// if the textbox is active it fills the buffer with the input
// if esc is pressed it deactivates the textbox
// if enter is pressed the value in the buffer is saved to the box src
internal b32
float_textbox_get_input(Float_Textbox *box, Input *input, char *buffer, u32 buffer_size, u32 *cursor_position) {
    b32 active_changed = false;

    if (box->active == -1)
        return active_changed;

    s32 ch = 0;
    while((ch = get_keyboard_input(input)) != 0) {
        if (!is_ascii(ch)) continue;

        switch(ch) {
            case 27: // Esc: leave textbox the same
                     // deactiving this textbox with mean no textbox is active
                     // which makes it switch the input mode to game
                box->active = -1;
                active_changed = true;
            break;

            default: {
                if (update_textbox(buffer, buffer_size - 1, cursor_position, ch)) {
                    // enter
                    f32 *src = float_textbox_get_element(box, box->active);
                    char_array_to_f32(buffer, src);
                }
            } break;
        }
    }

    return active_changed;
}

internal void
float_textbox_load_src_values(Float_Textbox *box) {
    for (u32 i = 0; i < box->src_elements; i++) {
        // copy temp to box memory
        f32 *src = float_textbox_get_element(box, i);
        const char *temp = ftos((f32)*src);
        for (u32 j = 0; j < float_digit_size - 1; j++) {
            box->memory[i][j] = temp[j];
        }
        platform_free((void*)temp);
    }
}

internal void
float_textbox_draw(Float_Textbox *box, Draw_Textbox draw, u32 *cursor_position,
                    char *active_buffer, Button mouse_left, v2s mouse_coords) {
    for (u32 i = 0; i < box->src_elements; i++) {
        draw.coords = float_textbox_get_coords(box, i);
        draw.dim = float_textbox_get_dim(box);
        
        if (box->active == i) {
            draw.active = true;
            
            // the active one draws the buffer that is being edited
            draw.buffer = active_buffer;

            // update cursor now that active coords and buffer are figured out
            // doing update cursor on draw because font is required to do it
            if (is_down(mouse_left)) 
                *cursor_position = get_textbox_cursor_position(&draw, mouse_left, mouse_coords);

            draw.cursor_position = *cursor_position;
            //log("cursor %d", draw.cursor_position
        } else {
            draw.active = false;
            draw.buffer = box->memory[i]; // values loaded from src values
        }

        draw_textbox(draw);
    }
}

internal Float_Textbox
f32_textbox() {
    Float_Textbox box = {};
    box.src_elements = 1;
    box.element_size = sizeof(f32);

    return box;
}

internal Float_Textbox
v3_textbox() {
    Float_Textbox box = {};
    box.src_elements = 3;
    box.element_size = sizeof(f32);

    return box;
}

internal void
init_camera_menu(Camera_Menu *menu, Assets *assets) {
    menu->draw = {
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

    menu->boxs[0] = v3_textbox();
    menu->boxs[1] = v3_textbox();
    menu->boxs[2] = v3_textbox();
    menu->boxs[3] = f32_textbox();
    menu->boxs[4] = f32_textbox();
    menu->boxs[5] = f32_textbox();
}

internal void
draw_camera_menu(Camera_Menu *menu, Camera *camera, Button mouse_left, v2s mouse_coords, Input *input) {

    u32 num_of_boxs = 6;
    u32 index = 0;
    menu->boxs[index++].src = &camera->position;
    menu->boxs[index++].src = &camera->target;
    menu->boxs[index++].src = &camera->up;
    menu->boxs[index++].src = &camera->fov;
    menu->boxs[index++].src = &camera->yaw;
    menu->boxs[index++].src = &camera->pitch;

    // figuring out which box is active
    b32 active_changed = false;
    for (u32 i = 0; i < num_of_boxs; i++) {
        b32 temp_active_changed = float_textbox_update_mouse(&menu->boxs[i], mouse_left, mouse_coords);
        if (temp_active_changed) // if there is one active changed
            active_changed = true;
    }

    // checking if a box is active and setting up the active box to be editted
    s32 active_float_textbox = -1;
    b8 active_textbox = false;
    for (u32 i = 0; i < num_of_boxs; i++) {
        // checking if any box is active
        if (menu->boxs[i].active != -1) {
            active_float_textbox = i;
            active_textbox = true;

            if (active_changed) {
                // if there is a new box that is active load value into edit buffer
                f32 *src = float_textbox_get_element(&menu->boxs[i], menu->boxs[i].active);
                const char *temp = ftos((f32)*src);
                for (u32 j = 0; j < float_digit_size - 1; j++) {
                    menu->buffer[j] = temp[j];
                }
                platform_free((void*)temp);

                input->mode = INPUT_MODE_KEYBOARD;

                break;
            }
        }
    }
    if (!active_textbox && active_changed) // no box is active
        input->mode = INPUT_MODE_GAME;

    // load input now that we know which box is active
    if (active_textbox) {
        // if true that means that the one active textbox was disabled
        if (float_textbox_get_input(&menu->boxs[active_float_textbox], input, menu->buffer, float_digit_size, &menu->cursor_position))
            input->mode = INPUT_MODE_GAME;
    }

    // the drawing
    v2 coords = { 0, 0 };
    v2 dim = { 125, 40 };
    dim.x *= 3.0f;
    for (u32 i = 0; i < num_of_boxs; i++) {
        menu->boxs[i].coords = coords;
        menu->boxs[i].dim = dim;
        float_textbox_load_src_values(&menu->boxs[i]);
        float_textbox_draw(&menu->boxs[i], menu->draw, &menu->cursor_position, menu->buffer, mouse_left, mouse_coords);
        coords.y += dim.y;
    }
}



internal void
draw_float_texboxes(Easy_Textboxs *easy, Button mouse_left, v2s mouse_coords, Input *input) {
    // figuring out which box is active
    b32 active_changed = false;
    for (u32 i = 0; i < easy->num_of_boxs; i++) {
        b32 temp_active_changed = float_textbox_update_mouse(&easy->boxs[i], mouse_left, mouse_coords);
        if (temp_active_changed) // if there is one active changed
            active_changed = true;
    }

    // checking if a box is active and setting up the active box to be editted
    s32 active_float_textbox = -1;
    b8 active_textbox = false;
    for (u32 i = 0; i < easy->num_of_boxs; i++) {// checking if any box is active
        if (easy->boxs[i].active != -1) {
            active_float_textbox = i;
            active_textbox = true;

            if (active_changed) {
                // if there is a new box that is active load value into edit buffer
                f32 *src = float_textbox_get_element(&easy->boxs[i], easy->boxs[i].active);
                const char *temp = ftos((f32)*src);
                for (u32 j = 0; j < float_digit_size - 1; j++) {
                    easy->edit.buffer[j] = temp[j];
                }
                platform_free((void*)temp);

                input->mode = INPUT_MODE_KEYBOARD;

                break;
            }
        }
    }
    if (!active_textbox && active_changed) // no box is active
        input->mode = INPUT_MODE_GAME;

    // load input now that we know which box is active
    if (active_textbox) {
        // if true that means that the one active textbox was disabled
        if (float_textbox_get_input(&easy->boxs[active_float_textbox], input, easy->edit.buffer, float_digit_size, &easy->edit.cursor_position))
            input->mode = INPUT_MODE_GAME;
    }

    // the drawing
    v2 coords = { 0, 0 };
    v2 dim = { 125, 40 };
    dim.x *= 3.0f;
    for (u32 i = 0; i < easy->num_of_boxs; i++) {
        easy->boxs[i].coords = coords;
        easy->boxs[i].dim = dim;
        float_textbox_load_src_values(&easy->boxs[i]);
        float_textbox_draw(&easy->boxs[i], easy->draw, &easy->edit.cursor_position, easy->edit.buffer, mouse_left, mouse_coords);
        coords.y += dim.y;
    }
}