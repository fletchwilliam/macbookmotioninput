# Motion-Controlled MacBook Input Device

A gesture-based input device using Doppler radar technology to control your MacBook with hand motions.

## Overview

This project uses an Arduino Uno R3 with an HB100 Doppler radar module to detect hand gestures and translate them into macOS system actions like mouse clicks, scrolling, and window management.

## Features

- **6 Gesture Types**: Tap, Double Tap, Push, Pull, Wave, Hold
- **macOS Integration**: Native system event generation
- **Visual Feedback**: 8x8 LED matrix shows gesture recognition status
- **Configurable**: Customizable gesture-to-action mappings
- **Menu Bar App**: Easy access to settings and status

## Hardware Requirements

| Component | Model | Purpose |
|-----------|-------|---------|
| Microcontroller | Arduino Uno R3 | Signal processing |
| Motion Sensor | HB100 Doppler Radar | Motion detection |
| Visual Feedback | 8x8 LED Matrix (MAX7219) | Status display |
| Op-Amp | LM358N | Signal amplification |

See [Hardware Setup Guide](docs/HARDWARE_SETUP.md) for detailed wiring instructions.

## Software Components

### Arduino Firmware (`arduino/MotionController/`)

- **SignalAcquisition**: ADC sampling at 2kHz
- **DSP**: Zero-crossing frequency analysis
- **GestureRecognition**: State machine for 6 gestures
- **Display**: LED matrix control
- **SerialComm**: JSON communication protocol

### macOS Host App (`macos/MotionControllerHost/`)

- Serial port monitoring
- Gesture-to-action mapping
- System event generation (CGEvent)
- Menu bar interface

## Installation

### Arduino Firmware

1. Install Arduino IDE
2. Install required libraries:
   - LedControl
3. Open `arduino/MotionController/MotionController.ino`
4. Select Arduino Uno from Tools → Board
5. Upload to Arduino

### macOS Host Application

```bash
cd macos/MotionControllerHost
pip install -r requirements.txt
python menu_bar_app.py
```

**Note**: Requires Accessibility permissions in System Preferences → Security & Privacy.

## Default Gesture Mappings

| Gesture | Action | macOS Equivalent |
|---------|--------|------------------|
| Tap | Left Click | Mouse button 1 |
| Double Tap | Double Click | Double-click |
| Push | Scroll Down | Scroll wheel |
| Pull | Scroll Up | Scroll wheel |
| Wave | Mission Control | Ctrl + Up |
| Hold | App Switcher | Cmd + Tab |

## Serial Protocol

Communication between Arduino and macOS uses JSON at 115200 baud:

```json
{"type":"gesture","gesture":"TAP","confidence":0.95,"timestamp":12345}
```

## Project Structure

```
macbookmotioninput/
├── arduino/
│   └── MotionController/
│       ├── MotionController.ino    # Main sketch
│       ├── Config.h                # Configuration
│       ├── SignalAcquisition.*     # ADC sampling
│       ├── DSP.*                   # Frequency analysis
│       ├── GestureRecognition.*    # Gesture detection
│       ├── Display.*               # LED matrix
│       └── SerialComm.*            # Serial communication
├── macos/
│   └── MotionControllerHost/
│       ├── menu_bar_app.py         # Main application
│       ├── config.py               # Configuration
│       ├── serial_handler.py       # Serial communication
│       ├── event_generator.py      # macOS events
│       └── requirements.txt        # Python dependencies
└── docs/
    └── HARDWARE_SETUP.md           # Hardware assembly guide
```

## Testing

### Arduino Tests
```bash
cd arduino/MotionController/tests
g++ -DTEST_MODE test_gesture_recognition.cpp ../GestureRecognition.cpp -o test_gestures
./test_gestures
```

### Python Tests
```bash
cd macos/MotionControllerHost
pytest tests/ -v
```

## License

MIT License
