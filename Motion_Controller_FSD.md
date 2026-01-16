# Functional Specification Document

## Motion-Controlled MacBook Input Device

| Field   | Value            |
|---------|------------------|
| Version | 1.0              |
| Date    | January 16, 2026 |
| Status  | Draft            |

---

## 1. Introduction

### 1.1 Purpose

This Functional Specification Document (FSD) defines the requirements and behavior for a motion-controlled input device that interfaces with a MacBook. The device uses Doppler radar technology to detect hand/body motion and translate those movements into operating system control functions such as mouse clicks, scrolling, window management, and application control.

### 1.2 Scope

This document covers:

- Hardware component specifications and interfaces
- Motion detection and interpretation algorithms
- Gesture-to-action mapping
- Visual feedback system via LED matrix
- Communication protocol with MacBook host

### 1.3 Target Audience

This document is intended for developers implementing the firmware and software components using Claude Code or similar AI-assisted development tools.

---

## 2. System Overview

### 2.1 System Architecture

The system consists of three primary hardware components connected to an Arduino Uno R3 microcontroller, which communicates with the MacBook via USB serial connection. The HB100 Doppler radar module detects motion, the Arduino processes the signals and determines gestures, and the 8x8 LED matrix provides visual feedback to the user.

### 2.2 Hardware Components

| Component       | Model                    | Purpose                                          |
|-----------------|--------------------------|--------------------------------------------------|
| Microcontroller | Arduino Uno R3           | Signal processing, gesture logic, USB communication |
| Motion Sensor   | HB100 Doppler Radar      | Detect motion via microwave Doppler shift        |
| Visual Feedback | 8x8 LED Matrix (MAX7219) | Display current mode, gesture recognition status |

---

## 3. Hardware Specifications

### 3.1 Arduino Uno R3

- **Microcontroller:** ATmega328P
- **Operating Voltage:** 5V
- **Digital I/O Pins:** 14 (6 provide PWM output)
- **Analog Input Pins:** 6
- **Clock Speed:** 16 MHz
- **USB Connection:** Type-B for programming and serial communication

### 3.2 HB100 Doppler Radar Module

- **Frequency:** 10.525 GHz (X-Band)
- **Operating Voltage:** 4.5V - 5.2V DC
- **Operating Current:** ~40mA
- **Output:** IF (Intermediate Frequency) signal - Doppler shift frequency
- **Output Signal Level:** Microvolts (requires pre-amplification)
- **Doppler Frequency Formula:** Fd = 19.49 × V (where V is velocity in km/h)
- **Detection Range:** Depends on target size and amplifier gain

#### 3.2.1 Required Signal Conditioning

The HB100 IF output produces a signal in the microvolt range, requiring a pre-amplifier circuit to boost the signal to logic levels readable by the Arduino. The recommended circuit includes:

- Operational amplifier (e.g., LM358N) for signal amplification
- Bandpass filter to isolate relevant Doppler frequencies (typically 10Hz - 4kHz)
- Voltage biasing to center the signal around 2.5V for ADC reading

### 3.3 8x8 LED Matrix Module (MAX7219)

- **Display:** 64 LEDs arranged in 8 rows × 8 columns
- **Driver IC:** MAX7219 serial interface LED driver
- **Operating Voltage:** 5V
- **Interface:** 3-wire SPI (DIN, CLK, CS/LOAD)
- **Brightness:** 16 levels (software adjustable)
- **Daisy-chainable:** For expanded displays (future enhancement)

---

## 4. Pin Connections

### 4.1 HB100 to Arduino

| HB100 Pin | Arduino Pin      | Notes                   |
|-----------|------------------|-------------------------|
| VCC       | 5V               | Power supply            |
| GND       | GND              | Ground reference        |
| IF Output | A0 (via pre-amp) | Amplified Doppler signal |

### 4.2 LED Matrix (MAX7219) to Arduino

| MAX7219 Pin | Arduino Pin | Notes              |
|-------------|-------------|--------------------|
| VCC         | 5V          | Power supply       |
| GND         | GND         | Ground reference   |
| DIN         | D11 (MOSI)  | Serial data input  |
| CS/LOAD     | D10 (SS)    | Chip select / latch |
| CLK         | D13 (SCK)   | Serial clock       |

---

## 5. Functional Requirements

### 5.1 Motion Detection (FR-001)

The system SHALL detect motion within the sensor field of view and determine:

- Presence of motion (motion detected vs. no motion)
- Direction of motion (toward sensor vs. away from sensor)
- Relative speed of motion (slow, medium, fast)
- Motion patterns (sustained motion, quick gesture, oscillation)

### 5.2 Gesture Recognition (FR-002)

The system SHALL recognize the following gesture types based on motion characteristics:

| Gesture ID | Description                          | Detection Criteria        |
|------------|--------------------------------------|---------------------------|
| G-001      | Push (hand moves toward sensor)      | Positive Doppler, fast    |
| G-002      | Pull (hand moves away from sensor)   | Negative Doppler, fast    |
| G-003      | Wave (oscillating motion)            | Alternating +/- Doppler   |
| G-004      | Hold (sustained presence near sensor)| Low amplitude, sustained  |
| G-005      | Tap (quick approach and retreat)     | Quick +/- sequence        |
| G-006      | Double Tap (two quick taps)          | Two taps within 500ms     |

### 5.3 OS Action Mapping (FR-003)

The system SHALL map recognized gestures to macOS actions. The default mapping is:

| Gesture          | Action              | macOS Equivalent     |
|------------------|---------------------|----------------------|
| Tap (G-005)      | Left Click          | Mouse button 1       |
| Double Tap (G-006)| Double Click       | Double-click mouse   |
| Push (G-001)     | Scroll Down         | Scroll wheel down    |
| Pull (G-002)     | Scroll Up           | Scroll wheel up      |
| Wave (G-003)     | Mission Control     | Control + Up Arrow   |
| Hold (G-004)     | Application Switcher| Cmd + Tab            |

### 5.4 Visual Feedback (FR-004)

The LED matrix SHALL provide visual feedback for:

- System ready state (idle animation)
- Motion detected (activity indicator)
- Gesture recognized (gesture-specific icon for 500ms)
- Current operating mode (mode indicator in corner)
- Error conditions (error pattern)

### 5.5 Communication Protocol (FR-005)

The Arduino SHALL communicate with the MacBook host via USB serial connection:

- **Baud rate:** 115200
- **Data format:** 8N1 (8 data bits, no parity, 1 stop bit)
- **Message format:** JSON-encoded gesture events

Example message:
```json
{"gesture":"TAP","confidence":0.95,"timestamp":12345}
```

---

## 6. Software Architecture

### 6.1 Arduino Firmware Modules

1. **Signal Acquisition Module:** Reads analog values from pre-amp output at configurable sample rate
2. **DSP Module:** Performs frequency analysis using FFT or zero-crossing detection
3. **Gesture Recognition Module:** State machine for gesture detection and classification
4. **Display Module:** Controls LED matrix via LedControl or MD_MAX72xx library
5. **Serial Communication Module:** Formats and transmits gesture events to host

### 6.2 MacBook Host Application

A companion application running on macOS SHALL:

- Monitor USB serial port for incoming gesture events
- Parse JSON messages and validate gesture data
- Generate corresponding system events using CGEvent or AppleScript
- Provide configuration UI for gesture-to-action mapping
- Run as a background daemon or menu bar application

---

## 7. Performance Requirements

| Requirement                   | Specification                      |
|-------------------------------|-----------------------------------|
| Gesture recognition latency   | < 100ms from gesture completion   |
| End-to-end action latency     | < 200ms from gesture to OS action |
| Gesture recognition accuracy  | > 90% under normal conditions     |
| Sample rate                   | > 1000 Hz for ADC readings        |
| False positive rate           | < 5% unintended gesture triggers  |
| Detection range               | 10cm - 100cm from sensor          |

---

## 8. Configuration Parameters

The following parameters SHALL be configurable via the host application:

- **Sensitivity threshold:** Minimum signal amplitude to register motion
- **Gesture timing windows:** Duration thresholds for tap, hold, wave detection
- **Gesture-to-action mapping:** Customizable action assignments
- **LED brightness:** Display intensity (0-15)
- **Debug mode:** Enable verbose serial output for diagnostics

---

## 9. Dependencies and Libraries

### 9.1 Arduino Libraries

- **LedControl** or **MD_MAX72xx:** LED matrix driver
- **FreqMeasure** or **AnalogFrequency:** Doppler frequency measurement
- **ArduinoJson:** JSON message formatting

### 9.2 macOS Requirements

- macOS 10.15 (Catalina) or later
- Python 3.8+ with pyserial (or native Swift/Objective-C implementation)
- Accessibility permissions for generating system events

---

## 10. Testing Requirements

- Unit tests for each firmware module
- Integration tests for gesture detection pipeline
- End-to-end tests for gesture-to-action flow
- User acceptance testing with defined gesture scenarios
- Performance benchmarks for latency and accuracy

---

## 11. Future Enhancements

- Multi-sensor array for directional gesture detection (left/right swipes)
- Machine learning-based gesture recognition for improved accuracy
- Bluetooth connectivity for wireless operation
- Expanded LED matrix display (daisy-chained modules)
- Custom gesture recording and training interface

---

## Appendix A: Revision History

| Version | Date         | Author | Changes       |
|---------|--------------|--------|---------------|
| 1.0     | Jan 16, 2026 | -      | Initial draft |
