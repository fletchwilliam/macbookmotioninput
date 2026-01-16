"""
event_generator.py - macOS system event generation

Translates gesture events into macOS system events using Quartz/CoreGraphics.
"""

import subprocess
from typing import Dict, Optional
from dataclasses import dataclass

# macOS frameworks via PyObjC
try:
    from Quartz import (
        CGEventCreateMouseEvent,
        CGEventCreateScrollWheelEvent,
        CGEventCreateKeyboardEvent,
        CGEventPost,
        CGEventSetFlags,
        CGEventSetIntegerValueField,
        kCGEventMouseMoved,
        kCGEventLeftMouseDown,
        kCGEventLeftMouseUp,
        kCGEventRightMouseDown,
        kCGEventRightMouseUp,
        kCGEventOtherMouseDown,
        kCGEventOtherMouseUp,
        kCGEventScrollWheel,
        kCGScrollEventUnitPixel,
        kCGHIDEventTap,
        kCGEventFlagMaskCommand,
        kCGEventFlagMaskControl,
        kCGEventFlagMaskAlternate,
        kCGEventFlagMaskShift,
        kCGMouseButtonLeft,
        kCGMouseButtonRight,
        kCGMouseButtonCenter,
    )
    from Quartz.CoreGraphics import CGEventGetLocation
    from AppKit import NSEvent
    QUARTZ_AVAILABLE = True
except ImportError:
    QUARTZ_AVAILABLE = False
    print("Warning: Quartz framework not available. Event generation disabled.")


@dataclass
class MousePosition:
    """Current mouse position."""
    x: float
    y: float


class EventGenerator:
    """Generates macOS system events from gesture commands."""

    # Key code mapping for common keys
    KEY_CODES = {
        'a': 0, 'b': 11, 'c': 8, 'd': 2, 'e': 14, 'f': 3, 'g': 5, 'h': 4,
        'i': 34, 'j': 38, 'k': 40, 'l': 37, 'm': 46, 'n': 45, 'o': 31, 'p': 35,
        'q': 12, 'r': 15, 's': 1, 't': 17, 'u': 32, 'v': 9, 'w': 13, 'x': 7,
        'y': 16, 'z': 6,
        '1': 18, '2': 19, '3': 20, '4': 21, '5': 23, '6': 22, '7': 26, '8': 28,
        '9': 25, '0': 29,
        'space': 49, 'return': 36, 'tab': 48, 'escape': 53, 'delete': 51,
        'up': 126, 'down': 125, 'left': 123, 'right': 124,
        'f1': 122, 'f2': 120, 'f3': 99, 'f4': 118, 'f5': 96, 'f6': 97,
        'f7': 98, 'f8': 100, 'f9': 101, 'f10': 109, 'f11': 103, 'f12': 111,
    }

    def __init__(self):
        self.enabled = QUARTZ_AVAILABLE

    def get_mouse_position(self) -> MousePosition:
        """Get current mouse cursor position."""
        if not self.enabled:
            return MousePosition(0, 0)

        loc = NSEvent.mouseLocation()
        # Convert from bottom-left origin to top-left origin
        screen_height = NSEvent.valueForKey_("screenHeight") or 1080
        return MousePosition(loc.x, screen_height - loc.y)

    def execute_action(self, action: str, params: Dict) -> bool:
        """Execute an action with given parameters."""
        if not self.enabled:
            print(f"Event generation disabled. Would execute: {action} {params}")
            return False

        actions = {
            "click": self._click,
            "double_click": self._double_click,
            "scroll": self._scroll,
            "mission_control": self._mission_control,
            "app_switcher": self._app_switcher,
            "launchpad": self._launchpad,
            "notification_center": self._notification_center,
            "keystroke": self._keystroke,
            "none": lambda **_: True,
        }

        handler = actions.get(action)
        if handler:
            try:
                return handler(**params)
            except Exception as e:
                print(f"Error executing action {action}: {e}")
                return False

        print(f"Unknown action: {action}")
        return False

    def _click(self, button: str = "left") -> bool:
        """Perform a mouse click."""
        pos = self.get_mouse_position()
        point = (pos.x, pos.y)

        if button == "left":
            down_type = kCGEventLeftMouseDown
            up_type = kCGEventLeftMouseUp
            btn = kCGMouseButtonLeft
        elif button == "right":
            down_type = kCGEventRightMouseDown
            up_type = kCGEventRightMouseUp
            btn = kCGMouseButtonRight
        else:  # middle
            down_type = kCGEventOtherMouseDown
            up_type = kCGEventOtherMouseUp
            btn = kCGMouseButtonCenter

        # Create and post mouse down event
        event_down = CGEventCreateMouseEvent(None, down_type, point, btn)
        CGEventPost(kCGHIDEventTap, event_down)

        # Create and post mouse up event
        event_up = CGEventCreateMouseEvent(None, up_type, point, btn)
        CGEventPost(kCGHIDEventTap, event_up)

        return True

    def _double_click(self, button: str = "left") -> bool:
        """Perform a double click."""
        pos = self.get_mouse_position()
        point = (pos.x, pos.y)

        if button == "left":
            down_type = kCGEventLeftMouseDown
            up_type = kCGEventLeftMouseUp
            btn = kCGMouseButtonLeft
        else:
            down_type = kCGEventRightMouseDown
            up_type = kCGEventRightMouseUp
            btn = kCGMouseButtonRight

        # First click
        event_down = CGEventCreateMouseEvent(None, down_type, point, btn)
        CGEventSetIntegerValueField(event_down, 1, 1)  # Click count = 1
        CGEventPost(kCGHIDEventTap, event_down)

        event_up = CGEventCreateMouseEvent(None, up_type, point, btn)
        CGEventSetIntegerValueField(event_up, 1, 1)
        CGEventPost(kCGHIDEventTap, event_up)

        # Second click
        event_down = CGEventCreateMouseEvent(None, down_type, point, btn)
        CGEventSetIntegerValueField(event_down, 1, 2)  # Click count = 2
        CGEventPost(kCGHIDEventTap, event_down)

        event_up = CGEventCreateMouseEvent(None, up_type, point, btn)
        CGEventSetIntegerValueField(event_up, 1, 2)
        CGEventPost(kCGHIDEventTap, event_up)

        return True

    def _scroll(self, direction: str = "down", amount: int = 5) -> bool:
        """Perform a scroll action."""
        # Determine scroll direction and amount
        if direction == "down":
            delta_y = -amount
            delta_x = 0
        elif direction == "up":
            delta_y = amount
            delta_x = 0
        elif direction == "left":
            delta_y = 0
            delta_x = amount
        else:  # right
            delta_y = 0
            delta_x = -amount

        # Create scroll event
        event = CGEventCreateScrollWheelEvent(
            None,
            kCGScrollEventUnitPixel,
            2,  # Number of axes
            delta_y * 10,  # Multiply for more noticeable scroll
            delta_x * 10
        )
        CGEventPost(kCGHIDEventTap, event)

        return True

    def _mission_control(self) -> bool:
        """Activate Mission Control (Control + Up Arrow)."""
        return self._send_keystroke("up", ["ctrl"])

    def _app_switcher(self) -> bool:
        """Activate App Switcher (Cmd + Tab)."""
        return self._send_keystroke("tab", ["cmd"])

    def _launchpad(self) -> bool:
        """Activate Launchpad via AppleScript."""
        try:
            subprocess.run([
                "osascript", "-e",
                'tell application "System Events" to key code 160'  # Launchpad key
            ], check=True, capture_output=True)
            return True
        except subprocess.CalledProcessError:
            # Fallback: use F4 with Fn (if configured)
            return self._send_keystroke("f4", [])

    def _notification_center(self) -> bool:
        """Toggle Notification Center via AppleScript."""
        try:
            # Click the date/time in menu bar to toggle Notification Center
            script = '''
            tell application "System Events"
                tell process "ControlCenter"
                    click menu bar item "Clock" of menu bar 1
                end tell
            end tell
            '''
            subprocess.run(["osascript", "-e", script], check=True, capture_output=True)
            return True
        except subprocess.CalledProcessError:
            return False

    def _keystroke(self, key: str = "", modifiers: list = None) -> bool:
        """Send a keyboard shortcut."""
        if not key:
            return False
        return self._send_keystroke(key, modifiers or [])

    def _send_keystroke(self, key: str, modifiers: list) -> bool:
        """Send a keystroke with modifiers."""
        key_code = self.KEY_CODES.get(key.lower())
        if key_code is None:
            print(f"Unknown key: {key}")
            return False

        # Build modifier flags
        flags = 0
        for mod in modifiers:
            if mod == "cmd":
                flags |= kCGEventFlagMaskCommand
            elif mod == "ctrl":
                flags |= kCGEventFlagMaskControl
            elif mod == "alt":
                flags |= kCGEventFlagMaskAlternate
            elif mod == "shift":
                flags |= kCGEventFlagMaskShift

        # Key down
        event_down = CGEventCreateKeyboardEvent(None, key_code, True)
        if flags:
            CGEventSetFlags(event_down, flags)
        CGEventPost(kCGHIDEventTap, event_down)

        # Key up
        event_up = CGEventCreateKeyboardEvent(None, key_code, False)
        if flags:
            CGEventSetFlags(event_up, flags)
        CGEventPost(kCGHIDEventTap, event_up)

        return True


class GestureActionExecutor:
    """Executes macOS actions based on gesture events."""

    def __init__(self, config):
        self.config = config
        self.event_gen = EventGenerator()
        self.last_gesture_time = 0
        self.min_gesture_interval = 0.1  # Minimum time between gestures (seconds)

    def handle_gesture(self, gesture: str, confidence: float) -> bool:
        """Handle a gesture event and execute the mapped action."""
        import time

        # Rate limiting
        current_time = time.time()
        if current_time - self.last_gesture_time < self.min_gesture_interval:
            return False
        self.last_gesture_time = current_time

        # Get mapping for this gesture
        mapping = self.config.get_mapping(gesture)
        if not mapping:
            print(f"No mapping for gesture: {gesture}")
            return False

        if mapping.action == "none":
            return True

        # Execute the action
        print(f"Executing: {gesture} -> {mapping.action} (confidence: {confidence:.2f})")
        return self.event_gen.execute_action(mapping.action, mapping.params)
