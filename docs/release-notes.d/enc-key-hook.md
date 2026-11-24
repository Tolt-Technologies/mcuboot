- Added ``boot_enc_key_hook()``, enabled with ``MCUBOOT_ENC_KEY_HOOK``
  (``CONFIG_BOOT_ENC_KEY_HOOK`` on Zephyr), which is passed each encrypted
  image's key TLV and returns its AES key in place of MCUboot decrypting the
  TLV, so that an earlier boot stage can unwrap image keys with a key MCUboot
  never holds.
