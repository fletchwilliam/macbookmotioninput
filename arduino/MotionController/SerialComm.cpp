/**
 * SerialComm.cpp - Implementation of serial communication module
 */

#include "SerialComm.h"

SerialComm::SerialComm() {
    _inputIndex = 0;
    _isConnected = false;
    memset(_inputBuffer, 0, JSON_BUFFER_SIZE);
}

void SerialComm::begin() {
    Serial.begin(SERIAL_BAUD_RATE);

    // Wait for serial connection (with timeout)
    unsigned long startTime = millis();
    while (!Serial && (millis() - startTime < 3000)) {
        ; // Wait up to 3 seconds
    }

    _isConnected = (bool)Serial;
}

bool SerialComm::isReady() {
    return (bool)Serial;
}

void SerialComm::sendGestureEvent(const GestureEvent& event) {
    if (!event.isValid) return;

    Serial.print(F("{\"type\":\"gesture\",\"gesture\":\""));
    Serial.print(GestureRecognition::getGestureName(event.type));
    Serial.print(F("\",\"confidence\":"));
    Serial.print(event.confidence, 2);
    Serial.print(F(",\"timestamp\":"));
    Serial.print(event.timestamp);
    Serial.println(F("}"));
}

void SerialComm::sendDebugMessage(const char* message) {
    #if DEBUG_MODE
        Serial.print(F("{\"type\":\"debug\",\"message\":\""));
        Serial.print(message);
        Serial.println(F("\"}"));
    #endif
}

void SerialComm::sendMotionData(float frequency, int amplitude, int direction) {
    Serial.print(F("{\"type\":\"motion\",\"frequency\":"));
    Serial.print(frequency, 1);
    Serial.print(F(",\"amplitude\":"));
    Serial.print(amplitude);
    Serial.print(F(",\"direction\":"));
    Serial.print(direction);
    Serial.println(F("}"));
}

void SerialComm::sendAck(const char* command) {
    Serial.print(F("{\"type\":\"ack\",\"command\":\""));
    Serial.print(command);
    Serial.println(F("\"}"));
}

void SerialComm::sendError(const char* message) {
    Serial.print(F("{\"type\":\"error\",\"message\":\""));
    Serial.print(message);
    Serial.println(F("\"}"));
}

void SerialComm::sendPong() {
    Serial.println(F("{\"type\":\"pong\"}"));
}

ParsedCommand SerialComm::checkForCommand() {
    ParsedCommand result;
    result.type = CMD_NONE;
    result.intValue = 0;
    result.boolValue = false;
    result.isValid = false;

    while (Serial.available() > 0) {
        char c = Serial.read();

        if (c == '\n' || c == '\r') {
            if (_inputIndex > 0) {
                _inputBuffer[_inputIndex] = '\0';
                result = parseCommand(_inputBuffer);
                _inputIndex = 0;
                memset(_inputBuffer, 0, JSON_BUFFER_SIZE);
                return result;
            }
        } else if (_inputIndex < JSON_BUFFER_SIZE - 1) {
            _inputBuffer[_inputIndex++] = c;
        }
    }

    return result;
}

ParsedCommand SerialComm::parseCommand(const char* json) {
    ParsedCommand result;
    result.type = CMD_NONE;
    result.intValue = 0;
    result.boolValue = false;
    result.isValid = false;

    // Simple JSON parsing (avoiding ArduinoJson for memory efficiency)
    // Expected formats:
    // {"cmd":"ping"}
    // {"cmd":"sensitivity","value":50}
    // {"cmd":"brightness","value":8}
    // {"cmd":"debug","value":true}
    // {"cmd":"calibrate"}

    // Find command type
    const char* cmdStart = strstr(json, "\"cmd\":\"");
    if (!cmdStart) return result;

    cmdStart += 7;  // Skip past "cmd":"

    if (strncmp(cmdStart, "ping", 4) == 0) {
        result.type = CMD_PING;
        result.isValid = true;
    }
    else if (strncmp(cmdStart, "sensitivity", 11) == 0) {
        result.type = CMD_SET_SENSITIVITY;
        // Find value
        const char* valStart = strstr(json, "\"value\":");
        if (valStart) {
            valStart += 8;
            result.intValue = atoi(valStart);
            result.isValid = true;
        }
    }
    else if (strncmp(cmdStart, "brightness", 10) == 0) {
        result.type = CMD_SET_BRIGHTNESS;
        const char* valStart = strstr(json, "\"value\":");
        if (valStart) {
            valStart += 8;
            result.intValue = atoi(valStart);
            result.isValid = true;
        }
    }
    else if (strncmp(cmdStart, "debug", 5) == 0) {
        result.type = CMD_SET_DEBUG;
        const char* valStart = strstr(json, "\"value\":");
        if (valStart) {
            valStart += 8;
            result.boolValue = (strncmp(valStart, "true", 4) == 0);
            result.isValid = true;
        }
    }
    else if (strncmp(cmdStart, "calibrate", 9) == 0) {
        result.type = CMD_CALIBRATE;
        result.isValid = true;
    }

    return result;
}

// JSON building helpers (for more complex messages if needed)
void SerialComm::beginJson() {
    Serial.print(F("{"));
}

void SerialComm::addJsonString(const char* key, const char* value, bool isLast) {
    Serial.print(F("\""));
    Serial.print(key);
    Serial.print(F("\":\""));
    Serial.print(value);
    Serial.print(F("\""));
    if (!isLast) Serial.print(F(","));
}

void SerialComm::addJsonInt(const char* key, int value, bool isLast) {
    Serial.print(F("\""));
    Serial.print(key);
    Serial.print(F("\":"));
    Serial.print(value);
    if (!isLast) Serial.print(F(","));
}

void SerialComm::addJsonFloat(const char* key, float value, bool isLast) {
    Serial.print(F("\""));
    Serial.print(key);
    Serial.print(F("\":"));
    Serial.print(value, 2);
    if (!isLast) Serial.print(F(","));
}

void SerialComm::addJsonBool(const char* key, bool value, bool isLast) {
    Serial.print(F("\""));
    Serial.print(key);
    Serial.print(F("\":"));
    Serial.print(value ? F("true") : F("false"));
    if (!isLast) Serial.print(F(","));
}

void SerialComm::endJson() {
    Serial.println(F("}"));
}
