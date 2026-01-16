/**
 * Config.h - Configuration parameters for Motion Controller
 *
 * Hardware: Arduino Uno R3 + HB100 Doppler Radar + MAX7219 LED Matrix
 */

#ifndef CONFIG_H
#define CONFIG_H

// ============================================================================
// PIN DEFINITIONS
// ============================================================================

// HB100 Doppler Radar (via pre-amplifier)
#define PIN_DOPPLER_INPUT    A0    // Analog input from pre-amp

// MAX7219 LED Matrix (SPI interface)
#define PIN_LED_DIN          11    // MOSI - Serial data input
#define PIN_LED_CS           10    // SS - Chip select / latch
#define PIN_LED_CLK          13    // SCK - Serial clock

// ============================================================================
// SIGNAL ACQUISITION SETTINGS
// ============================================================================

#define SAMPLE_RATE_HZ       2000  // ADC sample rate (Hz)
#define SAMPLE_BUFFER_SIZE   128   // Samples for frequency analysis
#define ADC_CENTER_VALUE     512   // Expected center value (2.5V bias)
#define NOISE_THRESHOLD      15    // Minimum amplitude to register motion

// ============================================================================
// DOPPLER FREQUENCY SETTINGS
// ============================================================================

// Fd = 19.49 * V (km/h), so for hand gestures (0.5 - 5 km/h):
// Expected frequencies: ~10 Hz to ~100 Hz
#define MIN_DOPPLER_FREQ     5     // Minimum valid Doppler frequency (Hz)
#define MAX_DOPPLER_FREQ     200   // Maximum valid Doppler frequency (Hz)

// Speed thresholds (based on Doppler frequency)
#define SPEED_SLOW_THRESHOLD   20   // Hz - below this is slow
#define SPEED_FAST_THRESHOLD   60   // Hz - above this is fast

// ============================================================================
// GESTURE TIMING PARAMETERS (milliseconds)
// ============================================================================

#define TAP_MIN_DURATION       50   // Minimum tap duration
#define TAP_MAX_DURATION       300  // Maximum tap duration
#define DOUBLE_TAP_WINDOW      500  // Max time between taps for double-tap
#define HOLD_MIN_DURATION      800  // Minimum hold duration
#define WAVE_CYCLE_MIN         150  // Minimum wave cycle duration
#define WAVE_CYCLE_MAX         600  // Maximum wave cycle duration
#define WAVE_MIN_CYCLES        2    // Minimum oscillations for wave gesture
#define GESTURE_COOLDOWN       200  // Cooldown between gestures

// ============================================================================
// MOTION STATE THRESHOLDS
// ============================================================================

#define MOTION_START_THRESHOLD  3   // Consecutive samples to confirm motion start
#define MOTION_END_THRESHOLD    5   // Consecutive quiet samples to end motion
#define DIRECTION_SAMPLES       8   // Samples to determine direction

// ============================================================================
// SERIAL COMMUNICATION
// ============================================================================

#define SERIAL_BAUD_RATE     115200
#define JSON_BUFFER_SIZE     128

// ============================================================================
// LED MATRIX SETTINGS
// ============================================================================

#define LED_NUM_DEVICES      1      // Number of daisy-chained MAX7219
#define LED_BRIGHTNESS       8      // Default brightness (0-15)
#define LED_FEEDBACK_DURATION 500   // Gesture feedback display time (ms)

// ============================================================================
// DEBUG SETTINGS
// ============================================================================

#define DEBUG_MODE           false  // Enable verbose serial output
#define DEBUG_SAMPLE_RATE    false  // Print actual sample rate
#define DEBUG_RAW_SIGNAL     false  // Print raw ADC values
#define DEBUG_FREQUENCY      false  // Print detected frequencies
#define DEBUG_GESTURES       true   // Print gesture detection

#endif // CONFIG_H
