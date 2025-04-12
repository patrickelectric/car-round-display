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

    lv_obj_t *box = lv_obj_create(lv_scr_act());
    lv_obj_set_style_bg_color(box, lv_color_hex(0x0000FF), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(box, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_align(box, LV_ALIGN_CENTER, 0, 0);

    lv_obj_t *btn = lv_btn_create(lv_scr_act());
    lv_obj_t *label2 = lv_label_create(btn);
    lv_label_set_text(label2, "Click me");
    lv_obj_align(btn, LV_ALIGN_CENTER, 0, 0);
}

void loop()
{
    lv_task_handler();
    auto freeSRAM = ESP.getFreeHeap();
    auto freePSRAM = ESP.getFreePsram();

    debug("Free heap: %u bytes", ESP.getFreeHeap());
    debug("Free PSRAM: %u bytes", ESP.getFreePsram());
    debug("Heap size: %u bytes", ESP.getHeapSize());
    debug("Min free heap: %u bytes", ESP.getMinFreeHeap());
    debug("Max alloc heap: %u bytes", ESP.getMaxAllocHeap());
    debug("CPU Temp: %.2f °C", temperatureRead());
    debug("Uptime: %lu ms", millis());
    delay(1000);
}

