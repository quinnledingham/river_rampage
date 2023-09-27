struct Menu_Button_Style
{
    v2 dim;
    
    v4 default_back_color;
    v4 active_back_color;
    v4 default_text_color;
    v4 active_text_color;
};

struct Menu
{
    Menu_Button_Style button_style;
    Font *font;
    
    v2 padding;
    v2 coords;
    v2 dim;
};

// index  = number of button
// active = which index is active
// press  = interact key is pressed
function b8
menu_button(Menu *menu, const char *text, u32 index, u32 active, u32 press)
{
    f32 pixel_height = menu->button_style.dim.y;
    v4 back_color = menu->button_style.default_back_color;
    v4 text_color = menu->button_style.default_text_color;
    
    b8 button_pressed = false;
    if (index == active && press) button_pressed = true;
    if (index == active)
    {
        back_color = menu->button_style.active_back_color;
        text_color = menu->button_style.active_text_color;
    }
    
    // drawing
    draw_rect(menu->coords, 0, menu->button_style.dim, back_color);
    
    v2 text_dim = get_string_dim(menu->font, text, pixel_height, text_color);
    f32 text_x_coord = menu->coords.x + (menu->button_style.dim.x / 2.0f) - (text_dim.x / 2.0f);
    f32 text_y_coord = menu->coords.y + (menu->button_style.dim.y / 2.0f) + (text_dim.y / 2.0f);
    draw_string(menu->font, text, { text_x_coord, text_y_coord }, pixel_height, text_color);
    
    menu->coords.y += menu->button_style.dim.y + menu->padding.y;
    
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