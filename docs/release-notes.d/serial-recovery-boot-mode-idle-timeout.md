- Added ``CONFIG_BOOT_SERIAL_BOOT_MODE_IDLE_TIMEOUT`` for Zephyr: when
  serial recovery was entered by boot mode, MCUboot reboots into the
  application if no host opens the port, or no valid frame arrives, for
  the configured number of milliseconds. The default, 0, keeps waiting.
