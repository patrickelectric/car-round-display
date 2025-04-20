#include <Arduino.h>
#include <SPI.h>

#include "display.h"
#include "obd.h"
#include "screen.h"
#include "utils/debug.h"

void setup()
{
    Serial.begin(115200);
    delay(1000);
    debug("Starting ESP32-C3 OBD-II BLE Client");
    BLEDevice::init("ESP32-C3 OBD Client");

    setup_screen();

    // Connect to OBD device
    if (connectToOBD()) {
      debug("Connected to OBD device successfully!");

      // Initialize OBD with standard AT commands
      initializeOBD();
    } else {
      debug("Failed to connect to OBD device");
    }

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

    static auto reconnect_bluetooth = lv_timer_create([](lv_timer_t * timer) {
        static unsigned long lastReconnectAttempt = 0;
        unsigned long currentTime = millis();
        if (currentTime - lastReconnectAttempt > 10000) {
            lastReconnectAttempt = currentTime;
            debug("Attempting to reconnect...");
            if (connectToOBD()) {
                debug("Reconnected to OBD device successfully!");
                initializeOBD();
            } else {
                debug("Failed to reconnect to OBD device");
            }
        }
    }, 1000, NULL);

    static auto ask_data = lv_timer_create([](lv_timer_t * timer) {
        //sendOBDCommand("0105"); // Engine coolant temperature
        //sendOBDCommand("010C"); // Engine RPM
        sendOBDCommand("010B"); // MAP (Manifold Absolute Pressure)
        sendOBDCommand("0133"); // Barometric pressure
        //sendOBDCommand("015E"); // Request available PIDs
        //sendOBDCommand("010D"); // Request available PIDs
    }, 60, NULL);
}

void loop()
{
    lv_timer_handler();
    delay(5);
}
