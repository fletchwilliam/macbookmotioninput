/**
 * SignalAcquisition.h - ADC sampling module for Doppler radar signal
 *
 * Reads analog values from the pre-amplified HB100 output at high sample rate.
 */

#ifndef SIGNAL_ACQUISITION_H
#define SIGNAL_ACQUISITION_H

#include <Arduino.h>
#include "Config.h"

class SignalAcquisition {
public:
    SignalAcquisition();

    // Initialize the ADC and timer
    void begin();

    // Read a single sample (blocking)
    int readSample();

    // Fill buffer with samples at configured rate
    // Returns actual number of samples collected
    int collectSamples(int* buffer, int bufferSize);

    // Get the amplitude (peak-to-peak) of collected samples
    int getAmplitude(int* buffer, int bufferSize);

    // Check if motion is present based on signal amplitude
    bool isMotionPresent(int* buffer, int bufferSize);

    // Get DC offset (average value)
    int getDCOffset(int* buffer, int bufferSize);

    // Remove DC offset from buffer (center around zero)
    void removeDCOffset(int* buffer, int bufferSize);

    // Calculate sample interval for timing
    unsigned long getSampleIntervalMicros();

private:
    unsigned long _sampleIntervalMicros;
    unsigned long _lastSampleTime;
    int _noiseThreshold;
};

#endif // SIGNAL_ACQUISITION_H
