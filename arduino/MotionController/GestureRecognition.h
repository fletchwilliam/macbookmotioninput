/**
 * GestureRecognition.h - Gesture recognition state machine
 *
 * Detects 6 gesture types based on motion characteristics:
 * - G-001: Push (toward sensor, fast)
 * - G-002: Pull (away from sensor, fast)
 * - G-003: Wave (oscillating motion)
 * - G-004: Hold (sustained presence)
 * - G-005: Tap (quick approach and retreat)
 * - G-006: Double Tap (two taps within 500ms)
 */

#ifndef GESTURE_RECOGNITION_H
#define GESTURE_RECOGNITION_H

#include <Arduino.h>
#include "Config.h"
#include "DSP.h"

// Gesture types (matching FSD specification)
enum GestureType {
    GESTURE_NONE = 0,
    GESTURE_PUSH,        // G-001
    GESTURE_PULL,        // G-002
    GESTURE_WAVE,        // G-003
    GESTURE_HOLD,        // G-004
    GESTURE_TAP,         // G-005
    GESTURE_DOUBLE_TAP   // G-006
};

// State machine states
enum GestureState {
    STATE_IDLE,
    STATE_MOTION_DETECTED,
    STATE_TRACKING_PUSH,
    STATE_TRACKING_PULL,
    STATE_TRACKING_WAVE,
    STATE_TRACKING_HOLD,
    STATE_POTENTIAL_TAP,
    STATE_WAITING_DOUBLE_TAP,
    STATE_COOLDOWN
};

// Gesture event structure
struct GestureEvent {
    GestureType type;
    float confidence;
    unsigned long timestamp;
    bool isValid;
};

class GestureRecognition {
public:
    GestureRecognition();

    // Initialize gesture recognition
    void begin();

    // Process motion analysis and update state machine
    // Returns a gesture event if one is detected
    GestureEvent update(const MotionAnalysis& analysis);

    // Get current state (for debugging)
    GestureState getState() const;

    // Get state name string (for debugging)
    const char* getStateName() const;

    // Get gesture name string
    static const char* getGestureName(GestureType type);

    // Reset state machine
    void reset();

    // Check if in cooldown period
    bool isInCooldown() const;

private:
    GestureState _state;
    unsigned long _stateEntryTime;
    unsigned long _lastGestureTime;
    unsigned long _motionStartTime;
    unsigned long _lastTapTime;

    // Motion tracking
    int _consecutiveMotionFrames;
    int _consecutiveQuietFrames;
    int _waveCycleCount;
    MotionDirection _lastDirection;
    MotionDirection _initialDirection;

    // Amplitude tracking
    int _peakAmplitude;
    int _sustainedAmplitudeCount;

    // Helper methods
    void transitionTo(GestureState newState);
    GestureEvent processIdleState(const MotionAnalysis& analysis);
    GestureEvent processMotionDetectedState(const MotionAnalysis& analysis);
    GestureEvent processTrackingPushState(const MotionAnalysis& analysis);
    GestureEvent processTrackingPullState(const MotionAnalysis& analysis);
    GestureEvent processTrackingWaveState(const MotionAnalysis& analysis);
    GestureEvent processTrackingHoldState(const MotionAnalysis& analysis);
    GestureEvent processPotentialTapState(const MotionAnalysis& analysis);
    GestureEvent processWaitingDoubleTapState(const MotionAnalysis& analysis);
    GestureEvent processCooldownState(const MotionAnalysis& analysis);

    GestureEvent createGestureEvent(GestureType type, float confidence);
    GestureEvent noGesture();

    bool isDirectionChange(MotionDirection current);
    unsigned long timeSinceStateEntry() const;
    unsigned long timeSinceMotionStart() const;
};

#endif // GESTURE_RECOGNITION_H
