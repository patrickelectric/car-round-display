#include <Arduino.h>
#include <SPI.h>

#include "display.h"
#include "screen.h"
#include "utils/debug.h"

void setup()
{
    Serial.begin(115200);
    delay(1000);

    setup_screen();

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
