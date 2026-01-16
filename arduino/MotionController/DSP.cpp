/**
 * DSP.cpp - Implementation of Digital Signal Processing module
 */

#include "DSP.h"

DSP::DSP() {
    _sampleIntervalMicros = 500;  // Default 2kHz
    _sampleRate = 2000.0f;
}

void DSP::begin(unsigned long sampleIntervalMicros) {
    _sampleIntervalMicros = sampleIntervalMicros;
    _sampleRate = 1000000.0f / (float)sampleIntervalMicros;
}

MotionAnalysis DSP::analyze(int* buffer, int bufferSize) {
    MotionAnalysis result;
    result.motionDetected = false;
    result.direction = DIRECTION_NONE;
    result.speed = SPEED_NONE;
    result.frequency = 0.0f;
    result.amplitude = 0;
    result.zeroCrossings = 0;
    result.confidence = 0.0f;

    // Calculate amplitude (peak-to-peak)
    int minVal = 1023;
    int maxVal = -1023;
    for (int i = 0; i < bufferSize; i++) {
        if (buffer[i] < minVal) minVal = buffer[i];
        if (buffer[i] > maxVal) maxVal = buffer[i];
    }
    result.amplitude = maxVal - minVal;

    // Check if signal is above noise threshold
    if (result.amplitude < NOISE_THRESHOLD) {
        return result;
    }

    result.motionDetected = true;

    // Count zero crossings
    result.zeroCrossings = countZeroCrossings(buffer, bufferSize);

    // Calculate frequency
    result.frequency = calculateFrequency(result.zeroCrossings, bufferSize);

    // Validate frequency is in expected Doppler range
    if (result.frequency < MIN_DOPPLER_FREQ || result.frequency > MAX_DOPPLER_FREQ) {
        // Frequency out of range - likely noise
        result.motionDetected = false;
        result.confidence = 0.2f;
        return result;
    }

    // Detect direction
    result.direction = detectDirection(buffer, bufferSize);

    // Classify speed
    result.speed = classifySpeed(result.frequency);

    // Calculate confidence based on signal quality
    float amplitudeScore = min(1.0f, (float)result.amplitude / 100.0f);
    float freqScore = (result.frequency >= MIN_DOPPLER_FREQ &&
                       result.frequency <= MAX_DOPPLER_FREQ) ? 1.0f : 0.5f;
    result.confidence = amplitudeScore * freqScore;

    #if DEBUG_FREQUENCY
        Serial.print(F("Freq: "));
        Serial.print(result.frequency);
        Serial.print(F(" Hz, Amp: "));
        Serial.print(result.amplitude);
        Serial.print(F(", ZC: "));
        Serial.print(result.zeroCrossings);
        Serial.print(F(", Dir: "));
        Serial.println(result.direction == DIRECTION_TOWARD ? "TOWARD" :
                       result.direction == DIRECTION_AWAY ? "AWAY" : "NONE");
    #endif

    return result;
}

int DSP::countZeroCrossings(int* buffer, int bufferSize) {
    int crossings = 0;
    bool wasPositive = buffer[0] > 0;

    for (int i = 1; i < bufferSize; i++) {
        bool isPositive = buffer[i] > 0;
        if (isPositive != wasPositive) {
            crossings++;
            wasPositive = isPositive;
        }
    }

    return crossings;
}

float DSP::calculateFrequency(int zeroCrossings, int bufferSize) {
    // Each complete cycle has 2 zero crossings
    // Frequency = (crossings / 2) / time
    float bufferDurationSec = (float)bufferSize / _sampleRate;
    float frequency = ((float)zeroCrossings / 2.0f) / bufferDurationSec;
    return frequency;
}

MotionDirection DSP::detectDirection(int* buffer, int bufferSize) {
    // Direction detection based on signal envelope and phase
    // For a simple approach, we look at whether the signal trends
    // positive-first (approaching) or negative-first (receding)

    int direction = findFirstPeakDirection(buffer, bufferSize);

    if (direction > 0) {
        return DIRECTION_TOWARD;
    } else if (direction < 0) {
        return DIRECTION_AWAY;
    }

    return DIRECTION_NONE;
}

int DSP::findFirstPeakDirection(int* buffer, int bufferSize) {
    // Find first significant peak or trough
    int threshold = NOISE_THRESHOLD / 2;
    int positiveSum = 0;
    int negativeSum = 0;
    int samplesChecked = min(bufferSize, DIRECTION_SAMPLES * 4);

    for (int i = 0; i < samplesChecked; i++) {
        if (buffer[i] > threshold) {
            positiveSum += buffer[i];
        } else if (buffer[i] < -threshold) {
            negativeSum += abs(buffer[i]);
        }
    }

    // Determine dominant direction in early samples
    if (positiveSum > negativeSum * 1.2) {
        return 1;   // Toward
    } else if (negativeSum > positiveSum * 1.2) {
        return -1;  // Away
    }

    return 0;  // Indeterminate
}

MotionSpeed DSP::classifySpeed(float frequency) {
    if (frequency < SPEED_SLOW_THRESHOLD) {
        return SPEED_SLOW;
    } else if (frequency > SPEED_FAST_THRESHOLD) {
        return SPEED_FAST;
    }
    return SPEED_MEDIUM;
}

long DSP::getSignalEnergy(int* buffer, int bufferSize) {
    long energy = 0;
    for (int i = 0; i < bufferSize; i++) {
        energy += (long)buffer[i] * buffer[i];
    }
    return energy / bufferSize;
}

void DSP::lowPassFilter(int* buffer, int bufferSize, int windowSize) {
    if (windowSize < 2 || windowSize > bufferSize) return;

    // Simple moving average filter
    int* temp = new int[bufferSize];
    if (!temp) return;  // Memory allocation failed

    for (int i = 0; i < bufferSize; i++) {
        long sum = 0;
        int count = 0;
        int start = max(0, i - windowSize / 2);
        int end = min(bufferSize - 1, i + windowSize / 2);

        for (int j = start; j <= end; j++) {
            sum += buffer[j];
            count++;
        }
        temp[i] = (int)(sum / count);
    }

    // Copy filtered values back
    for (int i = 0; i < bufferSize; i++) {
        buffer[i] = temp[i];
    }

    delete[] temp;
}
