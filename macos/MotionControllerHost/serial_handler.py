"""
serial_handler.py - Serial port communication with Arduino

Handles connection, reconnection, and message parsing for the Motion Controller.
"""

import json
import threading
import time
from typing import Callable, Optional, List
from dataclasses import dataclass
from enum import Enum
import serial
import serial.tools.list_ports


class ConnectionState(Enum):
    """Serial connection states."""
    DISCONNECTED = "disconnected"
    CONNECTING = "connecting"
    CONNECTED = "connected"
    ERROR = "error"


@dataclass
class GestureEvent:
    """Represents a gesture event from the Arduino."""
    gesture: str
    confidence: float
    timestamp: int

    @classmethod
    def from_json(cls, data: dict) -> Optional['GestureEvent']:
        """Create GestureEvent from JSON data."""
        if data.get("type") != "gesture":
            return None
        return cls(
            gesture=data.get("gesture", "NONE"),
            confidence=data.get("confidence", 0.0),
            timestamp=data.get("timestamp", 0)
        )


@dataclass
class MotionData:
    """Represents motion data from the Arduino (debug/calibration)."""
    frequency: float
    amplitude: int
    direction: int

    @classmethod
    def from_json(cls, data: dict) -> Optional['MotionData']:
        """Create MotionData from JSON data."""
        if data.get("type") != "motion":
            return None
        return cls(
            frequency=data.get("frequency", 0.0),
            amplitude=data.get("amplitude", 0),
            direction=data.get("direction", 0)
        )


class SerialHandler:
    """Handles serial communication with the Arduino Motion Controller."""

    # Known Arduino USB identifiers
    ARDUINO_VID_PID = [
        (0x2341, 0x0043),  # Arduino Uno
        (0x2341, 0x0001),  # Arduino Uno (older)
        (0x2A03, 0x0043),  # Arduino Uno (clone)
        (0x1A86, 0x7523),  # CH340 (common clone chip)
    ]

    def __init__(self, baud_rate: int = 115200):
        self.baud_rate = baud_rate
        self.serial_port: Optional[serial.Serial] = None
        self.port_name: str = ""
        self.state = ConnectionState.DISCONNECTED

        # Callbacks
        self.on_gesture: Optional[Callable[[GestureEvent], None]] = None
        self.on_motion: Optional[Callable[[MotionData], None]] = None
        self.on_state_change: Optional[Callable[[ConnectionState], None]] = None
        self.on_message: Optional[Callable[[dict], None]] = None
        self.on_error: Optional[Callable[[str], None]] = None

        # Threading
        self._read_thread: Optional[threading.Thread] = None
        self._running = False
        self._reconnect_enabled = True
        self._reconnect_delay = 2.0

    def find_arduino_port(self) -> Optional[str]:
        """Auto-detect Arduino serial port."""
        ports = serial.tools.list_ports.comports()

        for port in ports:
            # Check by VID/PID
            if port.vid and port.pid:
                for vid, pid in self.ARDUINO_VID_PID:
                    if port.vid == vid and port.pid == pid:
                        return port.device

            # Check by description
            desc_lower = (port.description or "").lower()
            if "arduino" in desc_lower or "ch340" in desc_lower:
                return port.device

            # Check by manufacturer
            mfr_lower = (port.manufacturer or "").lower()
            if "arduino" in mfr_lower:
                return port.device

        # Fallback: return first USB serial port on macOS
        for port in ports:
            if "/dev/cu.usb" in port.device:
                return port.device

        return None

    def list_ports(self) -> List[str]:
        """List available serial ports."""
        return [port.device for port in serial.tools.list_ports.comports()]

    def connect(self, port: Optional[str] = None) -> bool:
        """Connect to the Arduino."""
        if self.state == ConnectionState.CONNECTED:
            return True

        self._set_state(ConnectionState.CONNECTING)

        # Find port if not specified
        if not port:
            port = self.find_arduino_port()
            if not port:
                self._set_state(ConnectionState.ERROR)
                if self.on_error:
                    self.on_error("No Arduino found. Please check connection.")
                return False

        try:
            self.serial_port = serial.Serial(
                port=port,
                baudrate=self.baud_rate,
                timeout=1.0,
                write_timeout=1.0
            )
            self.port_name = port

            # Wait for Arduino reset (after serial connection)
            time.sleep(2.0)

            # Clear any pending data
            self.serial_port.reset_input_buffer()

            # Start read thread
            self._running = True
            self._read_thread = threading.Thread(target=self._read_loop, daemon=True)
            self._read_thread.start()

            self._set_state(ConnectionState.CONNECTED)
            return True

        except serial.SerialException as e:
            self._set_state(ConnectionState.ERROR)
            if self.on_error:
                self.on_error(f"Failed to connect: {e}")
            return False

    def disconnect(self):
        """Disconnect from the Arduino."""
        self._running = False
        self._reconnect_enabled = False

        if self._read_thread and self._read_thread.is_alive():
            self._read_thread.join(timeout=2.0)

        if self.serial_port and self.serial_port.is_open:
            self.serial_port.close()

        self.serial_port = None
        self._set_state(ConnectionState.DISCONNECTED)

    def send_command(self, command: str, value=None) -> bool:
        """Send a command to the Arduino."""
        if not self.serial_port or not self.serial_port.is_open:
            return False

        try:
            cmd_data = {"cmd": command}
            if value is not None:
                cmd_data["value"] = value

            message = json.dumps(cmd_data) + "\n"
            self.serial_port.write(message.encode('utf-8'))
            return True

        except serial.SerialException:
            return False

    def send_ping(self) -> bool:
        """Send a ping to check connection."""
        return self.send_command("ping")

    def set_sensitivity(self, value: int) -> bool:
        """Set motion sensitivity on Arduino."""
        return self.send_command("sensitivity", value)

    def set_brightness(self, value: int) -> bool:
        """Set LED brightness on Arduino."""
        return self.send_command("brightness", value)

    def set_debug(self, enabled: bool) -> bool:
        """Enable/disable debug mode on Arduino."""
        return self.send_command("debug", enabled)

    def calibrate(self) -> bool:
        """Start calibration on Arduino."""
        return self.send_command("calibrate")

    def _set_state(self, state: ConnectionState):
        """Update connection state and notify callback."""
        self.state = state
        if self.on_state_change:
            self.on_state_change(state)

    def _read_loop(self):
        """Background thread for reading serial data."""
        while self._running:
            try:
                if not self.serial_port or not self.serial_port.is_open:
                    break

                if self.serial_port.in_waiting > 0:
                    line = self.serial_port.readline().decode('utf-8').strip()
                    if line:
                        self._process_message(line)
                else:
                    time.sleep(0.01)  # Small delay to prevent busy waiting

            except serial.SerialException as e:
                if self._running:
                    self._set_state(ConnectionState.ERROR)
                    if self.on_error:
                        self.on_error(f"Serial error: {e}")

                    # Attempt reconnection
                    if self._reconnect_enabled:
                        self._attempt_reconnect()
                break

            except Exception as e:
                if self.on_error:
                    self.on_error(f"Read error: {e}")

    def _process_message(self, line: str):
        """Process a JSON message from the Arduino."""
        try:
            data = json.loads(line)

            # Notify generic message callback
            if self.on_message:
                self.on_message(data)

            msg_type = data.get("type", "")

            # Handle gesture events
            if msg_type == "gesture":
                event = GestureEvent.from_json(data)
                if event and self.on_gesture:
                    self.on_gesture(event)

            # Handle motion data
            elif msg_type == "motion":
                motion = MotionData.from_json(data)
                if motion and self.on_motion:
                    self.on_motion(motion)

            # Handle ready message
            elif msg_type == "ready":
                version = data.get("version", "unknown")
                if self.on_message:
                    self.on_message({"type": "ready", "version": version})

            # Handle pong response
            elif msg_type == "pong":
                pass  # Connection confirmed

            # Handle errors from device
            elif msg_type == "error":
                if self.on_error:
                    self.on_error(data.get("message", "Unknown device error"))

        except json.JSONDecodeError:
            # Not valid JSON, might be debug output
            pass

    def _attempt_reconnect(self):
        """Attempt to reconnect after connection loss."""
        if not self._reconnect_enabled:
            return

        self._set_state(ConnectionState.CONNECTING)

        while self._running and self._reconnect_enabled:
            time.sleep(self._reconnect_delay)

            if self.serial_port:
                try:
                    self.serial_port.close()
                except Exception:
                    pass

            port = self.find_arduino_port()
            if port:
                try:
                    self.serial_port = serial.Serial(
                        port=port,
                        baudrate=self.baud_rate,
                        timeout=1.0
                    )
                    time.sleep(2.0)  # Wait for Arduino reset
                    self.serial_port.reset_input_buffer()
                    self._set_state(ConnectionState.CONNECTED)
                    return

                except serial.SerialException:
                    pass

        self._set_state(ConnectionState.DISCONNECTED)
