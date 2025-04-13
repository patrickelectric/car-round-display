#include <Arduino.h>
#include <SPI.h>

#include "display.h"
#include "utils/debug.h"

void setup()
{
    Serial.begin(115200);
    delay(1000);

    // Turn on backlight
    pinMode(3, OUTPUT);
    digitalWrite(3, HIGH);
    debug("Starting...");

    lv_init();
    lvgl_init();

    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_black(), 0);
    lv_obj_set_scrollbar_mode(lv_scr_act(), LV_SCROLLBAR_MODE_OFF);

    // Create meter
    static lv_obj_t *meter = lv_meter_create(lv_scr_act());
    lv_obj_set_style_text_color(meter, lv_color_make(0, 0, 255), LV_PART_TICKS);
    lv_obj_center(meter);
    lv_obj_set_size(meter, 280, 280);

    // Set background color to black
    lv_obj_set_style_bg_color(meter, lv_color_black(), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(meter, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);

    // Add meter scale
    lv_meter_scale_t *scale = lv_meter_add_scale(meter);
    lv_meter_set_scale_range(meter, scale, 0, 100, 270, 90);

    // Set blue ticks
    lv_meter_set_scale_ticks(meter, scale, 11, 2, 10, lv_color_make(0, 0, 255));
    lv_meter_set_scale_major_ticks(meter, scale, 1, 4, 15, lv_color_make(0, 0, 255), 10);

    // Red needle
    static lv_meter_indicator_t *needle = lv_meter_add_needle_line(meter, scale, 4, lv_color_make(255, 0, 0), -35);
    lv_obj_set_style_shadow_color(meter, lv_color_make(255, 0, 0), LV_PART_ITEMS);
    lv_obj_set_style_shadow_width(meter, 20, LV_PART_ITEMS);
    lv_obj_set_style_shadow_opa(meter, LV_OPA_50, LV_PART_ITEMS);
    lv_meter_set_indicator_value(meter, needle, 0);

    // Create a black circle to cover the needle
    static lv_obj_t *circle = lv_obj_create(lv_scr_act());
    lv_obj_set_style_bg_color(circle, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(circle, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_border_color(circle, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_border_width(circle, 1, LV_PART_MAIN);
    lv_obj_set_style_radius(circle, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    const auto radius = 80;
    lv_obj_set_size(circle, radius, radius);
    lv_obj_center(circle);

    // Create a white text in the middle
    static lv_obj_t *text = lv_label_create(lv_scr_act());
    lv_label_set_text(text, "0");
    lv_obj_set_style_text_color(text, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_font(text, &lv_font_montserrat_34, LV_PART_MAIN);
    lv_obj_center(text);

    // Timers
    static auto update_meter = lv_timer_create([](lv_timer_t * timer) {
        static int16_t val = 0;
        if (val == 0) {
            lv_obj_invalidate(lv_scr_act());
        }
        static int8_t dir = 1;
        val += dir;
        if (val >= 100 || val <= 0) dir = -dir;
        lv_meter_set_indicator_value(meter, needle, val);
        lv_label_set_text(text, String(val).c_str());
    }, 30, NULL);

    static auto print_status = lv_timer_create([](lv_timer_t * timer) {
        auto freeSRAM = ESP.getFreeHeap();
        auto freePSRAM = ESP.getFreePsram();

        debug("Free heap: %u bytes", ESP.getFreeHeap());
        debug("Free PSRAM: %u bytes", ESP.getFreePsram());
        debug("Heap size: %u bytes", ESP.getHeapSize());
        debug("Min free heap: %u bytes", ESP.getMinFreeHeap());
        debug("Max alloc heap: %u bytes", ESP.getMaxAllocHeap());
        debug("CPU Temp: %.2f °C", temperatureRead());
        debug("Uptime: %lu ms", millis());
    }, 1000, NULL);
}

void loop()
{
    lv_timer_handler();
    delay(5);
}
