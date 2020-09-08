//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#ifndef __EEP_H__
#define __EEP_H__

#include "common.h"     /* Always include common.h at the first place of user-defined herder files */

#if TRANSIT_LLDP
#include "lldp.h"
#endif

#if TRANSIT_FAN_CONTROL
#include "fan_api.h"
#endif

#if TRANSIT_EEE
#include "eee_api.h"
#endif

#if !TRANSIT_UNMANAGED_SWUP
#if TRANSIT_SPI_FLASH
#error "SPI Flash API is disabled!"
#endif
#endif

/* Flash image type */
#define FLASH_IMG_TYPE_BL   911
#define FLASH_IMG_TYPE_RT   1


/* Support image checksum
 * Note the checksum method is calculate the sum of image and the size is 8 (uchar).
 * for (csum = 0, i = 0; i < data_len; i++) csum += data_content[i];
 */
#define FLASH_SUPPORT_IMAGE_CHECKSUM    1

/* Functions for flash reading/updating */
#ifndef NO_DEBUG_IF

#if TRANSIT_UNMANAGED_SWUP
/*
 * Flash erase
 *  return: 0 - success, 1 - fail
 */
uchar flash_erase_code (uchar img_type);

/*
 * Flash download image
 *  return: 0 - success, 1 - fail
 */
uchar flash_download_image (uchar img_type, ulong len, uchar csum);
#endif /* TRANSIT_UNMANAGED_SWUP */

#ifndef UNMANAGED_REDUCED_DEBUG_IF
uchar flash_checksum_ok (ulong len);
void flash_read_bytes (ulong start_addr, ulong len);
#endif /* UNMANAGED_REDUCED_DEBUG_IF */

#endif /* NO_DEBUG_IF */

uchar flash_program_config (void);
void flash_load_config (void);

/* Functions for updating/reading config at RAM shadow */
void flash_read_mac_addr (uchar xdata *mac_addr);
uchar flash_write_mac_addr (uchar xdata *mac_addr);

/*
 * Flash initialization
 *  return: 0 - success, 1 - fail
 */
int flash_init (void);

extern mac_addr_t xdata mac_addr_0;
extern mac_addr_t xdata mac_addr_ff;
extern mac_addr_t spiflash_mac_addr;

#endif /* __EEP_H__ */

