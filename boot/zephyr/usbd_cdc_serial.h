/* SPDX-License-Identifier: Apache-2.0 */
/*
 * Copyright (c) 2026 Tolt Technologies
 */

#ifndef BOOT_USBD_CDC_SERIAL_H
#define BOOT_USBD_CDC_SERIAL_H

#include <zephyr/kernel.h>

struct usbd_context;

int boot_usb_cdc_serial_init(void);

#if defined(CONFIG_BOOT_SERIAL_CDC_ACM_SN_HOOK)
/* Application hook: write a NUL-terminated ASCII serial into buf, return 0. */
int boot_usb_cdc_serial_sn(char *buf, size_t len);
#endif
struct usbd_context *boot_usb_cdc_serial_get_context(void);
extern struct k_sem boot_cdc_acm_ready;
extern struct k_sem boot_usbd_vbus_ready;

#endif /* BOOT_USBD_CDC_SERIAL_H */
