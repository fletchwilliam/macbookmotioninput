"""
menu_bar_app.py - macOS menu bar application for Motion Controller

Provides a system tray interface for configuration and status monitoring.
"""

import rumps
from typing import Optional

from config import Config, AVAILABLE_ACTIONS
from serial_handler import SerialHandler, ConnectionState, GestureEvent
from event_generator import GestureActionExecutor


class MotionControllerApp(rumps.App):
    """Menu bar application for Motion Controller."""

    # Status icons (using emoji as placeholders - can be replaced with actual icons)
    ICON_CONNECTED = "🟢"
    ICON_DISCONNECTED = "🔴"
    ICON_CONNECTING = "🟡"

    def __init__(self):
        super().__init__(
            name="Motion Controller",
            title="MC",
            quit_button=None  # We'll add our own quit button
        )

        # Load configuration
        self.config = Config.load()

        # Initialize components
        self.serial_handler = SerialHandler(baud_rate=self.config.baud_rate)
        self.gesture_executor = GestureActionExecutor(self.config)

        # Set up callbacks
        self.serial_handler.on_gesture = self._on_gesture
        self.serial_handler.on_state_change = self._on_state_change
        self.serial_handler.on_error = self._on_error
        self.serial_handler.on_message = self._on_message

        # State
        self.is_enabled = True
        self.gesture_count = 0

        # Build menu
        self._build_menu()

        # Auto-connect if enabled
        if self.config.auto_connect:
            self._connect()

    def _build_menu(self):
        """Build the menu bar menu."""
        # Status item (non-clickable)
        self.status_item = rumps.MenuItem("Status: Disconnected")
        self.status_item.set_callback(None)

        # Connection controls
        self.connect_item = rumps.MenuItem("Connect", callback=self._on_connect_click)
        self.disconnect_item = rumps.MenuItem("Disconnect", callback=self._on_disconnect_click)
        self.disconnect_item.set_callback(None)  # Initially disabled

        # Enable/Disable toggle
        self.enable_item = rumps.MenuItem("Enabled", callback=self._on_enable_toggle)
        self.enable_item.state = True

        # Gesture mappings submenu
        self.mappings_menu = rumps.MenuItem("Gesture Mappings")
        self._build_mappings_submenu()

        # Settings submenu
        self.settings_menu = rumps.MenuItem("Settings")
        self._build_settings_submenu()

        # Calibrate
        self.calibrate_item = rumps.MenuItem("Calibrate", callback=self._on_calibrate)

        # Debug mode toggle
        self.debug_item = rumps.MenuItem("Debug Mode", callback=self._on_debug_toggle)
        self.debug_item.state = self.config.debug_mode

        # Stats
        self.stats_item = rumps.MenuItem("Gestures: 0")
        self.stats_item.set_callback(None)

        # Quit
        self.quit_item = rumps.MenuItem("Quit", callback=self._on_quit)

        # Build menu structure
        self.menu = [
            self.status_item,
            None,  # Separator
            self.connect_item,
            self.disconnect_item,
            None,
            self.enable_item,
            None,
            self.mappings_menu,
            self.settings_menu,
            self.calibrate_item,
            None,
            self.debug_item,
            self.stats_item,
            None,
            self.quit_item,
        ]

    def _build_mappings_submenu(self):
        """Build gesture mappings submenu."""
        gestures = ["TAP", "DOUBLE_TAP", "PUSH", "PULL", "WAVE", "HOLD"]

        for gesture in gestures:
            mapping = self.config.get_mapping(gesture)
            action_name = AVAILABLE_ACTIONS.get(mapping.action, {}).get("name", mapping.action) if mapping else "None"

            gesture_menu = rumps.MenuItem(f"{gesture}: {action_name}")

            # Add action options
            for action_id, action_info in AVAILABLE_ACTIONS.items():
                action_item = rumps.MenuItem(
                    action_info["name"],
                    callback=lambda sender, g=gesture, a=action_id: self._on_mapping_change(g, a)
                )
                if mapping and mapping.action == action_id:
                    action_item.state = True
                gesture_menu.add(action_item)

            self.mappings_menu.add(gesture_menu)

    def _build_settings_submenu(self):
        """Build settings submenu."""
        # Sensitivity slider (simplified as menu items)
        sensitivity_menu = rumps.MenuItem("Sensitivity")
        for level in [25, 50, 75, 100]:
            item = rumps.MenuItem(
                f"{level}%",
                callback=lambda sender, v=level: self._on_sensitivity_change(v)
            )
            if self.config.sensitivity == level:
                item.state = True
            sensitivity_menu.add(item)
        self.settings_menu.add(sensitivity_menu)

        # LED Brightness
        brightness_menu = rumps.MenuItem("LED Brightness")
        for level in [0, 4, 8, 12, 15]:
            name = {0: "Off", 4: "Low", 8: "Medium", 12: "High", 15: "Max"}[level]
            item = rumps.MenuItem(
                name,
                callback=lambda sender, v=level: self._on_brightness_change(v)
            )
            if self.config.led_brightness == level:
                item.state = True
            brightness_menu.add(item)
        self.settings_menu.add(brightness_menu)

        # Notifications toggle
        notif_item = rumps.MenuItem(
            "Show Notifications",
            callback=self._on_notifications_toggle
        )
        notif_item.state = self.config.show_notifications
        self.settings_menu.add(notif_item)

        # Auto-connect toggle
        auto_item = rumps.MenuItem(
            "Auto-Connect",
            callback=self._on_auto_connect_toggle
        )
        auto_item.state = self.config.auto_connect
        self.settings_menu.add(auto_item)

    def _connect(self):
        """Connect to the Arduino."""
        port = self.config.serial_port if self.config.serial_port else None
        self.serial_handler.connect(port)

    def _update_status(self, state: ConnectionState):
        """Update status display."""
        if state == ConnectionState.CONNECTED:
            self.title = f"{self.ICON_CONNECTED} MC"
            self.status_item.title = f"Status: Connected ({self.serial_handler.port_name})"
            self.connect_item.set_callback(None)
            self.disconnect_item.set_callback(self._on_disconnect_click)
        elif state == ConnectionState.CONNECTING:
            self.title = f"{self.ICON_CONNECTING} MC"
            self.status_item.title = "Status: Connecting..."
        elif state == ConnectionState.ERROR:
            self.title = f"{self.ICON_DISCONNECTED} MC"
            self.status_item.title = "Status: Error"
            self.connect_item.set_callback(self._on_connect_click)
            self.disconnect_item.set_callback(None)
        else:
            self.title = f"{self.ICON_DISCONNECTED} MC"
            self.status_item.title = "Status: Disconnected"
            self.connect_item.set_callback(self._on_connect_click)
            self.disconnect_item.set_callback(None)

    # -------------------------------------------------------------------------
    # Callbacks
    # -------------------------------------------------------------------------

    def _on_gesture(self, event: GestureEvent):
        """Handle gesture event from Arduino."""
        if not self.is_enabled:
            return

        self.gesture_count += 1
        self.stats_item.title = f"Gestures: {self.gesture_count}"

        # Execute the mapped action
        self.gesture_executor.handle_gesture(event.gesture, event.confidence)

        # Show notification if enabled
        if self.config.show_notifications and self.config.debug_mode:
            rumps.notification(
                title="Motion Controller",
                subtitle=f"Gesture: {event.gesture}",
                message=f"Confidence: {event.confidence:.0%}"
            )

    def _on_state_change(self, state: ConnectionState):
        """Handle connection state change."""
        self._update_status(state)

        if state == ConnectionState.CONNECTED and self.config.show_notifications:
            rumps.notification(
                title="Motion Controller",
                subtitle="Connected",
                message=f"Arduino connected on {self.serial_handler.port_name}"
            )

    def _on_error(self, message: str):
        """Handle error from serial handler."""
        if self.config.show_notifications:
            rumps.notification(
                title="Motion Controller",
                subtitle="Error",
                message=message
            )

    def _on_message(self, data: dict):
        """Handle generic message from Arduino."""
        if data.get("type") == "ready":
            # Send current settings to Arduino
            self.serial_handler.set_sensitivity(self.config.sensitivity)
            self.serial_handler.set_brightness(self.config.led_brightness)

    def _on_connect_click(self, sender):
        """Handle connect menu item click."""
        self._connect()

    def _on_disconnect_click(self, sender):
        """Handle disconnect menu item click."""
        self.serial_handler.disconnect()

    def _on_enable_toggle(self, sender):
        """Toggle gesture recognition."""
        self.is_enabled = not self.is_enabled
        sender.state = self.is_enabled

    def _on_mapping_change(self, gesture: str, action: str):
        """Handle gesture mapping change."""
        self.config.set_mapping(gesture, action)
        self.config.save()
        # Rebuild mappings menu to show updated state
        self.mappings_menu.clear()
        self._build_mappings_submenu()

    def _on_sensitivity_change(self, value: int):
        """Handle sensitivity change."""
        self.config.sensitivity = value
        self.config.save()
        self.serial_handler.set_sensitivity(value)

    def _on_brightness_change(self, value: int):
        """Handle brightness change."""
        self.config.led_brightness = value
        self.config.save()
        self.serial_handler.set_brightness(value)

    def _on_notifications_toggle(self, sender):
        """Toggle notifications."""
        self.config.show_notifications = not self.config.show_notifications
        self.config.save()
        sender.state = self.config.show_notifications

    def _on_auto_connect_toggle(self, sender):
        """Toggle auto-connect."""
        self.config.auto_connect = not self.config.auto_connect
        self.config.save()
        sender.state = self.config.auto_connect

    def _on_calibrate(self, sender):
        """Start calibration."""
        if self.serial_handler.state != ConnectionState.CONNECTED:
            rumps.notification(
                title="Motion Controller",
                subtitle="Not Connected",
                message="Please connect to Arduino first."
            )
            return

        self.serial_handler.calibrate()
        rumps.notification(
            title="Motion Controller",
            subtitle="Calibrating",
            message="Keep area clear of motion for calibration."
        )

    def _on_debug_toggle(self, sender):
        """Toggle debug mode."""
        self.config.debug_mode = not self.config.debug_mode
        self.config.save()
        sender.state = self.config.debug_mode
        self.serial_handler.set_debug(self.config.debug_mode)

    def _on_quit(self, sender):
        """Handle quit."""
        self.serial_handler.disconnect()
        rumps.quit_application()


def main():
    """Main entry point."""
    app = MotionControllerApp()
    app.run()


if __name__ == "__main__":
    main()
