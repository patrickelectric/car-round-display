#include <Arduino.h>
#include <SPI.h>


#include <lvgl.h>
#include <Arduino_GFX_Library.h>

#include "utils/debug.h"

#define LV_HOR_RES_MAX 240
#define LV_VER_RES_MAX 240

void lvgl_init() {
    static Arduino_DataBus *bus = new Arduino_ESP32SPI(
        2 /* DC */,
        10 /* CS */,
        6 /* SCK */,
        7 /* MOSI */,
        GFX_NOT_DEFINED /* MISO */
    );
    static Arduino_GC9A01 *gfx = new Arduino_GC9A01(bus);

    gfx->begin();
    /* Invert display color
     * | Color  | Normal RGB565 | Inverted for Display |
     * |--------|---------------|----------------------|
     * | Red    | `0xF800`      | `~0xF800 = 0x07FF`   |
     * | Green  | `0x07E0`      | `~0x07E0 = 0xF81F`   |
     * | Blue   | `0x001F`      | `~0x001F = 0xFFE0`   |
     * | White  | `0xFFFF`      | `~0xFFFF = 0x0000`   |
     * | Black  | `0x0000`      | `~0x0000 = 0xFFFF`   |
     * | Yellow | `0xFFE0`      | `~0xFFE0 = 0x001F`   |
     */
    gfx->invertDisplay(1);

    static lv_disp_draw_buf_t draw_buf;
    lv_color_t* buf = (lv_color_t*)malloc(LV_HOR_RES_MAX * LV_VER_RES_MAX * sizeof(lv_color_t));
    lv_disp_draw_buf_init(&draw_buf, buf, NULL, LV_HOR_RES_MAX * LV_VER_RES_MAX);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = gfx->width();
    disp_drv.ver_res = gfx->height();
    disp_drv.flush_cb = [](lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p) {
        uint32_t w = (area->x2 - area->x1 + 1);
        uint32_t h = (area->y2 - area->y1 + 1);
        gfx->draw16bitRGBBitmap(area->x1, area->y1, (uint16_t *)&color_p->full, w, h);
    };
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);
}

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