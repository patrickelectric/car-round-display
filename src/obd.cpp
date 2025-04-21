#include "obd.h"
#include "screen.h"
#include "utils/debug.h"
#include "utils/elm.h"

void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic,
                   uint8_t* pData,
                   size_t length,
                   bool isNotify) {
    Serial.print("Notification received: ");

    // Print raw bytes as hex
    for (int i = 0; i < length; i++) {
        Serial.printf("%02X ", pData[i]);
    }
    Serial.println();

    // Also try to print as ASCII if possible
    Serial.print("As text: ");
    for (int i = 0; i < length; i++) {
        if (pData[i] >= 32 && pData[i] <= 126) {
            Serial.print((char)pData[i]);
        } else {
            Serial.printf("\\x%02X", pData[i]);
        }
    }
    Serial.println();

    auto decoded = decodeOBD2Response(String(pData, length).c_str());
    if (!decoded) {
        return;
    }

    debug("Decoded: %s", decoded->msg.c_str());

    static uint8_t MAP = 0;
    static uint8_t BarometricPressure = 0;
    switch (decoded->pid) {
        case OBD2PIDs::MAP:
            MAP = decoded->value.value();
            break;
        case OBD2PIDs::BarometricPressure:
            BarometricPressure = decoded->value.value();
            break;
    }

    Screen::instance().updateValue(kpaToCbar(MAP - BarometricPressure));
}

bool OBD::connect() {
    if (pClient && pClient->isConnected()) {
        deviceConnected = true;
        return true;
    }
    deviceConnected = false;

    debug("Connecting to OBD device...");

    // Create a client
    pClient = BLEDevice::createClient();
    debug("- Created client");

    // Connect to the remote device
    if (pClient->connect(OBD_ADDRESS)) {
        debug("- Connected to server");

        // Request a larger MTU size
        pClient->setMTU(517);
        debug("- Requested larger MTU and updated connection parameters");
    } else {
        debug("- Connection failed");
        return false;
    }

    // Get the OBD service
    BLERemoteService* pRemoteService = pClient->getService(BLEUUID(OBD_SERVICE_UUID));
    if (pRemoteService == nullptr) {
        debug("- Failed to find OBD service UUID");
        pClient->disconnect();
        return false;
    }
    debug("- Found OBD service");

    // Get the read characteristic
    pReadCharacteristic = pRemoteService->getCharacteristic(BLEUUID(OBD_READ_CHAR_UUID));
    if (pReadCharacteristic == nullptr) {
        debug("- Failed to find read characteristic UUID");
        pClient->disconnect();
        return false;
    }
    debug("- Found read characteristic");

    // Get the write characteristic
    pWriteCharacteristic = pRemoteService->getCharacteristic(BLEUUID(OBD_WRITE_CHAR_UUID));
    if (pWriteCharacteristic == nullptr) {
        debug("- Failed to find write characteristic UUID");
        pClient->disconnect();
        return false;
    }
    debug("- Found write characteristic");

    // Register for notifications from the read characteristic
    if (pReadCharacteristic->canNotify()) {
        pReadCharacteristic->registerForNotify(notifyCallback);
        debug("- Registered for notifications from read characteristic");
    } else {
        debug("- Warning: Read characteristic does not support notifications");
    }

    deviceConnected = true;
    return true;
}

void OBD::sendCommand(const char* command) {
    if (!pClient || !pClient->isConnected()) {
        deviceConnected = false;
        debug("Cannot send command - not connected");
        return;
    }

    if (pWriteCharacteristic == nullptr) {
        debug("Cannot send command - write characteristic unavailable");
        return;
    }

    // Create a buffer with the command plus CR
    size_t cmdLen = strlen(command);
    uint8_t* cmdBuffer = new uint8_t[cmdLen + 1];
    memcpy(cmdBuffer, command, cmdLen);

    // Add CR at the end if it's not already there
    if (command[cmdLen-1] != '\r') {
        cmdBuffer[cmdLen] = '\r';
        cmdLen++;
    } else {
        cmdLen = strlen(command);
    }

    // Print the command being sent
    Serial.print("Sending command: ");
    for (size_t i = 0; i < cmdLen; i++) {
        if (cmdBuffer[i] >= 32 && cmdBuffer[i] <= 126) {
            Serial.print((char)cmdBuffer[i]);
        } else {
            Serial.printf("\\x%02X", cmdBuffer[i]);
        }
    }
    Serial.println();

    // Send with WRITE_NO_RESPONSE for better performance if available
    if (pWriteCharacteristic->canWriteNoResponse()) {
        pWriteCharacteristic->writeValue(cmdBuffer, cmdLen, false);
    } else {
        pWriteCharacteristic->writeValue(cmdBuffer, cmdLen);
    }

    delete[] cmdBuffer;
}

void OBD::initialize() {
    debug("Initializing OBD adapter with AT commands...");

    // Reset
    sendCommand("ATD"); // Reset to defaults
    delay(1000);
    sendCommand("ATZ");
    delay(1000); // Give device time to reset

    // Turn echo off
    sendCommand("ATE0");
    delay(200);

    // Linefeeds off
    sendCommand("ATL0");
    delay(200);

    // Spaces off
    sendCommand("ATS0");
    delay(200);

    // Show headers
    sendCommand("ATH1");
    delay(500);

    // Get device identification
    sendCommand("ATI");
    delay(500);

    // disable auto formatting
    sendCommand("ATCAF0");
    delay(500);

    // Set protocol to auto
    sendCommand("ATSP6");
    delay(2000);

    // Try a simple readiness command
    sendCommand("0100");
    delay(500);
}