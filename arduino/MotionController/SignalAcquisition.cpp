/**
 * SignalAcquisition.cpp - Implementation of ADC sampling module
 */

#include "SignalAcquisition.h"

SignalAcquisition::SignalAcquisition() {
    _sampleIntervalMicros = 1000000UL / SAMPLE_RATE_HZ;
    _lastSampleTime = 0;
    _noiseThreshold = NOISE_THRESHOLD;
}

void SignalAcquisition::begin() {
    // Configure ADC pin
    pinMode(PIN_DOPPLER_INPUT, INPUT);

    // Configure ADC for faster reading (reduce prescaler)
    // Default prescaler is 128, giving ~9600 Hz max sample rate
    // Prescaler 16 gives ~76800 Hz max sample rate (less accurate but faster)
    #if SAMPLE_RATE_HZ > 5000
        // Set ADC prescaler to 16 for faster sampling
        ADCSRA = (ADCSRA & 0xF8) | 0x04;  // Prescaler = 16
    #endif

    _lastSampleTime = micros();
}

int SignalAcquisition::readSample() {
    return analogRead(PIN_DOPPLER_INPUT);
}

int SignalAcquisition::collectSamples(int* buffer, int bufferSize) {
    unsigned long startTime = micros();
    unsigned long targetInterval = _sampleIntervalMicros;

    for (int i = 0; i < bufferSize; i++) {
        // Wait for next sample time
        while (micros() - _lastSampleTime < targetInterval) {
            // Busy wait for precise timing
        }
        _lastSampleTime = micros();

        // Read ADC value
        buffer[i] = analogRead(PIN_DOPPLER_INPUT);
    }

    #if DEBUG_SAMPLE_RATE
        unsigned long elapsed = micros() - startTime;
        float actualRate = (float)bufferSize * 1000000.0f / (float)elapsed;
        Serial.print(F("Actual sample rate: "));
        Serial.print(actualRate);
        Serial.println(F(" Hz"));
    #endif

    return bufferSize;
}

int SignalAcquisition::getAmplitude(int* buffer, int bufferSize) {
    int minVal = 1023;
    int maxVal = 0;

    for (int i = 0; i < bufferSize; i++) {
        if (buffer[i] < minVal) minVal = buffer[i];
        if (buffer[i] > maxVal) maxVal = buffer[i];
    }

    return maxVal - minVal;
}

bool SignalAcquisition::isMotionPresent(int* buffer, int bufferSize) {
    int amplitude = getAmplitude(buffer, bufferSize);
    return amplitude > _noiseThreshold;
}

int SignalAcquisition::getDCOffset(int* buffer, int bufferSize) {
    long sum = 0;
    for (int i = 0; i < bufferSize; i++) {
        sum += buffer[i];
    }
    return (int)(sum / bufferSize);
}

void SignalAcquisition::removeDCOffset(int* buffer, int bufferSize) {
    int dcOffset = getDCOffset(buffer, bufferSize);
    for (int i = 0; i < bufferSize; i++) {
        buffer[i] -= dcOffset;
    }
}

unsigned long SignalAcquisition::getSampleIntervalMicros() {
    return _sampleIntervalMicros;
}
