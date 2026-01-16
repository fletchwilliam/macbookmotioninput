/**
 * MotionController.ino - Main sketch for Motion-Controlled MacBook Input Device
 *
 * Hardware: Arduino Uno R3 + HB100 Doppler Radar + MAX7219 LED Matrix
 *
 * This sketch reads Doppler radar signals, processes them to detect gestures,
 * and communicates recognized gestures to the MacBook host via USB serial.
 *
 * Pin Connections:
 *   HB100 (via pre-amp): A0
 *   MAX7219 LED Matrix:  DIN=D11, CS=D10, CLK=D13
 *
 * Serial: 115200 baud, JSON formatted messages
 */

#include "Config.h"
#include "SignalAcquisition.h"
#include "DSP.h"
#include "GestureRecognition.h"
#include "Display.h"
#include "SerialComm.h"

// Module instances
SignalAcquisition signalAcq;
DSP dsp;
GestureRecognition gestureRec;
Display display;
SerialComm serialComm;

// Sample buffer
int sampleBuffer[SAMPLE_BUFFER_SIZE];

// Timing
unsigned long lastProcessTime = 0;
const unsigned long PROCESS_INTERVAL_MS = 50;  // Process every 50ms (20Hz)

// Runtime configuration (can be modified via serial commands)
int sensitivityThreshold = NOISE_THRESHOLD;
bool debugMode = DEBUG_MODE;

void setup() {
    // Initialize serial communication first (for debug output)
    serialComm.begin();

    serialComm.sendDebugMessage("Initializing Motion Controller...");

    // Initialize modules
    signalAcq.begin();
    dsp.begin(signalAcq.getSampleIntervalMicros());
    gestureRec.begin();
    display.begin();

    // Show startup animation
    display.clear();
    for (int i = 0; i < 8; i++) {
        display.setRow(i, 0xFF);
        delay(50);
    }
    for (int i = 0; i < 8; i++) {
        display.setRow(i, 0x00);
        delay(50);
    }

    serialComm.sendDebugMessage("Motion Controller ready");

    // Send ready message
    Serial.println(F("{\"type\":\"ready\",\"version\":\"1.0\"}"));

    lastProcessTime = millis();
}

void loop() {
    unsigned long currentTime = millis();

    // Check for incoming commands from host
    handleSerialCommands();

    // Process at regular intervals
    if (currentTime - lastProcessTime >= PROCESS_INTERVAL_MS) {
        lastProcessTime = currentTime;

        // Collect samples
        int samplesCollected = signalAcq.collectSamples(sampleBuffer, SAMPLE_BUFFER_SIZE);

        #if DEBUG_RAW_SIGNAL
            // Print raw samples for debugging
            for (int i = 0; i < min(10, samplesCollected); i++) {
                Serial.print(sampleBuffer[i]);
                Serial.print(" ");
            }
            Serial.println();
        #endif

        // Remove DC offset for analysis
        signalAcq.removeDCOffset(sampleBuffer, samplesCollected);

        // Analyze signal
        MotionAnalysis analysis = dsp.analyze(sampleBuffer, samplesCollected);

        // Update display based on motion
        if (analysis.motionDetected) {
            display.showMotionDetected(analysis.amplitude);
        }

        // Process gesture recognition
        GestureEvent gesture = gestureRec.update(analysis);

        // Handle recognized gesture
        if (gesture.isValid) {
            // Send gesture event to host
            serialComm.sendGestureEvent(gesture);

            // Show gesture feedback on display
            display.showGesture(gesture.type);
        }

        // Update display animations
        display.update();
    }
}

void handleSerialCommands() {
    ParsedCommand cmd = serialComm.checkForCommand();

    if (!cmd.isValid) return;

    switch (cmd.type) {
        case CMD_PING:
            serialComm.sendPong();
            break;

        case CMD_SET_SENSITIVITY:
            sensitivityThreshold = constrain(cmd.intValue, 5, 200);
            serialComm.sendAck("sensitivity");
            break;

        case CMD_SET_BRIGHTNESS:
            display.setBrightness(constrain(cmd.intValue, 0, 15));
            serialComm.sendAck("brightness");
            break;

        case CMD_SET_DEBUG:
            debugMode = cmd.boolValue;
            serialComm.sendAck("debug");
            break;

        case CMD_CALIBRATE:
            performCalibration();
            break;

        default:
            break;
    }
}

void performCalibration() {
    serialComm.sendDebugMessage("Starting calibration...");

    // Collect samples with no motion to determine noise floor
    display.clear();

    long noiseSum = 0;
    int maxNoise = 0;
    const int CALIBRATION_ROUNDS = 20;

    for (int round = 0; round < CALIBRATION_ROUNDS; round++) {
        signalAcq.collectSamples(sampleBuffer, SAMPLE_BUFFER_SIZE);
        signalAcq.removeDCOffset(sampleBuffer, SAMPLE_BUFFER_SIZE);

        int amplitude = signalAcq.getAmplitude(sampleBuffer, SAMPLE_BUFFER_SIZE);
        noiseSum += amplitude;
        if (amplitude > maxNoise) {
            maxNoise = amplitude;
        }

        // Show progress
        display.setRow(0, 0xFF >> (8 - (round * 8 / CALIBRATION_ROUNDS)));
        delay(100);
    }

    // Set threshold above noise floor
    int avgNoise = noiseSum / CALIBRATION_ROUNDS;
    sensitivityThreshold = max(avgNoise * 2, maxNoise + 5);

    // Send calibration result
    Serial.print(F("{\"type\":\"calibration\",\"noiseFloor\":"));
    Serial.print(avgNoise);
    Serial.print(F(",\"threshold\":"));
    Serial.print(sensitivityThreshold);
    Serial.println(F("}"));

    display.showIdle();
    serialComm.sendAck("calibrate");
}
