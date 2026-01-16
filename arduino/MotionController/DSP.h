/**
 * DSP.h - Digital Signal Processing module for Doppler frequency analysis
 *
 * Uses zero-crossing detection to determine motion frequency and direction.
 * Zero-crossing is preferred over FFT for Arduino Uno due to memory constraints.
 */

#ifndef DSP_H
#define DSP_H

#include <Arduino.h>
#include "Config.h"

// Motion direction enumeration
enum MotionDirection {
    DIRECTION_NONE = 0,
    DIRECTION_TOWARD,    // Positive Doppler (approaching)
    DIRECTION_AWAY       // Negative Doppler (receding)
};

// Motion speed enumeration
enum MotionSpeed {
    SPEED_NONE = 0,
    SPEED_SLOW,
    SPEED_MEDIUM,
    SPEED_FAST
};

// Analysis result structure
struct MotionAnalysis {
    bool motionDetected;
    MotionDirection direction;
    MotionSpeed speed;
    float frequency;          // Detected Doppler frequency (Hz)
    int amplitude;            // Signal amplitude
    int zeroCrossings;        // Number of zero crossings
    float confidence;         // Detection confidence (0.0 - 1.0)
};

class DSP {
public:
    DSP();

    // Initialize DSP module
    void begin(unsigned long sampleRateMicros);

    // Analyze buffer and return motion characteristics
    MotionAnalysis analyze(int* buffer, int bufferSize);

    // Count zero crossings in buffer (assumes DC removed)
    int countZeroCrossings(int* buffer, int bufferSize);

    // Calculate frequency from zero crossings
    float calculateFrequency(int zeroCrossings, int bufferSize);

    // Determine motion direction from signal phase
    MotionDirection detectDirection(int* buffer, int bufferSize);

    // Classify speed based on frequency
    MotionSpeed classifySpeed(float frequency);

    // Get signal energy (sum of squared values)
    long getSignalEnergy(int* buffer, int bufferSize);

    // Simple low-pass filter (moving average)
    void lowPassFilter(int* buffer, int bufferSize, int windowSize);

private:
    unsigned long _sampleIntervalMicros;
    float _sampleRate;

    // Helper to find peaks for direction detection
    int findFirstPeakDirection(int* buffer, int bufferSize);
};

#endif // DSP_H
