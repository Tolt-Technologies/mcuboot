// SPDX-License-Identifier: Apache-2.0
/*
 * Copyright (c) 2026 Tolt Technologies
 *
 * Self-contained USB CDC ACM setup for MCUboot serial recovery.
 * Initialized lazily in boot_uart_fifo_init() only when recovery is needed.
 */

#include <zephyr/kernel.h>
#include <zephyr/usb/usbd.h>
#include <zephyr/usb/usbd_msg.h>
#include <zephyr/app_version.h>

#include "bootutil/bootutil_log.h"
#include "usbd_cdc_serial.h"

BOOT_LOG_MODULE_DECLARE(mcuboot);

/* bcdDevice carries the bootloader's major.minor as packed BCD (high byte
 * major, low byte minor), letting a host read the bootloader generation from
 * the recovery USB descriptor with no app and no J-Link. The patch level does
 * not fit BCD and is dropped: v26.5.x -> 0x2605, shown as "26.05" by lsusb. */
#define BCD8(n)              (((((unsigned int)(n)) / 10U) << 4U) | (((unsigned int)(n)) % 10U))
#define BOOT_USBD_BCD_DEVICE ((BCD8(APP_VERSION_MAJOR) << 8U) | BCD8(APP_VERSION_MINOR))
BUILD_ASSERT((APP_VERSION_MAJOR <= 99) && (APP_VERSION_MINOR <= 99),
	     "bcdDevice BCD encoding requires major/minor <= 99");

/* cppcheck-suppress misra-c2012-10.1 ; constant inside USBD_DEVICE_DEFINE */
/* cppcheck-suppress misra-c2012-12.1 ; precedence inside USBD_DEVICE_DEFINE */
/* cppcheck-suppress misra-c2012-12.2 ; shift width inside USBD_DEVICE_DEFINE */
USBD_DEVICE_DEFINE(boot_usbd,
		   DEVICE_DT_GET(DT_NODELABEL(zephyr_udc0)),
		   CONFIG_BOOT_SERIAL_CDC_ACM_VID,
		   CONFIG_BOOT_SERIAL_CDC_ACM_PID);

/* cppcheck-suppress misra-c2012-8.9 ; USBD_DESC_LANG_DEFINE object is collected by the linker */
USBD_DESC_LANG_DEFINE(boot_usbd_lang);
/* cppcheck-suppress misra-c2012-8.9 ; USBD_DESC_MANUFACTURER_DEFINE object is collected by the linker */
/* cppcheck-suppress misra-c2012-10.4 ; constant inside USBD_DESC_MANUFACTURER_DEFINE */
USBD_DESC_MANUFACTURER_DEFINE(boot_usbd_mfr,
			      CONFIG_BOOT_SERIAL_CDC_ACM_MANUFACTURER_STRING);
/* cppcheck-suppress misra-c2012-8.9 ; USBD_DESC_PRODUCT_DEFINE object is collected by the linker */
/* cppcheck-suppress misra-c2012-10.4 ; constant inside USBD_DESC_PRODUCT_DEFINE */
USBD_DESC_PRODUCT_DEFINE(boot_usbd_product,
			 CONFIG_BOOT_SERIAL_CDC_ACM_PRODUCT_STRING);
/* cppcheck-suppress misra-c2012-8.9 ; USBD_DESC_SERIAL_NUMBER_DEFINE object is collected by the linker */
IF_ENABLED(CONFIG_HWINFO, (USBD_DESC_SERIAL_NUMBER_DEFINE(boot_usbd_sn)));

/* Selected by the preprocessor rather than a ternary: the conditional
 * expression mixes essential types and then narrows, and casting a composite
 * to fix that trades one rule for another. */
#if defined(CONFIG_BOOT_SERIAL_CDC_ACM_SELF_POWERED)
static const uint8_t boot_usbd_attributes = USB_SCD_SELF_POWERED;
#else
static const uint8_t boot_usbd_attributes = 0U;
#endif

/* cppcheck-suppress misra-c2012-10.4 ; constant inside USBD_DESC_CONFIG_DEFINE */
USBD_DESC_CONFIG_DEFINE(boot_usbd_fs_cfg_desc, "FS Configuration");
/* cppcheck-suppress misra-c2012-8.9 ; USBD_CONFIGURATION_DEFINE object is collected by the linker */
USBD_CONFIGURATION_DEFINE(boot_usbd_fs_config,
			  boot_usbd_attributes,
			  CONFIG_BOOT_SERIAL_CDC_ACM_MAX_POWER,
			  &boot_usbd_fs_cfg_desc);

#if USBD_SUPPORTS_HIGH_SPEED
USBD_DESC_CONFIG_DEFINE(boot_usbd_hs_cfg_desc, "HS Configuration");
USBD_CONFIGURATION_DEFINE(boot_usbd_hs_config,
			  boot_usbd_attributes,
			  CONFIG_BOOT_SERIAL_CDC_ACM_MAX_POWER,
			  &boot_usbd_hs_cfg_desc);
#endif

/* cppcheck-suppress misra-c2012-9.2 ; K_SEM_DEFINE's initialiser is Zephyr's */
/* cppcheck-suppress misra-c2012-10.4 ; constant inside K_SEM_DEFINE */
/* cppcheck-suppress misra-c2012-12.1 ; precedence inside K_SEM_DEFINE */
K_SEM_DEFINE(boot_cdc_acm_ready, 0, 1);

static void boot_usbd_msg_cb(struct usbd_context *const ctx,
			     const struct usbd_msg *const msg)
{
	ARG_UNUSED(ctx);

	if (msg->type == USBD_MSG_CDC_ACM_CONTROL_LINE_STATE) {
		k_sem_give(&boot_cdc_acm_ready);
	}
}

static int boot_usbd_register_fs(void)
{
	int err;

	err = usbd_add_configuration(&boot_usbd, USBD_SPEED_FS,
				     &boot_usbd_fs_config);
	if (0 != err) {
		BOOT_LOG_ERR("Failed to add FS configuration: %d", err);
		return err;
	}

	err = usbd_register_class(&boot_usbd, "cdc_acm_0", USBD_SPEED_FS, 1);
	if (0 != err) {
		BOOT_LOG_ERR("Failed to register CDC ACM class (FS): %d", err);
		return err;
	}

	err = usbd_device_set_code_triple(&boot_usbd, USBD_SPEED_FS,
					  USB_BCC_MISCELLANEOUS, 0x02, 0x01);
	if (0 != err) {
		BOOT_LOG_ERR("Failed to set code triple (FS): %d", err);
	}

	return err;
}

#if USBD_SUPPORTS_HIGH_SPEED
static int boot_usbd_register_hs(void)
{
	int err;

	err = usbd_add_configuration(&boot_usbd, USBD_SPEED_HS,
				     &boot_usbd_hs_config);
	if (0 != err) {
		BOOT_LOG_ERR("Failed to add HS configuration: %d", err);
		return err;
	}

	err = usbd_register_class(&boot_usbd, "cdc_acm_0", USBD_SPEED_HS, 1);
	if (0 != err) {
		BOOT_LOG_ERR("Failed to register CDC ACM class (HS): %d", err);
		return err;
	}

	err = usbd_device_set_code_triple(&boot_usbd, USBD_SPEED_HS,
					  USB_BCC_MISCELLANEOUS, 0x02, 0x01);
	if (0 != err) {
		BOOT_LOG_ERR("Failed to set code triple (HS): %d", err);
	}

	return err;
}
#endif

int boot_usb_cdc_serial_init(void)
{
	int err;
	unsigned int bcd_device;

	err = usbd_add_descriptor(&boot_usbd, &boot_usbd_lang);
	if (0 != err) {
		BOOT_LOG_ERR("Failed to add language descriptor: %d", err);
		return err;
	}

	err = usbd_add_descriptor(&boot_usbd, &boot_usbd_mfr);
	if (0 != err) {
		BOOT_LOG_ERR("Failed to add manufacturer descriptor: %d", err);
		return err;
	}

	err = usbd_add_descriptor(&boot_usbd, &boot_usbd_product);
	if (0 != err) {
		BOOT_LOG_ERR("Failed to add product descriptor: %d", err);
		return err;
	}

	/* Reserved identifiers below come from Zephyr's logging macros. */
	/* NOLINTNEXTLINE(bugprone-reserved-identifier,cert-dcl37-c,cert-dcl51-cpp) */
	IF_ENABLED(CONFIG_HWINFO, (
		err = usbd_add_descriptor(&boot_usbd, &boot_usbd_sn);
		if (0 != err) {
			BOOT_LOG_ERR("Failed to add serial number descriptor: %d", err);
			return err;
		}
	))

#if USBD_SUPPORTS_HIGH_SPEED
	if (usbd_caps_speed(&boot_usbd) == USBD_SPEED_HS) {
		err = boot_usbd_register_hs();
		if (0 != err) {
			BOOT_LOG_ERR("Failed to register HS configuration: %d", err);
			return err;
		}
	}
#endif

	err = boot_usbd_register_fs();
	if (0 != err) {
		BOOT_LOG_ERR("Failed to register FS configuration: %d", err);
		return err;
	}

	usbd_self_powered(&boot_usbd,
			  boot_usbd_attributes & USB_SCD_SELF_POWERED);

	bcd_device = BOOT_USBD_BCD_DEVICE;
	err = usbd_device_set_bcd_device(&boot_usbd, (uint16_t)bcd_device);
	if (0 != err) {
		BOOT_LOG_ERR("Failed to set bcdDevice: %d", err);
		return err;
	}

	err = usbd_msg_register_cb(&boot_usbd, boot_usbd_msg_cb);
	if (0 != err) {
		BOOT_LOG_ERR("Failed to register message callback: %d", err);
		return err;
	}

	err = usbd_init(&boot_usbd);
	if (0 != err) {
		BOOT_LOG_ERR("Failed to initialize USB device: %d", err);
	}

	return err;
}

struct usbd_context *boot_usb_cdc_serial_get_context(void)
{
	return &boot_usbd;
}
