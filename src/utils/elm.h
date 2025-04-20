#pragma once

#include <Arduino.h>
#include "debug.h"
#include "screen.h"

uint8_t hexByte(const std::string& byteStr) {
    return static_cast<uint8_t>(std::stoi(byteStr, nullptr, 16));
}

float kpaToCbar(float kpa) {
    return kpa * 0.1f;
}

// Manifold Absolute Pressure
static uint8_t MAP = 0;
static uint8_t BarometricPressure = 0;

void decodeOBD2Response(const std::string& response) {
    if (response.rfind("41", 0) != 0 || response.length() < 4) {
        debug("Not a valid OBD2 response");
        return;
    }

    std::string pid = response.substr(2, 2);
    if (pid == "0C" && response.length() >= 8) {  // RPM
        uint8_t A = hexByte(response.substr(4, 2));
        uint8_t B = hexByte(response.substr(6, 2));
        float rpm = ((A * 256) + B) / 4.0f;
        String msg = "Engine RPM: " + String(rpm) + " rpm";
        debug("%s", msg.c_str());
    } else if (pid == "0D" && response.length() >= 6) {  // Speed
        uint8_t A = hexByte(response.substr(4, 2));
        String msg = "Vehicle Speed: " + String(A) + " km/h";
        debug("%s", msg.c_str());
    } else if (pid == "05" && response.length() >= 6) {  // Coolant Temp
        uint8_t A = hexByte(response.substr(4, 2));
        String msg = "Coolant Temperature: " + String(static_cast<int>(A) - 40) + " °C";
        debug("%s", msg.c_str());
    } else if (pid == "0B" && response.length() >= 6) {  // Intake MAP
        MAP = hexByte(response.substr(4, 2));
        String msg = "Intake MAP: " + String(MAP) + " kPa";
        debug("%s", msg.c_str());
    } else if (pid == "33" && response.length() >= 6) {  // Barometric Pressure
        BarometricPressure = hexByte(response.substr(4, 2));
        String msg = "Barometric Pressure: " + String(BarometricPressure) + " kPa";
        debug("%s", msg.c_str());
    } else {
        debug("Unsupported PID or malformed message");
    }

    // Calculate Manifold Absolute Pressure
    String msg = "Turbo pressure: " + String(kpaToCbar(MAP - BarometricPressure)) + " cbar";
    debug("%s", msg.c_str());

    update_cbar_value(kpaToCbar(MAP - BarometricPressure));
}