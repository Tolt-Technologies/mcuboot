- Added ``CONFIG_BOOT_SERIAL_BOOT_MODE_IDLE_TIMEOUT`` for Zephyr: when
  serial recovery was entered by boot mode, MCUboot reboots into the
  application if no host opens the port, or no valid frame arrives, for
  the configured number of milliseconds. The default, 0, keeps waiting.
  A board can override the weak boot_serial_boot_mode_idle_timeout_ms()
  to choose the timeout per entry.
