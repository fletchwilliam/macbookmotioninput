/**
 * Display.cpp - Implementation of LED Matrix display module
 */

#include "Display.h"

// Icon patterns (8x8 bit patterns, one byte per row)
const byte ICON_ARROW_DOWN[] = {
    B00011000,
    B00011000,
    B00011000,
    B00011000,
    B01011010,
    B00111100,
    B00011000,
    B00000000
};

const byte ICON_ARROW_UP[] = {
    B00000000,
    B00011000,
    B00111100,
    B01011010,
    B00011000,
    B00011000,
    B00011000,
    B00011000
};

const byte ICON_WAVE[] = {
    B00000000,
    B00100010,
    B01010101,
    B10001000,
    B01010101,
    B00100010,
    B00000000,
    B00000000
};

const byte ICON_CIRCLE[] = {
    B00000000,
    B00111100,
    B01000010,
    B01000010,
    B01000010,
    B01000010,
    B00111100,
    B00000000
};

const byte ICON_FILLED_CIRCLE[] = {
    B00000000,
    B00111100,
    B01111110,
    B01111110,
    B01111110,
    B01111110,
    B00111100,
    B00000000
};

const byte ICON_TAP[] = {
    B00011000,
    B00111100,
    B01111110,
    B00011000,
    B00011000,
    B00011000,
    B00011000,
    B00000000
};

const byte ICON_DOUBLE_TAP[] = {
    B00100100,
    B01100110,
    B11100111,
    B00100100,
    B00100100,
    B00100100,
    B00100100,
    B00000000
};

const byte ICON_HOLD[] = {
    B01111110,
    B01000010,
    B01011010,
    B01011010,
    B01011010,
    B01011010,
    B01000010,
    B01111110
};

const byte ICON_ERROR[] = {
    B10000001,
    B01000010,
    B00100100,
    B00011000,
    B00011000,
    B00100100,
    B01000010,
    B10000001
};

Display::Display() {
    _lc = nullptr;
    _displayState = DISPLAY_IDLE;
    _stateStartTime = 0;
    _lastUpdateTime = 0;
    _animationFrame = 0;
    _currentGesture = GESTURE_NONE;
}

void Display::begin() {
    // Initialize LedControl
    // Parameters: DIN, CLK, CS, number of devices
    _lc = new LedControl(PIN_LED_DIN, PIN_LED_CLK, PIN_LED_CS, LED_NUM_DEVICES);

    // Wake up MAX7219
    _lc->shutdown(0, false);

    // Set brightness
    _lc->setIntensity(0, LED_BRIGHTNESS);

    // Clear display
    _lc->clearDisplay(0);

    _displayState = DISPLAY_IDLE;
    _stateStartTime = millis();
    _lastUpdateTime = millis();
}

void Display::update() {
    unsigned long now = millis();

    // Check if gesture feedback should end
    if (_displayState == DISPLAY_GESTURE) {
        if (now - _stateStartTime >= LED_FEEDBACK_DURATION) {
            _displayState = DISPLAY_IDLE;
            _stateStartTime = now;
        }
    }

    // Update animation frame (every 100ms)
    if (now - _lastUpdateTime >= 100) {
        _lastUpdateTime = now;
        _animationFrame++;

        switch (_displayState) {
            case DISPLAY_IDLE:
                animateIdle();
                break;
            case DISPLAY_MOTION:
                animateMotion(50);  // Default amplitude
                break;
            case DISPLAY_GESTURE:
                animateGestureFeedback();
                break;
            case DISPLAY_ERROR:
                // Static error display
                break;
        }
    }
}

void Display::showIdle() {
    _displayState = DISPLAY_IDLE;
    _stateStartTime = millis();
}

void Display::showMotionDetected(int amplitude) {
    _displayState = DISPLAY_MOTION;
    _stateStartTime = millis();
    animateMotion(amplitude);
}

void Display::showGesture(GestureType gesture) {
    _displayState = DISPLAY_GESTURE;
    _currentGesture = gesture;
    _stateStartTime = millis();

    // Draw gesture-specific pattern
    switch (gesture) {
        case GESTURE_PUSH:
            drawPushPattern();
            break;
        case GESTURE_PULL:
            drawPullPattern();
            break;
        case GESTURE_WAVE:
            drawWavePattern();
            break;
        case GESTURE_HOLD:
            drawHoldPattern();
            break;
        case GESTURE_TAP:
            drawTapPattern();
            break;
        case GESTURE_DOUBLE_TAP:
            drawDoubleTapPattern();
            break;
        default:
            clear();
            break;
    }
}

void Display::showError() {
    _displayState = DISPLAY_ERROR;
    _stateStartTime = millis();

    for (int i = 0; i < 8; i++) {
        _lc->setRow(0, i, ICON_ERROR[i]);
    }
}

void Display::clear() {
    _lc->clearDisplay(0);
}

void Display::setBrightness(int level) {
    if (level < 0) level = 0;
    if (level > 15) level = 15;
    _lc->setIntensity(0, level);
}

void Display::setRow(int row, byte pattern) {
    if (row >= 0 && row < 8) {
        _lc->setRow(0, row, pattern);
    }
}

void Display::setLed(int row, int col, bool state) {
    if (row >= 0 && row < 8 && col >= 0 && col < 8) {
        _lc->setLed(0, row, col, state);
    }
}

void Display::animateIdle() {
    // Simple breathing/pulsing dot in center
    clear();

    int frame = _animationFrame % 16;
    int intensity = frame < 8 ? frame : 15 - frame;

    // Temporarily adjust brightness for breathing effect
    _lc->setIntensity(0, intensity);

    // Show small center pattern
    _lc->setLed(0, 3, 3, true);
    _lc->setLed(0, 3, 4, true);
    _lc->setLed(0, 4, 3, true);
    _lc->setLed(0, 4, 4, true);
}

void Display::animateMotion(int amplitude) {
    clear();

    // Map amplitude to number of lit rows (1-8)
    int bars = map(amplitude, 0, 150, 1, 8);
    bars = constrain(bars, 1, 8);

    // Draw vertical bars from bottom
    for (int row = 7; row >= 8 - bars; row--) {
        byte pattern = 0;
        // Center the bar horizontally based on amplitude
        int width = map(amplitude, 0, 150, 2, 8);
        width = constrain(width, 2, 8);
        int start = (8 - width) / 2;
        for (int col = start; col < start + width; col++) {
            pattern |= (1 << (7 - col));
        }
        _lc->setRow(0, row, pattern);
    }
}

void Display::animateGestureFeedback() {
    // Flash the gesture icon
    int frame = (_animationFrame % 4);
    if (frame < 2) {
        // Show icon
        switch (_currentGesture) {
            case GESTURE_PUSH:  drawPushPattern(); break;
            case GESTURE_PULL:  drawPullPattern(); break;
            case GESTURE_WAVE:  drawWavePattern(); break;
            case GESTURE_HOLD:  drawHoldPattern(); break;
            case GESTURE_TAP:   drawTapPattern(); break;
            case GESTURE_DOUBLE_TAP: drawDoubleTapPattern(); break;
            default: break;
        }
    } else {
        // Brief off
        clear();
    }
}

void Display::drawPushPattern() {
    for (int i = 0; i < 8; i++) {
        _lc->setRow(0, i, ICON_ARROW_DOWN[i]);
    }
}

void Display::drawPullPattern() {
    for (int i = 0; i < 8; i++) {
        _lc->setRow(0, i, ICON_ARROW_UP[i]);
    }
}

void Display::drawWavePattern() {
    for (int i = 0; i < 8; i++) {
        _lc->setRow(0, i, ICON_WAVE[i]);
    }
}

void Display::drawHoldPattern() {
    for (int i = 0; i < 8; i++) {
        _lc->setRow(0, i, ICON_HOLD[i]);
    }
}

void Display::drawTapPattern() {
    for (int i = 0; i < 8; i++) {
        _lc->setRow(0, i, ICON_TAP[i]);
    }
}

void Display::drawDoubleTapPattern() {
    for (int i = 0; i < 8; i++) {
        _lc->setRow(0, i, ICON_DOUBLE_TAP[i]);
    }
}

void Display::drawCheckmark() {
    const byte checkmark[] = {
        B00000000,
        B00000001,
        B00000010,
        B00000100,
        B10001000,
        B01010000,
        B00100000,
        B00000000
    };
    for (int i = 0; i < 8; i++) {
        _lc->setRow(0, i, checkmark[i]);
    }
}

void Display::drawArrowDown() {
    for (int i = 0; i < 8; i++) {
        _lc->setRow(0, i, ICON_ARROW_DOWN[i]);
    }
}

void Display::drawArrowUp() {
    for (int i = 0; i < 8; i++) {
        _lc->setRow(0, i, ICON_ARROW_UP[i]);
    }
}

void Display::drawWaveIcon() {
    for (int i = 0; i < 8; i++) {
        _lc->setRow(0, i, ICON_WAVE[i]);
    }
}

void Display::drawCircle() {
    for (int i = 0; i < 8; i++) {
        _lc->setRow(0, i, ICON_CIRCLE[i]);
    }
}

void Display::drawDoubleCircle() {
    for (int i = 0; i < 8; i++) {
        _lc->setRow(0, i, ICON_FILLED_CIRCLE[i]);
    }
}
