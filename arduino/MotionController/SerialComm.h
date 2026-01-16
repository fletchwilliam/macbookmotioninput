/**
 * SerialComm.h - Serial communication module for gesture events
 *
 * Formats and transmits gesture events as JSON to the MacBook host.
 */

#ifndef SERIAL_COMM_H
#define SERIAL_COMM_H

#include <Arduino.h>
#include "Config.h"
#include "GestureRecognition.h"

// Command types received from host
enum HostCommand {
    CMD_NONE = 0,
    CMD_SET_SENSITIVITY,
    CMD_SET_BRIGHTNESS,
    CMD_SET_DEBUG,
    CMD_CALIBRATE,
    CMD_PING
};

// Host command structure
struct ParsedCommand {
    HostCommand type;
    int intValue;
    bool boolValue;
    bool isValid;
};

class SerialComm {
public:
    SerialComm();

    // Initialize serial communication
    void begin();

    // Send gesture event as JSON
    void sendGestureEvent(const GestureEvent& event);

    // Send debug/status message
    void sendDebugMessage(const char* message);

    // Send motion data (for calibration/debug)
    void sendMotionData(float frequency, int amplitude, int direction);

    // Send acknowledgment
    void sendAck(const char* command);

    // Send error message
    void sendError(const char* message);

    // Send ping response
    void sendPong();

    // Check for and parse incoming commands
    ParsedCommand checkForCommand();

    // Check if serial is connected and ready
    bool isReady();

private:
    char _inputBuffer[JSON_BUFFER_SIZE];
    int _inputIndex;
    bool _isConnected;

    // Parse JSON command from buffer
    ParsedCommand parseCommand(const char* json);

    // JSON building helpers
    void beginJson();
    void addJsonString(const char* key, const char* value, bool isLast = false);
    void addJsonInt(const char* key, int value, bool isLast = false);
    void addJsonFloat(const char* key, float value, bool isLast = false);
    void addJsonBool(const char* key, bool value, bool isLast = false);
    void endJson();
};

#endif // SERIAL_COMM_H
