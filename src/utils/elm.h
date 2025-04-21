#pragma once

#include <Arduino.h>
#include "debug.h"

uint8_t hexByte(const std::string& byteStr) {
    return static_cast<uint8_t>(std::stoi(byteStr, nullptr, 16));
}

float kpaToCbar(float kpa) {
    return kpa * 0.1f;
}

enum OBD2PIDs {
    CoolantTemp = 0x05,
    MAP = 0x0B, // Manifold Absolute Pressure
    RPM = 0x0C,
    Speed = 0x0D,
    BarometricPressure = 0x33,
};

struct OBD2Response {
    OBD2PIDs pid;
    std::optional<float> value;
    String msg;
};

std::optional<OBD2Response> decodeOBD2Response(const std::string& response) {
    if (response.rfind("41", 0) != 0 || response.length() < 4) {
        debug("Not a valid OBD2 response");
        return std::nullopt;
    }

    // TODO: Sort this by PID
    std::string pid = response.substr(2, 2);
    if (pid == "0C" && response.length() >= 8) {  // RPM
        uint8_t A = hexByte(response.substr(4, 2));
        uint8_t B = hexByte(response.substr(6, 2));
        float rpm = ((A * 256) + B) / 4.0f;
        String msg = "Engine RPM: " + String(rpm) + " rpm";
        return OBD2Response{OBD2PIDs::RPM, rpm, msg};
    } else if (pid == "0D" && response.length() >= 6) {  // Speed
        uint8_t A = hexByte(response.substr(4, 2));
        String msg = "Vehicle Speed: " + String(A) + " km/h";
        return OBD2Response{OBD2PIDs::Speed, A, msg};
    } else if (pid == "05" && response.length() >= 6) {  // Coolant Temp
        uint8_t A = hexByte(response.substr(4, 2));
        String msg = "Coolant Temperature: " + String(static_cast<int>(A) - 40) + " °C";
        return OBD2Response{OBD2PIDs::CoolantTemp, A, msg};
    } else if (pid == "0B" && response.length() >= 6) {  // Intake MAP
        uint8_t MAP = hexByte(response.substr(4, 2));
        String msg = "Intake MAP: " + String(MAP) + " kPa";
        return OBD2Response{OBD2PIDs::MAP, MAP, msg};
    } else if (pid == "33" && response.length() >= 6) {  // Barometric Pressure
        uint8_t BarometricPressure = hexByte(response.substr(4, 2));
        String msg = "Barometric Pressure: " + String(BarometricPressure) + " kPa";
        return OBD2Response{OBD2PIDs::BarometricPressure, BarometricPressure, msg};
    } else {
        debug("Unsupported PID or malformed message");
    }

    return std::nullopt;
}