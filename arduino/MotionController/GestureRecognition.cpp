/**
 * GestureRecognition.cpp - Implementation of gesture recognition state machine
 */

#include "GestureRecognition.h"

GestureRecognition::GestureRecognition() {
    _state = STATE_IDLE;
    _stateEntryTime = 0;
    _lastGestureTime = 0;
    _motionStartTime = 0;
    _lastTapTime = 0;
    _consecutiveMotionFrames = 0;
    _consecutiveQuietFrames = 0;
    _waveCycleCount = 0;
    _lastDirection = DIRECTION_NONE;
    _initialDirection = DIRECTION_NONE;
    _peakAmplitude = 0;
    _sustainedAmplitudeCount = 0;
}

void GestureRecognition::begin() {
    reset();
}

void GestureRecognition::reset() {
    _state = STATE_IDLE;
    _stateEntryTime = millis();
    _consecutiveMotionFrames = 0;
    _consecutiveQuietFrames = 0;
    _waveCycleCount = 0;
    _lastDirection = DIRECTION_NONE;
    _initialDirection = DIRECTION_NONE;
    _peakAmplitude = 0;
    _sustainedAmplitudeCount = 0;
}

GestureState GestureRecognition::getState() const {
    return _state;
}

const char* GestureRecognition::getStateName() const {
    switch (_state) {
        case STATE_IDLE: return "IDLE";
        case STATE_MOTION_DETECTED: return "MOTION_DETECTED";
        case STATE_TRACKING_PUSH: return "TRACKING_PUSH";
        case STATE_TRACKING_PULL: return "TRACKING_PULL";
        case STATE_TRACKING_WAVE: return "TRACKING_WAVE";
        case STATE_TRACKING_HOLD: return "TRACKING_HOLD";
        case STATE_POTENTIAL_TAP: return "POTENTIAL_TAP";
        case STATE_WAITING_DOUBLE_TAP: return "WAITING_DOUBLE_TAP";
        case STATE_COOLDOWN: return "COOLDOWN";
        default: return "UNKNOWN";
    }
}

const char* GestureRecognition::getGestureName(GestureType type) {
    switch (type) {
        case GESTURE_PUSH: return "PUSH";
        case GESTURE_PULL: return "PULL";
        case GESTURE_WAVE: return "WAVE";
        case GESTURE_HOLD: return "HOLD";
        case GESTURE_TAP: return "TAP";
        case GESTURE_DOUBLE_TAP: return "DOUBLE_TAP";
        default: return "NONE";
    }
}

bool GestureRecognition::isInCooldown() const {
    return _state == STATE_COOLDOWN;
}

void GestureRecognition::transitionTo(GestureState newState) {
    #if DEBUG_GESTURES
        Serial.print(F("State: "));
        Serial.print(getStateName());
        Serial.print(F(" -> "));
    #endif

    _state = newState;
    _stateEntryTime = millis();

    #if DEBUG_GESTURES
        Serial.println(getStateName());
    #endif
}

GestureEvent GestureRecognition::update(const MotionAnalysis& analysis) {
    // Update motion tracking
    if (analysis.motionDetected) {
        _consecutiveMotionFrames++;
        _consecutiveQuietFrames = 0;
        if (analysis.amplitude > _peakAmplitude) {
            _peakAmplitude = analysis.amplitude;
        }
    } else {
        _consecutiveQuietFrames++;
        _consecutiveMotionFrames = 0;
    }

    // Process based on current state
    switch (_state) {
        case STATE_IDLE:
            return processIdleState(analysis);
        case STATE_MOTION_DETECTED:
            return processMotionDetectedState(analysis);
        case STATE_TRACKING_PUSH:
            return processTrackingPushState(analysis);
        case STATE_TRACKING_PULL:
            return processTrackingPullState(analysis);
        case STATE_TRACKING_WAVE:
            return processTrackingWaveState(analysis);
        case STATE_TRACKING_HOLD:
            return processTrackingHoldState(analysis);
        case STATE_POTENTIAL_TAP:
            return processPotentialTapState(analysis);
        case STATE_WAITING_DOUBLE_TAP:
            return processWaitingDoubleTapState(analysis);
        case STATE_COOLDOWN:
            return processCooldownState(analysis);
        default:
            return noGesture();
    }
}

GestureEvent GestureRecognition::processIdleState(const MotionAnalysis& analysis) {
    if (analysis.motionDetected && _consecutiveMotionFrames >= MOTION_START_THRESHOLD) {
        _motionStartTime = millis();
        _initialDirection = analysis.direction;
        _lastDirection = analysis.direction;
        _peakAmplitude = analysis.amplitude;
        _waveCycleCount = 0;
        transitionTo(STATE_MOTION_DETECTED);
    }
    return noGesture();
}

GestureEvent GestureRecognition::processMotionDetectedState(const MotionAnalysis& analysis) {
    unsigned long duration = timeSinceMotionStart();

    // Check for motion end (potential tap)
    if (!analysis.motionDetected && _consecutiveQuietFrames >= MOTION_END_THRESHOLD) {
        if (duration >= TAP_MIN_DURATION && duration <= TAP_MAX_DURATION) {
            transitionTo(STATE_POTENTIAL_TAP);
            return noGesture();
        }
        // Motion too short or too long for tap, reset
        transitionTo(STATE_IDLE);
        return noGesture();
    }

    // Check for direction change (potential wave)
    if (analysis.motionDetected && isDirectionChange(analysis.direction)) {
        _waveCycleCount++;
        _lastDirection = analysis.direction;
    }

    // Determine gesture type based on characteristics
    if (duration > TAP_MAX_DURATION) {
        // Classify based on motion pattern
        if (_waveCycleCount >= WAVE_MIN_CYCLES) {
            transitionTo(STATE_TRACKING_WAVE);
        } else if (analysis.speed == SPEED_SLOW && analysis.amplitude < NOISE_THRESHOLD * 2) {
            transitionTo(STATE_TRACKING_HOLD);
        } else if (_initialDirection == DIRECTION_TOWARD && analysis.speed >= SPEED_MEDIUM) {
            transitionTo(STATE_TRACKING_PUSH);
        } else if (_initialDirection == DIRECTION_AWAY && analysis.speed >= SPEED_MEDIUM) {
            transitionTo(STATE_TRACKING_PULL);
        } else if (duration > HOLD_MIN_DURATION / 2) {
            transitionTo(STATE_TRACKING_HOLD);
        }
    }

    return noGesture();
}

GestureEvent GestureRecognition::processTrackingPushState(const MotionAnalysis& analysis) {
    // Push completes when motion stops
    if (!analysis.motionDetected && _consecutiveQuietFrames >= MOTION_END_THRESHOLD) {
        float confidence = min(1.0f, (float)_peakAmplitude / 80.0f);
        transitionTo(STATE_COOLDOWN);
        return createGestureEvent(GESTURE_PUSH, confidence);
    }

    // Continue tracking...
    return noGesture();
}

GestureEvent GestureRecognition::processTrackingPullState(const MotionAnalysis& analysis) {
    // Pull completes when motion stops
    if (!analysis.motionDetected && _consecutiveQuietFrames >= MOTION_END_THRESHOLD) {
        float confidence = min(1.0f, (float)_peakAmplitude / 80.0f);
        transitionTo(STATE_COOLDOWN);
        return createGestureEvent(GESTURE_PULL, confidence);
    }

    return noGesture();
}

GestureEvent GestureRecognition::processTrackingWaveState(const MotionAnalysis& analysis) {
    // Continue counting wave cycles
    if (analysis.motionDetected && isDirectionChange(analysis.direction)) {
        _waveCycleCount++;
        _lastDirection = analysis.direction;
    }

    // Wave completes when motion stops
    if (!analysis.motionDetected && _consecutiveQuietFrames >= MOTION_END_THRESHOLD) {
        if (_waveCycleCount >= WAVE_MIN_CYCLES) {
            float confidence = min(1.0f, (float)_waveCycleCount / 6.0f);
            transitionTo(STATE_COOLDOWN);
            return createGestureEvent(GESTURE_WAVE, confidence);
        }
        transitionTo(STATE_IDLE);
    }

    return noGesture();
}

GestureEvent GestureRecognition::processTrackingHoldState(const MotionAnalysis& analysis) {
    unsigned long duration = timeSinceMotionStart();

    // Hold completes when sustained for long enough and then released
    if (!analysis.motionDetected && _consecutiveQuietFrames >= MOTION_END_THRESHOLD) {
        if (duration >= HOLD_MIN_DURATION) {
            float confidence = min(1.0f, (float)duration / 1500.0f);
            transitionTo(STATE_COOLDOWN);
            return createGestureEvent(GESTURE_HOLD, confidence);
        }
        // Too short for hold
        transitionTo(STATE_IDLE);
    }

    return noGesture();
}

GestureEvent GestureRecognition::processPotentialTapState(const MotionAnalysis& analysis) {
    // Check if this could be a double tap
    unsigned long timeSinceLastTap = millis() - _lastTapTime;

    if (_lastTapTime > 0 && timeSinceLastTap <= DOUBLE_TAP_WINDOW) {
        // This is a double tap!
        _lastTapTime = 0;
        transitionTo(STATE_COOLDOWN);
        return createGestureEvent(GESTURE_DOUBLE_TAP, 0.9f);
    }

    // Record tap time and wait for potential second tap
    _lastTapTime = millis();
    transitionTo(STATE_WAITING_DOUBLE_TAP);
    return noGesture();
}

GestureEvent GestureRecognition::processWaitingDoubleTapState(const MotionAnalysis& analysis) {
    unsigned long waitTime = millis() - _lastTapTime;

    // Check for new motion (potential second tap)
    if (analysis.motionDetected && _consecutiveMotionFrames >= MOTION_START_THRESHOLD) {
        _motionStartTime = millis();
        _initialDirection = analysis.direction;
        transitionTo(STATE_MOTION_DETECTED);
        return noGesture();
    }

    // Timeout - it was a single tap
    if (waitTime > DOUBLE_TAP_WINDOW) {
        _lastTapTime = 0;
        transitionTo(STATE_COOLDOWN);
        return createGestureEvent(GESTURE_TAP, 0.85f);
    }

    return noGesture();
}

GestureEvent GestureRecognition::processCooldownState(const MotionAnalysis& analysis) {
    if (timeSinceStateEntry() >= GESTURE_COOLDOWN) {
        transitionTo(STATE_IDLE);
    }
    return noGesture();
}

GestureEvent GestureRecognition::createGestureEvent(GestureType type, float confidence) {
    GestureEvent event;
    event.type = type;
    event.confidence = confidence;
    event.timestamp = millis();
    event.isValid = true;
    _lastGestureTime = event.timestamp;

    #if DEBUG_GESTURES
        Serial.print(F("GESTURE: "));
        Serial.print(getGestureName(type));
        Serial.print(F(" ("));
        Serial.print(confidence);
        Serial.println(F(")"));
    #endif

    return event;
}

GestureEvent GestureRecognition::noGesture() {
    GestureEvent event;
    event.type = GESTURE_NONE;
    event.confidence = 0.0f;
    event.timestamp = 0;
    event.isValid = false;
    return event;
}

bool GestureRecognition::isDirectionChange(MotionDirection current) {
    if (_lastDirection == DIRECTION_NONE || current == DIRECTION_NONE) {
        return false;
    }
    return current != _lastDirection;
}

unsigned long GestureRecognition::timeSinceStateEntry() const {
    return millis() - _stateEntryTime;
}

unsigned long GestureRecognition::timeSinceMotionStart() const {
    return millis() - _motionStartTime;
}
