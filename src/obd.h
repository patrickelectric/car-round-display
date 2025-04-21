#pragma once

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEClient.h>

class OBD {
public:
    static OBD& instance() {
        static OBD instance;
        return instance;
    }

    bool connect();
    void initialize();
    void sendCommand(const char* command);
    bool isConnected() const { return deviceConnected; }

    // Delete copy constructor and assignment operator
    OBD(const OBD&) = delete;
    OBD& operator=(const OBD&) = delete;

private:
    OBD() = default;  // Private constructor
    ~OBD() = default;

    // Constants
    const char* OBD_SERVICE_UUID = "0000fff0-0000-1000-8000-00805f9b34fb";
    const char* OBD_READ_CHAR_UUID = "0000fff1-0000-1000-8000-00805f9b34fb";
    const char* OBD_WRITE_CHAR_UUID = "0000fff2-0000-1000-8000-00805f9b34fb";
    const BLEAddress OBD_ADDRESS = BLEAddress("81:23:45:67:89:ba");

    // Member variables
    BLEClient* pClient = nullptr;
    BLERemoteCharacteristic* pReadCharacteristic = nullptr;
    BLERemoteCharacteristic* pWriteCharacteristic = nullptr;
    bool deviceConnected = false;

    // Friend function for notification callback
    friend void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic,
                             uint8_t* pData,
                             size_t length,
                             bool isNotify);
};