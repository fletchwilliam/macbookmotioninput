# Hardware Setup Guide

## Motion Controller Hardware Assembly

This guide covers the hardware assembly for the Motion-Controlled MacBook Input Device.

**See also:** [Visual Assembly Guide](HARDWARE_ASSEMBLY_VISUAL.md) for detailed breadboard diagrams using the Jaycar PB8820 (400 tie points).

---

## Components Required

| Component | Model | Quantity | Notes |
|-----------|-------|----------|-------|
| Microcontroller | Arduino Uno R3 | 1 | ATmega328P based |
| Motion Sensor | HB100 Doppler Radar | 1 | 10.525 GHz X-Band |
| LED Matrix | 8x8 MAX7219 Module | 1 | Red or other color |
| Op-Amp | LM358N | 1 | Dual op-amp IC |
| Resistors | Various | See circuit | 1/4W, 5% tolerance |
| Capacitors | Various | See circuit | Ceramic & Electrolytic |
| Breadboard | Full-size | 1 | For prototyping |
| Jumper Wires | M-M, M-F | Assorted | For connections |
| USB Cable | Type-B | 1 | For Arduino |

---

## Pin Connections

### HB100 Doppler Radar → Pre-Amplifier → Arduino

```
HB100 Module          Pre-Amp Circuit         Arduino Uno
┌─────────────┐      ┌─────────────────┐      ┌──────────┐
│         VCC ├──────┤ +5V             │      │          │
│         GND ├──────┤ GND             │      │          │
│   IF Output ├──────┤ Input    Output ├──────┤ A0       │
└─────────────┘      │             +5V ├──────┤ 5V       │
                     │             GND ├──────┤ GND      │
                     └─────────────────┘      └──────────┘
```

### MAX7219 LED Matrix → Arduino

```
MAX7219 Module        Arduino Uno
┌─────────────┐      ┌──────────┐
│         VCC ├──────┤ 5V       │
│         GND ├──────┤ GND      │
│         DIN ├──────┤ D11      │  (MOSI)
│      CS/LOAD├──────┤ D10      │  (SS)
│         CLK ├──────┤ D13      │  (SCK)
└─────────────┘      └──────────┘
```

---

## Pre-Amplifier Circuit

The HB100 Doppler radar outputs a very weak signal (microvolts) that requires amplification before the Arduino can read it.

### Circuit Schematic

```
                           +5V
                            │
                           ┌┴┐
                           │ │ R1
                           │ │ 10kΩ
                           └┬┘
                            │
                            ├────────────────────┐
                            │                    │
             C1             │    ┌───────┐       │
HB100 IF ────┤├─────┬───────┴────┤-      │       │
           100nF    │            │  U1A  ├───┬───┼─────────► To Arduino A0
                   ┌┴┐           │ LM358 │   │   │
                   │ │ R2        │       │   │  ┌┴┐
                   │ │ 100kΩ     │+      │   │  │ │ R4
                   └┬┘       ┌───┤       │   │  │ │ 10kΩ
                    │        │   └───────┘   │  └┬┘
                    │       ┌┴┐              │   │
                    │       │ │ R3           │   │
                    │       │ │ 1MΩ          │  ─┴─ GND
                    │       └┬┘              │
                    │        │               │
                    └────────┴───────────────┘
                             │
                            ─┴─ GND


DC Bias Circuit (connected to LM358 U1A non-inverting input):

        +5V
         │
        ┌┴┐
        │ │ R5
        │ │ 10kΩ
        └┬┘
         │
         ├──────► To U1A + input
         │
        ┌┴┐
        │ │ R6
        │ │ 10kΩ
        └┬┘
         │
        ─┴─ GND
```

### Component Values

| Reference | Value | Purpose |
|-----------|-------|---------|
| C1 | 100nF (0.1µF) | DC blocking / input coupling |
| R1 | 10kΩ | Pull-up / bias |
| R2 | 100kΩ | Input impedance |
| R3 | 1MΩ | Feedback resistor (gain) |
| R4 | 10kΩ | Output limiting |
| R5 | 10kΩ | Voltage divider (bias) |
| R6 | 10kΩ | Voltage divider (bias) |
| U1A | LM358N | Op-amp (half of dual IC) |

### Gain Calculation

The inverting amplifier gain is: **G = -R3/R2 = -1MΩ/100kΩ = -10**

This provides a gain of 10x (inverting), which amplifies the microvolt HB100 signal to millivolt levels readable by the Arduino ADC.

### Bandpass Filtering

For better noise rejection, add an optional bandpass filter:

```
Additional Components:
- C2: 10µF electrolytic (input high-pass, fc ≈ 0.16 Hz)
- C3: 100nF ceramic (output low-pass, fc ≈ 160 Hz)

                    C2
Input ───────┤├───────► Pre-amp input

Pre-amp output ────┬────► To Arduino A0
                   │
                  ─┴─
                 ─┬─┬─ C3
                   │
                  ─┴─ GND
```

---

## Assembly Instructions

### Step 1: Prepare the Breadboard

1. Place the Arduino Uno next to the breadboard
2. Connect Arduino 5V and GND to breadboard power rails
3. Connect both power rails (+ and -) across the breadboard

### Step 2: Build the Pre-Amplifier

1. Insert LM358N IC into the breadboard (straddle the center gap)
2. Connect pin 8 (VCC) to +5V rail
3. Connect pin 4 (GND) to GND rail
4. Build the voltage divider (R5, R6) for 2.5V bias
5. Connect bias to pin 3 (U1A non-inverting input)
6. Add R2 (100kΩ) from pin 2 to GND
7. Add R3 (1MΩ) feedback from pin 1 to pin 2
8. Add C1 (100nF) input coupling capacitor
9. Add R4 (10kΩ) output resistor

### Step 3: Connect HB100 Radar

1. Connect HB100 VCC to +5V rail
2. Connect HB100 GND to GND rail
3. Connect HB100 IF output to C1 input

### Step 4: Connect to Arduino

1. Connect pre-amp output to Arduino A0
2. Verify power connections

### Step 5: Connect LED Matrix

1. Connect MAX7219 VCC to Arduino 5V
2. Connect MAX7219 GND to Arduino GND
3. Connect MAX7219 DIN to Arduino D11
4. Connect MAX7219 CS to Arduino D10
5. Connect MAX7219 CLK to Arduino D13

---

## Testing

### Test 1: Power Check

1. Connect Arduino via USB (do NOT connect HB100 yet)
2. Measure voltage at pre-amp bias point - should read ~2.5V
3. Measure voltage at op-amp VCC (pin 8) - should read ~5V

### Test 2: Pre-Amp Test

1. Connect HB100
2. Open Arduino Serial Monitor at 115200 baud
3. Upload test sketch to read A0 and print values
4. With no motion, values should be around 512 (2.5V = mid-scale)
5. Wave hand in front of sensor - values should fluctuate

### Test 3: LED Matrix Test

1. Upload the main firmware
2. On startup, LED matrix should show initialization animation
3. Matrix should show breathing dot in idle state

### Test 4: Full System Test

1. Upload main MotionController.ino firmware
2. Open Serial Monitor at 115200 baud
3. Wave hand - should see JSON gesture events
4. Verify LED matrix shows motion indication

---

## Troubleshooting

| Problem | Possible Cause | Solution |
|---------|----------------|----------|
| No signal on A0 | Pre-amp not powered | Check VCC/GND connections |
| Signal stuck at 0 or 1023 | Op-amp saturated | Check bias voltage (should be 2.5V) |
| Very noisy signal | Insufficient filtering | Add bypass capacitors |
| No gesture detection | Signal too weak | Increase gain (larger R3) |
| LED matrix not working | Wrong pin connections | Verify DIN/CLK/CS pins |
| Erratic behavior | Power supply noise | Add 100µF cap across Arduino 5V/GND |

---

## Safety Notes

- The HB100 operates at 10.525 GHz microwave frequency
- RF output power is very low (<10 mW) and safe for normal use
- Do not stare directly at the radar aperture at close range
- Keep away from sensitive medical equipment

---

## Wiring Diagram (Fritzing-style)

```
    ┌────────────────────────────────────────────────────────────────┐
    │                        BREADBOARD                               │
    │  + ─────────────────────────────────────────────────────── +   │
    │  - ─────────────────────────────────────────────────────── -   │
    │                                                                 │
    │           ┌─────────┐                                          │
    │           │ LM358N  │                                          │
    │           │ 1     8 │──── +5V                                  │
    │    Out ◄──│ 2     7 │                                          │
    │    ─ In ──│ 3     6 │                                          │
    │    GND ───│ 4     5 │                                          │
    │           └─────────┘                                          │
    │                                                                 │
    └────────────────────────────────────────────────────────────────┘

    ┌───────────┐         ┌─────────────┐         ┌──────────────────┐
    │  HB100    │         │  Pre-Amp    │         │   Arduino Uno    │
    │           │         │  Circuit    │         │                  │
    │  VCC ─────┼────────►│ +5V         │         │            5V ◄──┼─┐
    │  GND ─────┼────────►│ GND         │         │           GND ◄──┼─┤
    │  IF  ─────┼────────►│ IN    OUT ──┼────────►│ A0               │ │
    │           │         │             │         │                  │ │
    └───────────┘         └─────────────┘         │ D11 ◄────────────┼─┤ DIN
                                                  │ D10 ◄────────────┼─┤ CS
                                                  │ D13 ◄────────────┼─┤ CLK
                                                  │                  │ │
                                                  └──────────────────┘ │
                                                                       │
                                                  ┌──────────────────┐ │
                                                  │  MAX7219 8x8     │ │
                                                  │  LED Matrix      │ │
                                                  │                  │ │
                                                  │  VCC ────────────┼─┘
                                                  │  GND ────────────┼── GND
                                                  │  DIN             │
                                                  │  CS              │
                                                  │  CLK             │
                                                  └──────────────────┘
```
