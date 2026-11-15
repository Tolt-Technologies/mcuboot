/* Pin the encryption key just past the vector table, deep inside the SPU-locked range. */
__boot_enc_key_start = .;
KEEP(*(.rodata.enc_priv_key))
__boot_enc_key_end = .;
ASSERT(__boot_enc_key_end > __boot_enc_key_start, "enc_priv_key was not pinned below the SPU lock");
