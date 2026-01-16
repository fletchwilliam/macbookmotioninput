"""
test_config.py - Unit tests for configuration management
"""

import pytest
import tempfile
from pathlib import Path
import sys
import os

# Add parent directory to path for imports
sys.path.insert(0, os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from config import Config, GestureMapping


class TestConfig:
    """Tests for Config class."""

    def test_default_config(self):
        """Test default configuration values."""
        config = Config()

        assert config.baud_rate == 115200
        assert config.sensitivity == 50
        assert config.led_brightness == 8
        assert config.auto_connect == True
        assert config.show_notifications == True

    def test_default_gesture_mappings(self):
        """Test default gesture mappings."""
        config = Config()

        tap_mapping = config.get_mapping("TAP")
        assert tap_mapping is not None
        assert tap_mapping.action == "click"
        assert tap_mapping.params.get("button") == "left"

        wave_mapping = config.get_mapping("WAVE")
        assert wave_mapping is not None
        assert wave_mapping.action == "mission_control"

    def test_set_mapping(self):
        """Test setting a gesture mapping."""
        config = Config()

        config.set_mapping("TAP", "double_click", {"button": "right"})

        mapping = config.get_mapping("TAP")
        assert mapping.action == "double_click"
        assert mapping.params.get("button") == "right"

    def test_save_and_load(self):
        """Test saving and loading configuration."""
        with tempfile.TemporaryDirectory() as tmpdir:
            config_path = Path(tmpdir) / "test_config.yaml"

            # Create and save config
            config = Config()
            config.sensitivity = 75
            config.set_mapping("TAP", "scroll", {"direction": "up"})
            config.save(config_path)

            # Load and verify
            loaded = Config.load(config_path)
            assert loaded.sensitivity == 75

            tap_mapping = loaded.get_mapping("TAP")
            assert tap_mapping.action == "scroll"
            assert tap_mapping.params.get("direction") == "up"

    def test_get_nonexistent_mapping(self):
        """Test getting a mapping for unknown gesture."""
        config = Config()

        mapping = config.get_mapping("UNKNOWN_GESTURE")
        assert mapping is None

    def test_load_creates_default_if_missing(self):
        """Test that loading creates default config if file doesn't exist."""
        with tempfile.TemporaryDirectory() as tmpdir:
            config_path = Path(tmpdir) / "nonexistent" / "config.yaml"

            config = Config.load(config_path)

            # Should return default config
            assert config.baud_rate == 115200
            # File should have been created
            assert config_path.exists()


class TestGestureMapping:
    """Tests for GestureMapping dataclass."""

    def test_gesture_mapping_creation(self):
        """Test creating a GestureMapping."""
        mapping = GestureMapping(
            gesture="TAP",
            action="click",
            params={"button": "left"}
        )

        assert mapping.gesture == "TAP"
        assert mapping.action == "click"
        assert mapping.params["button"] == "left"

    def test_gesture_mapping_default_params(self):
        """Test GestureMapping with default params."""
        mapping = GestureMapping(
            gesture="WAVE",
            action="mission_control"
        )

        assert mapping.params == {}


if __name__ == "__main__":
    pytest.main([__file__, "-v"])
