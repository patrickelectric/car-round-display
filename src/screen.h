#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <lvgl.h>
#include "utils/debug.h"

LV_FONT_DECLARE(font_awesome);

class Screen {
public:
    static Screen& instance() {
        static Screen instance;
        return instance;
    }

    void setup();
    void updateValue(float value);
    void updateBluetoothIcon(bool connected);

    // Delete copy constructor and assignment operator
    Screen(const Screen&) = delete;
    Screen& operator=(const Screen&) = delete;

    // Friend functions for timer callbacks
    friend void screen_update_timer(lv_timer_t* timer);
    friend void meter_update_timer(lv_timer_t* timer);

private:
    Screen() = default;  // Private constructor
    ~Screen() = default;

    // Member variables
    lv_obj_t* meter = nullptr;
    lv_meter_indicator_t* needle = nullptr;
    lv_obj_t* text = nullptr;
    lv_obj_t* bluetooth_icon = nullptr;
    lv_obj_t* gas_icon = nullptr;
    lv_obj_t* wifi_icon = nullptr;
    float cbar_value = 0;
};
