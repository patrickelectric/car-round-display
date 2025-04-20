#pragma once

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEClient.h>

#include "utils/elm.h"

// The remote BLE OBD device address
static BLEAddress obd_address("81:23:45:67:89:ba");

// The correct service and characteristics based on scan
#define OBD_SERVICE_UUID         "0000fff0-0000-1000-8000-00805f9b34fb"
#define OBD_READ_CHAR_UUID       "0000fff1-0000-1000-8000-00805f9b34fb"  // For reading responses
#define OBD_WRITE_CHAR_UUID      "0000fff2-0000-1000-8000-00805f9b34fb"  // For writing commands

// BLE client objects
BLEClient* pClient = nullptr;
BLERemoteCharacteristic* pReadCharacteristic = nullptr;   // For reading responses
BLERemoteCharacteristic* pWriteCharacteristic = nullptr;  // For writing commands
bool deviceConnected = false;

// Callback for BLE notifications from the read characteristic
static void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
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

    decodeOBD2Response(String(pData, length).c_str());
}

// Connect to the OBD device
bool connectToOBD() {
  debug("Connecting to OBD device...");

  // Create a client
  pClient = BLEDevice::createClient();
  debug("- Created client");

  // Connect to the remote device
  if (pClient->connect(obd_address)) {
    debug("- Connected to server");

    // Request optimal connection parameters
    //pClient->updateConnParams(0x0006, 0x0010, 0, 400);

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

// Function to send OBD command
void sendOBDCommand(const char* command) {
  if (!deviceConnected) {
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

// Initialize the OBD adapter with standard AT commands
void initializeOBD() {
  debug("Initializing OBD adapter with AT commands...");

  // Reset
  sendOBDCommand("ATD"); // Reset to defaults
  delay(1000);
  sendOBDCommand("ATZ");
  delay(1000); // Give device time to reset

  // Turn echo off
  sendOBDCommand("ATE0");
  delay(200);

  // Linefeeds off
  sendOBDCommand("ATL0");
  delay(200);

  // Headers off
  //sendOBDCommand("ATH0");
  //delay(200);

  // Spaces off
  sendOBDCommand("ATS0");
  delay(200);

  // Show headers
  sendOBDCommand("ATH1");
  delay(500);

  // Get device identification
  sendOBDCommand("ATI");
  delay(500);

  // Allow long frames
  //sendOBDCommand("ATAL");
  //delay(500);

  // disable auto formatting
  sendOBDCommand("ATCAF0");
  delay(500);

  // Set protocol to auto
  sendOBDCommand("ATSP6");
  delay(2000);

  // Try a simple readiness command
  sendOBDCommand("0100");
  delay(500);
}