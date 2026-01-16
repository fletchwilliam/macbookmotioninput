"""
test_serial_handler.py - Unit tests for serial handler
"""

import pytest
from unittest.mock import Mock, patch, MagicMock
import json
import sys
import os

# Add parent directory to path for imports
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from serial_handler import SerialHandler, GestureEvent, MotionData, ConnectionState


class TestGestureEvent:
    """Tests for GestureEvent dataclass."""

    def test_from_json_valid(self):
        """Test creating GestureEvent from valid JSON."""
        data = {
            "type": "gesture",
            "gesture": "TAP",
            "confidence": 0.95,
            "timestamp": 12345
        }

        event = GestureEvent.from_json(data)

        assert event is not None
        assert event.gesture == "TAP"
        assert event.confidence == 0.95
        assert event.timestamp == 12345

    def test_from_json_wrong_type(self):
        """Test that non-gesture messages return None."""
        data = {
            "type": "motion",
            "frequency": 50.0
        }

        event = GestureEvent.from_json(data)
        assert event is None

    def test_from_json_missing_fields(self):
        """Test handling of missing fields with defaults."""
        data = {
            "type": "gesture"
        }

        event = GestureEvent.from_json(data)

        assert event is not None
        assert event.gesture == "NONE"
        assert event.confidence == 0.0
        assert event.timestamp == 0


class TestMotionData:
    """Tests for MotionData dataclass."""

    def test_from_json_valid(self):
        """Test creating MotionData from valid JSON."""
        data = {
            "type": "motion",
            "frequency": 45.5,
            "amplitude": 80,
            "direction": 1
        }

        motion = MotionData.from_json(data)

        assert motion is not None
        assert motion.frequency == 45.5
        assert motion.amplitude == 80
        assert motion.direction == 1

    def test_from_json_wrong_type(self):
        """Test that non-motion messages return None."""
        data = {
            "type": "gesture",
            "gesture": "TAP"
        }

        motion = MotionData.from_json(data)
        assert motion is None


class TestSerialHandler:
    """Tests for SerialHandler class."""

    def test_initial_state(self):
        """Test initial connection state."""
        handler = SerialHandler()
        assert handler.state == ConnectionState.DISCONNECTED

    def test_list_ports(self):
        """Test listing available ports."""
        with patch('serial.tools.list_ports.comports') as mock_comports:
            mock_port = Mock()
            mock_port.device = "/dev/ttyUSB0"
            mock_comports.return_value = [mock_port]

            handler = SerialHandler()
            ports = handler.list_ports()

            assert "/dev/ttyUSB0" in ports

    def test_find_arduino_by_vid_pid(self):
        """Test finding Arduino by VID/PID."""
        with patch('serial.tools.list_ports.comports') as mock_comports:
            mock_port = Mock()
            mock_port.device = "/dev/cu.usbmodem1234"
            mock_port.vid = 0x2341  # Arduino VID
            mock_port.pid = 0x0043  # Uno PID
            mock_port.description = "Some device"
            mock_port.manufacturer = "Unknown"
            mock_comports.return_value = [mock_port]

            handler = SerialHandler()
            port = handler.find_arduino_port()

            assert port == "/dev/cu.usbmodem1234"

    def test_find_arduino_by_description(self):
        """Test finding Arduino by description."""
        with patch('serial.tools.list_ports.comports') as mock_comports:
            mock_port = Mock()
            mock_port.device = "/dev/ttyACM0"
            mock_port.vid = None
            mock_port.pid = None
            mock_port.description = "Arduino Uno"
            mock_port.manufacturer = None
            mock_comports.return_value = [mock_port]

            handler = SerialHandler()
            port = handler.find_arduino_port()

            assert port == "/dev/ttyACM0"

    def test_callbacks_on_gesture(self):
        """Test gesture callback is called."""
        handler = SerialHandler()
        callback = Mock()
        handler.on_gesture = callback

        # Simulate processing a gesture message
        gesture_json = '{"type":"gesture","gesture":"TAP","confidence":0.9,"timestamp":1000}'
        handler._process_message(gesture_json)

        callback.assert_called_once()
        event = callback.call_args[0][0]
        assert event.gesture == "TAP"
        assert event.confidence == 0.9

    def test_callbacks_on_motion(self):
        """Test motion callback is called."""
        handler = SerialHandler()
        callback = Mock()
        handler.on_motion = callback

        motion_json = '{"type":"motion","frequency":50.0,"amplitude":75,"direction":1}'
        handler._process_message(motion_json)

        callback.assert_called_once()
        motion = callback.call_args[0][0]
        assert motion.frequency == 50.0
        assert motion.amplitude == 75

    def test_send_command_format(self):
        """Test command JSON format."""
        handler = SerialHandler()

        # Mock serial port
        mock_serial = Mock()
        mock_serial.is_open = True
        handler.serial_port = mock_serial

        handler.send_command("ping")

        mock_serial.write.assert_called_once()
        written = mock_serial.write.call_args[0][0].decode('utf-8')
        data = json.loads(written.strip())
        assert data["cmd"] == "ping"

    def test_send_command_with_value(self):
        """Test command with value."""
        handler = SerialHandler()

        mock_serial = Mock()
        mock_serial.is_open = True
        handler.serial_port = mock_serial

        handler.send_command("sensitivity", 75)

        written = mock_serial.write.call_args[0][0].decode('utf-8')
        data = json.loads(written.strip())
        assert data["cmd"] == "sensitivity"
        assert data["value"] == 75

    def test_invalid_json_handling(self):
        """Test handling of invalid JSON."""
        handler = SerialHandler()
        callback = Mock()
        handler.on_error = callback

        # Should not raise exception
        handler._process_message("not valid json {{{")

        # Error callback should not be called for parse errors
        # (they're silently ignored as might be debug output)
        callback.assert_not_called()

    def test_state_change_callback(self):
        """Test state change callback."""
        handler = SerialHandler()
        callback = Mock()
        handler.on_state_change = callback

        handler._set_state(ConnectionState.CONNECTING)

        callback.assert_called_with(ConnectionState.CONNECTING)
        assert handler.state == ConnectionState.CONNECTING


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
