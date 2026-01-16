/**
 * test_gesture_recognition.cpp - Unit tests for GestureRecognition module
 *
 * These tests can be run using the ArduinoUnit or AUnit library,
 * or compiled and run on a PC using a test framework like Google Test.
 *
 * To run on PC:
 * 1. Create mock Arduino.h with millis() simulation
 * 2. Compile with: g++ -DTEST_MODE test_gesture_recognition.cpp ../GestureRecognition.cpp -o test_gestures
 * 3. Run: ./test_gestures
 */

#ifdef TEST_MODE

#include <iostream>
#include <cassert>
#include <cstring>

// Mock Arduino environment
unsigned long mock_millis = 0;
unsigned long millis() { return mock_millis; }
void advance_time(unsigned long ms) { mock_millis += ms; }

// Include the module under test
#include "../GestureRecognition.h"
#include "../GestureRecognition.cpp"

// Test helper to create motion analysis
MotionAnalysis createMotion(bool detected, MotionDirection dir, MotionSpeed speed, int amplitude) {
    MotionAnalysis m;
    m.motionDetected = detected;
    m.direction = dir;
    m.speed = speed;
    m.amplitude = amplitude;
    m.frequency = 50.0f;
    m.zeroCrossings = 10;
    m.confidence = 0.8f;
    return m;
}

// Test: Initial state should be IDLE
void test_initial_state() {
    GestureRecognition gr;
    gr.begin();
    assert(gr.getState() == STATE_IDLE);
    std::cout << "✓ test_initial_state passed" << std::endl;
}

// Test: Motion detection should transition from IDLE to MOTION_DETECTED
void test_motion_detected_transition() {
    GestureRecognition gr;
    gr.begin();

    MotionAnalysis motion = createMotion(true, DIRECTION_TOWARD, SPEED_FAST, 50);

    // Send enough frames to trigger motion detection
    for (int i = 0; i < MOTION_START_THRESHOLD + 1; i++) {
        gr.update(motion);
    }

    assert(gr.getState() == STATE_MOTION_DETECTED);
    std::cout << "✓ test_motion_detected_transition passed" << std::endl;
}

// Test: Quick motion followed by stillness should detect TAP
void test_tap_detection() {
    mock_millis = 0;
    GestureRecognition gr;
    gr.begin();

    MotionAnalysis motion = createMotion(true, DIRECTION_TOWARD, SPEED_FAST, 50);
    MotionAnalysis noMotion = createMotion(false, DIRECTION_NONE, SPEED_NONE, 0);

    // Start motion
    for (int i = 0; i < MOTION_START_THRESHOLD + 1; i++) {
        gr.update(motion);
        advance_time(10);
    }

    // Continue motion for TAP duration
    advance_time(100);  // Within TAP_MIN_DURATION to TAP_MAX_DURATION

    // End motion
    for (int i = 0; i < MOTION_END_THRESHOLD + 1; i++) {
        gr.update(noMotion);
        advance_time(10);
    }

    // Should now be waiting for double tap
    assert(gr.getState() == STATE_WAITING_DOUBLE_TAP);

    // Wait for double tap window to expire
    advance_time(DOUBLE_TAP_WINDOW + 100);
    GestureEvent event = gr.update(noMotion);

    assert(event.isValid);
    assert(event.type == GESTURE_TAP);
    std::cout << "✓ test_tap_detection passed" << std::endl;
}

// Test: Two quick taps should detect DOUBLE_TAP
void test_double_tap_detection() {
    mock_millis = 0;
    GestureRecognition gr;
    gr.begin();

    MotionAnalysis motion = createMotion(true, DIRECTION_TOWARD, SPEED_FAST, 50);
    MotionAnalysis noMotion = createMotion(false, DIRECTION_NONE, SPEED_NONE, 0);

    // First tap
    for (int i = 0; i < MOTION_START_THRESHOLD + 1; i++) {
        gr.update(motion);
        advance_time(10);
    }
    advance_time(100);
    for (int i = 0; i < MOTION_END_THRESHOLD + 1; i++) {
        gr.update(noMotion);
        advance_time(10);
    }

    // Brief pause (within double tap window)
    advance_time(200);
    gr.update(noMotion);

    // Second tap
    for (int i = 0; i < MOTION_START_THRESHOLD + 1; i++) {
        gr.update(motion);
        advance_time(10);
    }
    advance_time(100);

    GestureEvent event;
    for (int i = 0; i < MOTION_END_THRESHOLD + 1; i++) {
        event = gr.update(noMotion);
        advance_time(10);
        if (event.isValid) break;
    }

    assert(event.isValid);
    assert(event.type == GESTURE_DOUBLE_TAP);
    std::cout << "✓ test_double_tap_detection passed" << std::endl;
}

// Test: Sustained motion toward sensor should detect PUSH
void test_push_detection() {
    mock_millis = 0;
    GestureRecognition gr;
    gr.begin();

    MotionAnalysis motion = createMotion(true, DIRECTION_TOWARD, SPEED_FAST, 80);
    MotionAnalysis noMotion = createMotion(false, DIRECTION_NONE, SPEED_NONE, 0);

    // Start motion
    for (int i = 0; i < MOTION_START_THRESHOLD + 1; i++) {
        gr.update(motion);
        advance_time(10);
    }

    // Continue motion beyond TAP_MAX_DURATION
    for (int i = 0; i < 50; i++) {
        gr.update(motion);
        advance_time(20);
    }

    // End motion
    GestureEvent event;
    for (int i = 0; i < MOTION_END_THRESHOLD + 10; i++) {
        event = gr.update(noMotion);
        advance_time(10);
        if (event.isValid) break;
    }

    assert(event.isValid);
    assert(event.type == GESTURE_PUSH);
    std::cout << "✓ test_push_detection passed" << std::endl;
}

// Test: Sustained motion away from sensor should detect PULL
void test_pull_detection() {
    mock_millis = 0;
    GestureRecognition gr;
    gr.begin();

    MotionAnalysis motion = createMotion(true, DIRECTION_AWAY, SPEED_FAST, 80);
    MotionAnalysis noMotion = createMotion(false, DIRECTION_NONE, SPEED_NONE, 0);

    // Start motion
    for (int i = 0; i < MOTION_START_THRESHOLD + 1; i++) {
        gr.update(motion);
        advance_time(10);
    }

    // Continue motion
    for (int i = 0; i < 50; i++) {
        gr.update(motion);
        advance_time(20);
    }

    // End motion
    GestureEvent event;
    for (int i = 0; i < MOTION_END_THRESHOLD + 10; i++) {
        event = gr.update(noMotion);
        advance_time(10);
        if (event.isValid) break;
    }

    assert(event.isValid);
    assert(event.type == GESTURE_PULL);
    std::cout << "✓ test_pull_detection passed" << std::endl;
}

// Test: Gesture name strings
void test_gesture_names() {
    assert(strcmp(GestureRecognition::getGestureName(GESTURE_TAP), "TAP") == 0);
    assert(strcmp(GestureRecognition::getGestureName(GESTURE_DOUBLE_TAP), "DOUBLE_TAP") == 0);
    assert(strcmp(GestureRecognition::getGestureName(GESTURE_PUSH), "PUSH") == 0);
    assert(strcmp(GestureRecognition::getGestureName(GESTURE_PULL), "PULL") == 0);
    assert(strcmp(GestureRecognition::getGestureName(GESTURE_WAVE), "WAVE") == 0);
    assert(strcmp(GestureRecognition::getGestureName(GESTURE_HOLD), "HOLD") == 0);
    assert(strcmp(GestureRecognition::getGestureName(GESTURE_NONE), "NONE") == 0);
    std::cout << "✓ test_gesture_names passed" << std::endl;
}

// Test: Reset should return to IDLE state
void test_reset() {
    GestureRecognition gr;
    gr.begin();

    MotionAnalysis motion = createMotion(true, DIRECTION_TOWARD, SPEED_FAST, 50);

    // Get into a non-idle state
    for (int i = 0; i < MOTION_START_THRESHOLD + 1; i++) {
        gr.update(motion);
    }

    assert(gr.getState() != STATE_IDLE);

    // Reset
    gr.reset();

    assert(gr.getState() == STATE_IDLE);
    std::cout << "✓ test_reset passed" << std::endl;
}

int main() {
    std::cout << "\n=== GestureRecognition Unit Tests ===" << std::endl;
    std::cout << std::endl;

    test_initial_state();
    test_motion_detected_transition();
    test_tap_detection();
    test_double_tap_detection();
    test_push_detection();
    test_pull_detection();
    test_gesture_names();
    test_reset();

    std::cout << std::endl;
    std::cout << "All tests passed!" << std::endl;
    return 0;
}

#endif // TEST_MODE
