#include "screen.h"
#include "display.h"

LV_FONT_DECLARE(font_awesome);

void Screen::updateValue(float value) {
    cbar_value = value * 10;
}

void Screen::updateBluetoothIcon(bool connected) {
    auto color = connected ? lv_palette_main(LV_PALETTE_GREEN) : lv_palette_main(LV_PALETTE_RED);
    lv_obj_set_style_text_color(bluetooth_icon, color, LV_PART_MAIN);
}

void screen_update_timer(lv_timer_t* timer) {
    lv_obj_invalidate(lv_scr_act());
}

void meter_update_timer(lv_timer_t* timer) {
    auto& screen = Screen::instance();
    static float old_value = 0;
    old_value = old_value * 0.6 + screen.cbar_value * 0.4;

    lv_meter_set_indicator_value(screen.meter, screen.needle, old_value);
    lv_label_set_text(screen.text, String(static_cast<int>(old_value)).c_str());
}

void Screen::setup() {
    const auto screen_size = 274;

    // Turn on backlight
    pinMode(3, OUTPUT);
    digitalWrite(3, HIGH);
    debug("Starting...");

    lv_init();
    lvgl_init();

    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_black(), 0);
    lv_obj_set_scrollbar_mode(lv_scr_act(), LV_SCROLLBAR_MODE_OFF);

    // Create meter
    meter = lv_meter_create(lv_scr_act());
    lv_obj_set_style_text_color(meter, lv_color_make(0, 0, 255), LV_PART_TICKS);
    lv_obj_center(meter);
    lv_obj_set_size(meter, screen_size, screen_size);

    // Set background color to black
    lv_obj_set_style_bg_color(meter, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(meter, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Add meter scale
    lv_meter_scale_t *scale = lv_meter_add_scale(meter);
    lv_meter_set_scale_range(meter, scale, -60, 150, 270, 90);

    // Set blue ticks
    lv_meter_set_scale_ticks(meter, scale, 15, 2, 10, lv_palette_main(LV_PALETTE_BLUE));
    lv_meter_set_scale_major_ticks(meter, scale, 1, 3, 15, lv_palette_main(LV_PALETTE_BLUE), 10);

    // Add a green arc to the start
    needle = lv_meter_add_arc(meter, scale, 3, lv_palette_main(LV_PALETTE_GREEN), 0);
    lv_meter_set_indicator_start_value(meter, needle, -60);
    lv_meter_set_indicator_end_value(meter, needle, 0);

    // Make the tick lines green at the start of the scale
    needle = lv_meter_add_scale_lines(meter, scale, lv_palette_main(LV_PALETTE_GREEN), lv_palette_main(LV_PALETTE_GREEN), false, 0);
    lv_meter_set_indicator_start_value(meter, needle, -60);
    lv_meter_set_indicator_end_value(meter, needle, 0);

    // Add a red arc to the end
    needle = lv_meter_add_arc(meter, scale, 3, lv_palette_main(LV_PALETTE_RED), 0);
    lv_meter_set_indicator_start_value(meter, needle, 105);
    lv_meter_set_indicator_end_value(meter, needle, 150);

    // Make the tick lines red at the end of the scale
    needle = lv_meter_add_scale_lines(meter, scale, lv_palette_main(LV_PALETTE_RED), lv_palette_main(LV_PALETTE_RED), false, 0);
    lv_meter_set_indicator_start_value(meter, needle, 105);
    lv_meter_set_indicator_end_value(meter, needle, 150);

    // Red needle
    needle = lv_meter_add_needle_line(meter, scale, 4, lv_color_make(255, 0, 0), -35);
    lv_obj_set_style_shadow_color(meter, lv_color_make(255, 0, 0), LV_PART_ITEMS);
    lv_obj_set_style_shadow_width(meter, 20, LV_PART_ITEMS);
    lv_obj_set_style_shadow_opa(meter, LV_OPA_50, LV_PART_ITEMS);
    lv_meter_set_indicator_value(meter, needle, 0);

    { // Create a black circle to cover the needle
        static lv_obj_t *circle = lv_obj_create(lv_scr_act());
        lv_obj_set_style_bg_color(circle, lv_color_black(), LV_PART_MAIN);
        lv_obj_set_style_bg_opa(circle, LV_OPA_COVER, LV_PART_MAIN);
        lv_obj_set_style_border_color(circle, lv_color_black(), LV_PART_MAIN);
        lv_obj_set_style_border_width(circle, 1, LV_PART_MAIN);
        lv_obj_set_style_radius(circle, LV_RADIUS_CIRCLE, LV_PART_MAIN);
        const auto radius = 80;
        lv_obj_set_size(circle, radius, radius);
        lv_obj_center(circle);
    }

    // Create a white text in the middle
    text = lv_label_create(lv_scr_act());
    lv_label_set_text(text, "0");
    lv_obj_set_style_text_color(text, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(text, &lv_font_montserrat_34, LV_PART_MAIN);
    lv_obj_center(text);

    // cbar text under main value
    static lv_obj_t *text_sub = lv_label_create(lv_scr_act());
    lv_label_set_text(text_sub, "cbar");
    lv_obj_set_style_text_color(text_sub, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(text_sub, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_obj_align(text_sub, LV_ALIGN_CENTER, 0, 20);

    {
        bluetooth_icon = lv_label_create(lv_scr_act());
        lv_label_set_text(bluetooth_icon, "\uf293");
        lv_obj_set_style_text_color(bluetooth_icon, lv_palette_main(LV_PALETTE_GREY), LV_PART_MAIN);
        lv_obj_set_style_text_font(bluetooth_icon, &font_awesome, LV_PART_MAIN);
        const auto radius = screen_size / 3;
        const auto angle = M_PI / 4;
        const auto x = radius * cos(angle);
        const auto y = radius * sin(angle);
        lv_obj_align(bluetooth_icon, LV_ALIGN_CENTER, x, y);
    }

    if (false) {
        gas_icon = lv_label_create(lv_scr_act());
        lv_label_set_text(gas_icon, "\uf52f");
        lv_obj_set_style_text_color(gas_icon, lv_palette_main(LV_PALETTE_GREEN), LV_PART_MAIN);
        lv_obj_set_style_text_font(gas_icon, &font_awesome, LV_PART_MAIN);
        const auto radius = screen_size / 3;
        const auto angle = M_PI / 8;
        const auto x = radius * cos(angle);
        const auto y = radius * sin(angle);
        lv_obj_align(gas_icon, LV_ALIGN_CENTER, x, y);
    }

    if (false) {
        wifi_icon = lv_label_create(lv_scr_act());
        lv_label_set_text(wifi_icon, "\uf1b9");
        lv_obj_set_style_text_color(wifi_icon, lv_palette_main(LV_PALETTE_GREEN), LV_PART_MAIN);
        lv_obj_set_style_text_font(wifi_icon, &font_awesome, LV_PART_MAIN);
        const auto radius = screen_size / 3;
        const auto angle = 3 * M_PI / 8;
        const auto x = radius * cos(angle);
        const auto y = radius * sin(angle);
        lv_obj_align(wifi_icon, LV_ALIGN_CENTER, x, y);
    }

    // Repaint screen to take screen of artifacts
    static auto update_screen = lv_timer_create(screen_update_timer, 1000, NULL);

    // Update meter
    static auto update_meter = lv_timer_create(meter_update_timer, 10, NULL);

    // Update screen
    lv_obj_invalidate(lv_scr_act());
}