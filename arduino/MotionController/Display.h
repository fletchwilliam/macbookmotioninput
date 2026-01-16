/**
 * Display.h - LED Matrix display module using MAX7219
 *
 * Provides visual feedback for system states and gesture recognition.
 */

#ifndef DISPLAY_H
#define DISPLAY_H

#include <Arduino.h>
#include "Config.h"
#include "GestureRecognition.h"

// Forward declaration for LedControl
// Note: LedControl library must be installed
#include <LedControl.h>

// Display animation states
enum DisplayState {
    DISPLAY_IDLE,
    DISPLAY_MOTION,
    DISPLAY_GESTURE,
    DISPLAY_ERROR
};

class Display {
public:
    Display();

    // Initialize the LED matrix
    void begin();

    // Update display (call regularly in main loop)
    void update();

    // Show idle animation
    void showIdle();

    // Show motion detected indicator
    void showMotionDetected(int amplitude);

    // Show gesture feedback
    void showGesture(GestureType gesture);

    // Show error pattern
    void showError();

    // Clear display
    void clear();

    // Set brightness (0-15)
    void setBrightness(int level);

    // Direct row access for custom patterns
    void setRow(int row, byte pattern);

    // Set single LED
    void setLed(int row, int col, bool state);

private:
    LedControl* _lc;
    DisplayState _displayState;
    unsigned long _stateStartTime;
    unsigned long _lastUpdateTime;
    int _animationFrame;
    GestureType _currentGesture;

    // Animation helpers
    void animateIdle();
    void animateMotion(int amplitude);
    void animateGestureFeedback();

    // Gesture-specific patterns
    void drawPushPattern();
    void drawPullPattern();
    void drawWavePattern();
    void drawHoldPattern();
    void drawTapPattern();
    void drawDoubleTapPattern();

    // Utility patterns
    void drawCheckmark();
    void drawArrowDown();
    void drawArrowUp();
    void drawWaveIcon();
    void drawCircle();
    void drawDoubleCircle();
};

#endif // DISPLAY_H
