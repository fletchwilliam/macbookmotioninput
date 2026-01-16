"""
config.py - Configuration management for Motion Controller Host

Handles loading, saving, and accessing configuration settings.
"""

import os
import yaml
from pathlib import Path
from dataclasses import dataclass, field, asdict
from typing import Dict, Optional

# Default configuration file location
CONFIG_DIR = Path.home() / ".config" / "motion-controller"
CONFIG_FILE = CONFIG_DIR / "config.yaml"


@dataclass
class GestureMapping:
    """Maps a gesture to a macOS action."""
    gesture: str
    action: str
    params: Dict = field(default_factory=dict)


@dataclass
class Config:
    """Application configuration."""

    # Serial port settings
    serial_port: str = ""  # Auto-detect if empty
    baud_rate: int = 115200

    # Gesture-to-action mappings
    gesture_mappings: Dict[str, dict] = field(default_factory=lambda: {
        "TAP": {"action": "click", "params": {"button": "left"}},
        "DOUBLE_TAP": {"action": "double_click", "params": {"button": "left"}},
        "PUSH": {"action": "scroll", "params": {"direction": "down", "amount": 5}},
        "PULL": {"action": "scroll", "params": {"direction": "up", "amount": 5}},
        "WAVE": {"action": "mission_control", "params": {}},
        "HOLD": {"action": "app_switcher", "params": {}},
    })

    # Device settings (sent to Arduino)
    sensitivity: int = 50
    led_brightness: int = 8

    # Application settings
    start_minimized: bool = True
    show_notifications: bool = True
    debug_mode: bool = False

    # Auto-connect settings
    auto_connect: bool = True
    reconnect_delay: float = 2.0

    def save(self, path: Optional[Path] = None):
        """Save configuration to YAML file."""
        config_path = path or CONFIG_FILE
        config_path.parent.mkdir(parents=True, exist_ok=True)

        with open(config_path, 'w') as f:
            yaml.safe_dump(asdict(self), f, default_flow_style=False)

    @classmethod
    def load(cls, path: Optional[Path] = None) -> 'Config':
        """Load configuration from YAML file."""
        config_path = path or CONFIG_FILE

        if not config_path.exists():
            # Return default config
            config = cls()
            config.save(config_path)
            return config

        with open(config_path, 'r') as f:
            data = yaml.safe_load(f)

        if data is None:
            return cls()

        return cls(**data)

    def get_mapping(self, gesture: str) -> Optional[GestureMapping]:
        """Get the action mapping for a gesture."""
        if gesture in self.gesture_mappings:
            mapping_data = self.gesture_mappings[gesture]
            return GestureMapping(
                gesture=gesture,
                action=mapping_data.get("action", "none"),
                params=mapping_data.get("params", {})
            )
        return None

    def set_mapping(self, gesture: str, action: str, params: Dict = None):
        """Set the action mapping for a gesture."""
        self.gesture_mappings[gesture] = {
            "action": action,
            "params": params or {}
        }


# Available actions for configuration UI
AVAILABLE_ACTIONS = {
    "click": {
        "name": "Mouse Click",
        "description": "Perform a mouse click",
        "params": {
            "button": {"type": "choice", "options": ["left", "right", "middle"], "default": "left"}
        }
    },
    "double_click": {
        "name": "Double Click",
        "description": "Perform a double click",
        "params": {
            "button": {"type": "choice", "options": ["left", "right"], "default": "left"}
        }
    },
    "scroll": {
        "name": "Scroll",
        "description": "Scroll the mouse wheel",
        "params": {
            "direction": {"type": "choice", "options": ["up", "down", "left", "right"], "default": "down"},
            "amount": {"type": "int", "min": 1, "max": 20, "default": 5}
        }
    },
    "mission_control": {
        "name": "Mission Control",
        "description": "Show Mission Control (all windows)",
        "params": {}
    },
    "app_switcher": {
        "name": "App Switcher",
        "description": "Show application switcher (Cmd+Tab)",
        "params": {}
    },
    "launchpad": {
        "name": "Launchpad",
        "description": "Show Launchpad",
        "params": {}
    },
    "notification_center": {
        "name": "Notification Center",
        "description": "Toggle Notification Center",
        "params": {}
    },
    "keystroke": {
        "name": "Keystroke",
        "description": "Send a keyboard shortcut",
        "params": {
            "key": {"type": "string", "default": ""},
            "modifiers": {"type": "multi_choice", "options": ["cmd", "ctrl", "alt", "shift"], "default": []}
        }
    },
    "none": {
        "name": "No Action",
        "description": "Disable this gesture",
        "params": {}
    }
}
