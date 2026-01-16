# Hardware Assembly Visual Guide

## Breadboard Layout: Jaycar PB8820 (400 Tie Points)

**Specifications:**
- 30 rows × 10 columns (center) + 2 power rails
- Dimensions: 83mm × 55mm
- Hole spacing: 2.54mm

---

## Complete Wiring Diagram

```
                        JAYCAR PB8820 BREADBOARD (400 Tie Points)
    ┌──────────────────────────────────────────────────────────────────────────────┐
    │  + - │ a b c d e │     │ f g h i j │ + - │                                   │
    │══════╪═══════════╪═════╪═══════════╪═════│  LEGEND:                          │
    │  ●─● │           │     │           │ ●─● │  ═══ Connected internally          │
    │  ●─● │           │     │           │ ●─● │  ─── Jumper wire                   │
ROW │  ●─● │           │     │           │ ●─● │  ● ● Component lead               │
 1  │  ●─● │ ●─●─●─●─● │     │ ●─●─●─●─● │ ●─● │  [X] IC pin                        │
    │══════╪═══════════╪═════╪═══════════╪═════│  ▓▓▓ Component body               │
    │      │           │     │           │     │                                    │
    └──────────────────────────────────────────────────────────────────────────────┘

    ACTUAL COMPONENT PLACEMENT:

    ┌──────────────────────────────────────────────────────────────────────────────┐
    │                                                                              │
    │   +5V RAIL              CENTER SECTION                    GND RAIL           │
    │   (RED)          a   b   c   d   e       f   g   h   i   j  (BLUE)          │
    │  ┌─────┐        ┌───────────────────────────────────────────┐ ┌─────┐       │
    │  │+ │- │        │                                           │ │+ │- │       │
    │  │  │  │      1 │ ○   ○   ○   ○   ○       ○   ○   ○   ○   ○ │ │  │  │       │
    │  │●━│━━│━━━━━━━━│━●━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━│━│━━│━●│ ←─ Arduino 5V & GND
    │  │  │  │      2 │ ○   ○   ○   ○   ○       ○   ○   ○   ○   ○ │ │  │  │       │
    │  │  │  │      3 │ ○   ○   ○   ○   ○       ○   ○   ○   ○   ○ │ │  │  │       │
    │  │  │  │        │                                           │ │  │  │       │
    │  │  │  │      4 │ ○   ○   ○   ○   ○       ○   ○   ○   ○   ○ │ │  │  │       │
    │  │●━│━━│━━━━━━━━│━●   ○   ○   ○   ○       ○   ○   ○   ○   ○ │ │  │  │ ←─ HB100 VCC
    │  │  │  │      5 │ ○   ○   ○   ○   ○       ○   ○   ○   ○   ○ │ │  │  │       │
    │  │  │  │      6 │ ○   ●━━━━━━━━━━━━━━━━━━━●   ○   ○   ○   ○ │ │  │  │ ←─ HB100 IF → C1
    │  │  │  │        │     ║ C1 (100nF)        ║                 │ │  │  │       │
    │  │  │  │      7 │ ○   ●   ○   ○   ○       ●━━━●   ○   ○   ○ │ │  │  │       │
    │  │  │  │        │     ║                       ║R2           │ │  │  │       │
    │  │  │  │      8 │ ○   ●   ○   ○   ○       ○  100k  ○   ○   ○ │ │  │  │       │
    │  │  │  │        │     ║                       ║             │ │  │  │       │
    │  │  │  │      9 │ ○   ●   ○   ○   ○       ○   ●   ○   ○   ○ │ │  │  │       │
    │  │  │  │        │     ╠════════════════════════╝             │ │  │  │       │
    │  │  │  │        │     ║         LM358N                      │ │  │  │       │
    │  │  │  │     10 │ ○   ●  ┌──────────────┐  ●   ○   ○   ○   ○ │ │  │  │       │
    │  │  │  │        │     ║  │ 1 ●    ● 8   │  ║                 │ │  │  │       │
    │  │  │  │     11 │ ○   ●──┤OUT1    VCC├──●   ○   ○   ○   ○ │ │  │  │       │
    │  │●━│━━│━━━━━━━━│━━━━━━━━┤              ├━━━━━━━━━━━━━━━━━━━━│━│━━│━●│ ←─ VCC to pin 8
    │  │  │  │     12 │ ○   ○  │ 2 ●    ● 7   │  ○   ○   ○   ○   ○ │ │  │  │       │
    │  │  │  │        │        │ -IN1    OUT2 │                    │ │  │  │       │
    │  │  │  │     13 │ ○   ○  │ 3 ●    ● 6   │  ○   ○   ○   ○   ○ │ │  │  │       │
    │  │  │  │        │        │ +IN1    -IN2 │                    │ │  │  │       │
    │  │  │  │     14 │ ○   ○  │ 4 ●    ● 5   │  ○   ○   ○   ○   ○ │ │  │  │       │
    │  │  │  │        │        │ GND     +IN2 │                    │ │  │  │       │
    │  │  │━━│━━━━━━━━│━━━━━━━━┤              ├━━━━━━━━━━━━━━━━━━━━│━│━━│━●│ ←─ GND to pin 4
    │  │  │  │     15 │ ○   ○  └──────────────┘  ○   ○   ○   ○   ○ │ │  │  │       │
    │  │  │  │        │                                           │ │  │  │       │
    │  │  │  │     16 │ ○   ○   ○   ○   ○       ○   ●━━━●   ○   ○ │ │  │  │       │
    │  │  │  │        │                             ║R3 1M        │ │  │  │       │
    │  │  │  │     17 │ ○   ○   ○   ○   ○       ○   ●   ●   ○   ○ │ │  │  │ ←─ R3 feedback
    │  │  │  │        │                             ╚═══╝         │ │  │  │       │
    │  │  │  │     18 │ ○   ○   ○   ○   ○       ○   ○   ○   ○   ○ │ │  │  │       │
    │  │  │  │        │                                           │ │  │  │       │
    │  │  │  │        │           BIAS VOLTAGE (2.5V)             │ │  │  │       │
    │  │●━│━━│━━━━━━━━│━●   ○   ○   ○   ○       ○   ○   ○   ○   ○ │ │  │  │       │
    │  │  │  │     19 │ ●━━━●   ○   ○   ○       ○   ○   ○   ○   ○ │ │  │  │       │
    │  │  │  │        │     ║R5 10k                               │ │  │  │       │
    │  │  │  │     20 │ ○   ●   ○   ○   ○       ○   ○   ○   ○   ○ │ │  │  │       │
    │  │  │  │        │     ╠══════════════════════════════► to LM358 pin 3      │
    │  │  │  │     21 │ ○   ●━━━●   ○   ○       ○   ○   ○   ○   ○ │ │  │  │       │
    │  │  │  │        │         ║R6 10k                           │ │  │  │       │
    │  │  │  │     22 │ ○   ○   ●   ○   ○       ○   ○   ○   ○   ○ │ │  │  │       │
    │  │  │━━│━━━━━━━━│━━━━━━━━━●   ○   ○       ○   ○   ○   ○   ○ │ │  │  │ ←─ R6 to GND
    │  │  │  │        │                                           │ │  │  │       │
    │  │  │  │        │          OUTPUT SECTION                   │ │  │  │       │
    │  │  │  │     23 │ ○   ○   ○   ○   ○       ○   ○   ○   ○   ○ │ │  │  │       │
    │  │  │  │     24 │ ○   ●━━━━━━━━━━━━━━━━━━━●   ○   ○   ○   ○ │ │  │  │       │
    │  │  │  │        │     ║       R4 10k      ║                 │ │  │  │       │
    │  │  │  │     25 │ ○   ●   ○   ○   ○       ●═══════════════════════► Arduino A0
    │  │  │  │        │     ║ from LM358 pin 1  ║                 │ │  │  │       │
    │  │  │  │     26 │ ○   ○   ○   ○   ○       ○   ○   ○   ○   ○ │ │  │  │       │
    │  │  │  │        │                                           │ │  │  │       │
    │  │  │  │     27 │ ○   ○   ○   ○   ○       ○   ○   ○   ○   ○ │ │  │  │       │
    │  │  │  │     28 │ ○   ○   ○   ○   ○       ○   ○   ○   ○   ○ │ │  │  │       │
    │  │  │  │     29 │ ○   ○   ○   ○   ○       ○   ○   ○   ○   ○ │ │  │  │       │
    │  │  │  │     30 │ ○   ○   ○   ○   ○       ○   ○   ○   ○   ○ │ │  │  │       │
    │  │  │  │        │                                           │ │  │  │       │
    │  └─────┘        └───────────────────────────────────────────┘ └─────┘       │
    │                                                                              │
    └──────────────────────────────────────────────────────────────────────────────┘
```

---

## Component Placement Summary

```
    BREADBOARD ROW MAP
    ══════════════════

    Row 1-3:   Power distribution (Arduino 5V/GND connections)

    Row 4-6:   HB100 Doppler Radar
               • Row 4a: VCC (from + rail)
               • Row 6b-f: IF output through C1 (100nF)

    Row 7-9:   Input coupling
               • C1 connects IF to LM358 input
               • R2 (100kΩ) to ground

    Row 10-15: LM358N Op-Amp IC
               • Straddles center gap (pins in columns d,e and f,g)
               • Pin 1 (OUT1): Row 11, Col d
               • Pin 2 (-IN1): Row 12, Col d  ← Feedback point
               • Pin 3 (+IN1): Row 13, Col d  ← Bias input
               • Pin 4 (GND):  Row 14, Col d
               • Pin 8 (VCC):  Row 11, Col g

    Row 16-17: Feedback resistor R3 (1MΩ)
               • Connects pin 1 to pin 2

    Row 19-22: Voltage divider for 2.5V bias
               • R5 (10kΩ): +5V to junction
               • R6 (10kΩ): Junction to GND
               • Junction → LM358 pin 3

    Row 24-25: Output section
               • R4 (10kΩ) current limiting
               • Output to Arduino A0
```

---

## Wire Color Code

| Color  | Purpose              | From              | To                |
|--------|----------------------|-------------------|-------------------|
| RED    | +5V Power            | Arduino 5V        | Breadboard + rail |
| BLACK  | Ground               | Arduino GND       | Breadboard - rail |
| ORANGE | Doppler Signal       | HB100 IF          | C1 Input          |
| YELLOW | Amplified Output     | LM358 Pin 1       | R4                |
| GREEN  | To Arduino           | R4 Output         | Arduino A0        |
| BLUE   | Bias Voltage         | Divider Junction  | LM358 Pin 3       |

---

## External Connections Diagram

```
                                    ┌─────────────────────┐
                                    │     ARDUINO UNO     │
                                    │                     │
    ┌───────────────┐               │  ┌─────────────┐   │
    │  HB100 RADAR  │               │  │    USB      │   │
    │               │               │  │   Type-B    │   │
    │  ┌─────────┐  │               │  └──────┬──────┘   │
    │  │ Antenna │  │               │         │          │
    │  │ Patch   │  │               │    To MacBook      │
    │  └─────────┘  │               │                    │
    │               │               │ DIGITAL      5V ●──┼──► Breadboard +
    │    VCC ●──────┼───────────────┼──────────────────  │
    │    GND ●──────┼───────────────┼─────────── GND ●──┼──► Breadboard -
    │     IF ●──────┼──► C1 ──► Pre-Amp ──► A0 ●        │
    │               │               │                    │
    └───────────────┘               │               D11 ●──┐
                                    │               D10 ●──┼──► LED Matrix
                                    │               D13 ●──┘
    ┌───────────────┐               │                    │
    │ MAX7219 8x8   │               └────────────────────┘
    │  LED MATRIX   │
    │               │
    │   VCC ●───────┼──► Arduino 5V (via breadboard)
    │   GND ●───────┼──► Arduino GND (via breadboard)
    │   DIN ●───────┼──► Arduino D11
    │    CS ●───────┼──► Arduino D10
    │   CLK ●───────┼──► Arduino D13
    │               │
    │  ┌─────────┐  │
    │  │ ● ● ● ● │  │
    │  │ ● ● ● ● │  │
    │  │ ● ● ● ● │  │
    │  │ ● ● ● ● │  │
    │  │ ● ● ● ● │  │
    │  │ ● ● ● ● │  │
    │  │ ● ● ● ● │  │
    │  │ ● ● ● ● │  │
    │  └─────────┘  │
    └───────────────┘
```

---

## Physical Layout (Top View)

```
    ┌──────────────────────────────────────────────────────────────────────┐
    │                                                                      │
    │   ┌──────────────┐        ┌─────────────────┐    ┌──────────────┐   │
    │   │              │        │   BREADBOARD    │    │              │   │
    │   │    HB100     │        │    PB8820       │    │   MAX7219    │   │
    │   │   DOPPLER    │        │                 │    │  LED MATRIX  │   │
    │   │    RADAR     │◄──────►│   [LM358N]      │    │              │   │
    │   │              │  Wire  │   [Resistors]   │    │   ████████   │   │
    │   │  ┌────────┐  │        │   [Capacitor]   │    │   ████████   │   │
    │   │  │ ▓▓▓▓▓▓ │  │        │                 │    │   ████████   │   │
    │   │  │ ▓▓▓▓▓▓ │  │        │                 │    │   ████████   │   │
    │   │  └────────┘  │        └────────┬────────┘    │              │   │
    │   └──────────────┘                 │             └──────┬───────┘   │
    │                                    │                    │           │
    │                              ┌─────┴─────┐              │           │
    │                              │           │              │           │
    │                          ┌───┴───────────┴───┐          │           │
    │                          │                   │◄─────────┘           │
    │                          │   ARDUINO UNO     │                      │
    │                          │       R3          │                      │
    │                          │                   │                      │
    │                          │   ┌───────────┐   │                      │
    │                          │   │    USB    │   │                      │
    │                          │   └─────┬─────┘   │                      │
    │                          └─────────┼─────────┘                      │
    │                                    │                                │
    │                                    ▼                                │
    │                              To MacBook                             │
    │                                                                     │
    └─────────────────────────────────────────────────────────────────────┘

    Approximate Dimensions:
    ─────────────────────────
    HB100 Radar:    38mm × 30mm
    Breadboard:     83mm × 55mm
    LED Matrix:     32mm × 32mm
    Arduino Uno:    68mm × 53mm

    Total Footprint: ~150mm × 120mm (6" × 5")
```

---

## Step-by-Step Assembly

### Step 1: Power Rails
```
Arduino 5V  ────────►  Breadboard + (red) rail
Arduino GND ────────►  Breadboard - (blue) rail
```

### Step 2: Place LM358N IC
```
Insert LM358N at rows 10-14, straddling the center gap
Pin 1 at row 11 column d, Pin 8 at row 11 column g
```

### Step 3: Op-Amp Power
```
Row 11g (Pin 8) ────────►  + rail (VCC)
Row 14d (Pin 4) ────────►  - rail (GND)
```

### Step 4: Bias Voltage Divider
```
+ rail  ──► R5 (10kΩ) ──► Junction ──► R6 (10kΩ) ──► - rail
                              │
                              └──► LM358 Pin 3 (row 13d)
```

### Step 5: Input Coupling
```
HB100 IF ──► C1 (100nF) ──► R2 (100kΩ) to GND
                   │
                   └──► LM358 Pin 2 (row 12d)
```

### Step 6: Feedback
```
LM358 Pin 1 (row 11d) ──► R3 (1MΩ) ──► LM358 Pin 2 (row 12d)
```

### Step 7: Output
```
LM358 Pin 1 ──► R4 (10kΩ) ──► Arduino A0
```

### Step 8: Connect HB100
```
HB100 VCC ────────►  + rail
HB100 GND ────────►  - rail
HB100 IF  ────────►  C1 input
```

### Step 9: Connect LED Matrix
```
VCC ────────►  Arduino 5V (can share breadboard + rail)
GND ────────►  Arduino GND (can share breadboard - rail)
DIN ────────►  Arduino D11
CS  ────────►  Arduino D10
CLK ────────►  Arduino D13
```

---

## Verification Checklist

- [ ] No short circuits between + and - rails
- [ ] LM358N orientation correct (notch/dot at pin 1)
- [ ] All component legs fully inserted
- [ ] Bias voltage reads ~2.5V at Pin 3
- [ ] VCC reads ~5V at Pin 8
- [ ] Output reads ~2.5V at rest (no motion)
- [ ] LED matrix shows startup animation
