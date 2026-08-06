/*
 *  Copyright (c) 2022, Laird Connectivity
 *
 *  SPDX-License-Identifier: Apache-2.0
 */

#ifndef H_MCUBOOT_STATUS_
#define H_MCUBOOT_STATUS_

/* Enumeration representing the states that MCUboot can be in */
typedef enum
{
	MCUBOOT_STATUS_STARTUP = 0,
	MCUBOOT_STATUS_UPGRADING,
	MCUBOOT_STATUS_BOOTABLE_IMAGE_FOUND,
	MCUBOOT_STATUS_NO_BOOTABLE_IMAGE_FOUND,
	MCUBOOT_STATUS_BOOT_FAILED,
	MCUBOOT_STATUS_USB_DFU_WAITING,
	MCUBOOT_STATUS_USB_DFU_ENTERED,
	MCUBOOT_STATUS_USB_DFU_TIMED_OUT,
	MCUBOOT_STATUS_SERIAL_DFU_ENTERED,
} mcuboot_status_type_t;

typedef enum {
	MCUBOOT_PROGRESS_SWAP = 0,
	MCUBOOT_PROGRESS_UPLOAD,
} mcuboot_progress_type_t;

#if defined(CONFIG_MCUBOOT_ACTION_HOOKS)
extern void mcuboot_status_change(mcuboot_status_type_t status);
#else
/* Consumes its argument: a caller computing a value only to report it would
 * otherwise trip -Werror=unused-variable wherever the hooks are disabled. */
#define mcuboot_status_change(_status) do { (void)(_status); } while (0)
#endif

#if defined(CONFIG_MCUBOOT_PROGRESS_HOOKS)
extern void mcuboot_progress(mcuboot_progress_type_t type,
			     uint32_t offset, uint32_t total);
#else
#define mcuboot_progress(_type, _offset, _total) \
	do { (void)(_type); (void)(_offset); (void)(_total); } while (0)
#endif

#endif /* H_MCUBOOT_STATUS_ */
