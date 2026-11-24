/*
 * SPDX-License-Identifier: Apache-2.0
 *
 * Plays the earlier boot stage behind MCUBOOT_ENC_KEY_HOOK: unwraps each
 * image's AES-KW TLV with the simulator's key-encryption key.
 */

#include "mcuboot_config/mcuboot_config.h"

#if defined(MCUBOOT_ENC_KEY_HOOK)
#include <stddef.h>
#include "bootutil/boot_hooks.h"
#include "bootutil/enc_key.h"
#include "bootutil/sign_key.h"
#include "bootutil/crypto/aes_kw.h"

extern const struct bootutil_key bootutil_enc_key;

int
boot_enc_key_hook(int img_index, int slot, const uint8_t *enc_tlv,
                  size_t enc_tlv_len, uint8_t *enckey)
{
    bootutil_aes_kw_context kw;
    int rc;

    (void)img_index;
    (void)slot;

    if (enc_tlv_len != BOOT_ENC_TLV_SIZE) {
        return -1;
    }

    bootutil_aes_kw_init(&kw);
    rc = bootutil_aes_kw_set_unwrap_key(&kw, bootutil_enc_key.key, *bootutil_enc_key.len);
    if (rc == 0) {
        rc = bootutil_aes_kw_unwrap(&kw, enc_tlv, enc_tlv_len, enckey, BOOT_ENC_KEY_SIZE);
    }
    bootutil_aes_kw_drop(&kw);
    return rc;
}
#endif /* MCUBOOT_ENC_KEY_HOOK */
