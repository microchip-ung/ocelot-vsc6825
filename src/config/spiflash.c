//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#include <absacc.h>
#include <stddef.h>
#include <string.h>

#include "common.h"     /* Always include common.h at the first place of user-defined herder files */
#include "vtss_api_base_regs.h"
#include "h2io.h"
#include "misc3.h"
#include "uartdrv.h"
#include "spiflash.h"
#include "timer.h"
#include "print.h"
#include "h2txrx.h" /* using uchar rx_packet[] */
#include "timer.h"
#include "h2mactab.h"

#if defined(NPI_CHIP_PORT) && TRANSIT_UNMANAGED_MAC_OPER_SET
#include "h2mactab.h"
#endif // NPI_CHIP_PORT && TRANSIT_UNMANAGED_MAC_OPER_SET

/*****************************************************************************
 *
 *
 * SPI pins
 *
 *
 *
 ****************************************************************************/
#define SI_CLK 12
#define SI_CS   5
#define SI_DO  10
#define SI_DI   0
#if defined(VTSS_ARCH_OCELOT)
#define SI_CLK_MSK VTSS_M_ICPU_CFG_SPI_MST_SW_MODE_SW_SPI_SCK
#define SI_DO_MSK  VTSS_M_ICPU_CFG_SPI_MST_SW_MODE_SW_SPI_SDO
#define SI_DI_MSK  VTSS_M_ICPU_CFG_SPI_MST_SW_MODE_SW_SPI_SDI
#elif defined(VTSS_ARCH_LUTON26)
#define SI_CLK_MSK VTSS_F_ICPU_CFG_SPI_MST_SW_MODE_SW_SPI_SCK
#define SI_DO_MSK  VTSS_F_ICPU_CFG_SPI_MST_SW_MODE_SW_SPI_SDO
#define SI_DI_MSK  VTSS_F_ICPU_CFG_SPI_MST_SW_MODE_SW_SPI_SDI
#endif
#define SI_CS_MSK  VTSS_F_ICPU_CFG_SPI_MST_SW_MODE_SW_SPI_CS(0x01)

/*****************************************************************************
 *
 *
 * Defines
 *
 *
 *
 ****************************************************************************/
#define FLASH_CFG_SIGNATURE     0x77    // Switch system configuration signature
#define FLASH_RT_MAGIC          0x88    // Switch system run-time code magic no

/* Flash instruction set */
#define FLASH_WREN              0x06        // Write Enable
#define FLASH_WRDI              0x04        // Write Disable
#define FLASH_RDID              0x9F        // Read Identification
#define FLASH_RDSR              0x05        // Read Status Register
#define FLASH_WRSR              0x01        // Write Status Register
#define FLASH_READ              0x03        // Read Data Bytes
#define FLASH_FAST_READ         0x0B        // Read Data Bytes at Higher Speed
#define FLASH_PP                0x02        // Page Program

#define FLASH_PP_MAX_TIMEOUT    2           // We set 20ms here but 7 ms actually
#define FLASH_SE_MAX_TIMEOUT    600         // 6 seconds
#define FLASH_BE_MAX_TIMEOUT    25000       // 250 seconds

//Read status register
#define FLASH_STATUS_WIP        0x1         //SPI read status register WIP bit

// Page size
#define FLASH_PAGE_SIZE         256         // Flash page size

//Voltage level
#define FLASH_VOLTAGE_LOW       0           //Driving voltage low
#define FLASH_VOLTAGE_HIGH      1           //Driving voltage high

// Debug
#define FLASH_DEBUG             0


/*
 Flash layout
 Name               Starting address            Size
=============================================================
 Boot Code          0                           0x10000 (64K bytes)
 CFG                Dynamic                     0x4000  (16K bytes)
 RT0                Dynamic                     0xC000  (48K bytes)
 RT1                Dynamic                     0xC000  (48K bytes)
*/

// Image code size
#define FLASH_IMG_SIZE_BL   0x10000UL   // Boot loader size (64K bytes)
#define FLASH_IMG_SIZE_CFG  0x4000UL    // configuration data size (16K bytes)
#define FLASH_IMG_SIZE_RT   0xC000UL    // Runtime code size (48 bytes)

// Image start address
#define FLASH_IMG_START_ADDR_BL         0x0


/*****************************************************************************
 *
 *
 * Typedefs and enums
 *
 *
 *
 ****************************************************************************/
struct config_contents {
    uchar       rt_magic;       // run-time valid bit
    uchar       rt_idx;         // Runtime code index
    uchar       signature;      // Configuration signature
    mac_addr_t  sys_mac;        // System MAC address
};

struct flash_info {
    ulong   ss;             // Sector Size
    uchar   se;             // Sector Erase
    uchar   be;             // Bulk Erase
    uchar   is_single_img;  // Is Only Single Image Supported?
    uchar   zs_cnt;         // Zero Sector Count
    ushort  zs_mapping[8];  // Zero Sector Mapping
    ulong   sa_cfg;         // Start Address of Configuration
    ulong   sa_rt0;         // Start Address of Runtime Code 0
    ulong   sa_rt1;         // Start Address of Runtime Code 1
};

mac_addr_t xdata mac_addr_0  = {0x00,0x00,0x00,0x00,0x00,0x00};
mac_addr_t xdata mac_addr_ff = {0xff,0xff,0xff,0xff,0xff,0xff};
mac_addr_t spiflash_mac_addr = {0, 0x01, 0xc1, 0, 0, 0};


/*****************************************************************************
 *
 *
 * Local data
 *
 *
 *
 ****************************************************************************/
/* Configuration shadow in RAM */
static xdata struct config_contents config_shadow;
static ulong data simaster_shadow = 0x00;
static xdata struct flash_info cur_flash_info;


/*****************************************************************************
 *
 *
 * Local functions
 *
 *
 *
 ****************************************************************************/
#ifndef NO_DEBUG_IF
#if TRANSIT_UNMANAGED_SWUP
static void write_simaster (ulong value) small {
    simaster_shadow = value;
    H2_WRITE(VTSS_ICPU_CFG_SPI_MST_SW_MODE, simaster_shadow);
}

static void spi_ctrl_enter (void) small {
    write_simaster(0x2A22);
}

static void spi_ctrl_exit (void) small {
    write_simaster(0x0000);
}

static void set_cs (char voltage) small {
    if (voltage == FLASH_VOLTAGE_LOW) {
        write_simaster(simaster_shadow | SI_CS_MSK);
    } else {
        write_simaster(simaster_shadow & (~SI_CS_MSK));
    }
}

static void trigger_clock (void) small {
    write_simaster(simaster_shadow | SI_CLK_MSK);       // driving high
    write_simaster(simaster_shadow & (~SI_CLK_MSK));    // driving low
}

static uchar get_input_byte (void) small {
    ulong value;

    H2_READ(VTSS_ICPU_CFG_SPI_MST_SW_MODE, value);

    return ((value & SI_DI_MSK) != 0);
}

static void set_output_bit (char bit_value) small {
    if (bit_value) {
        write_simaster(simaster_shadow | SI_DO_MSK);
    } else {
        write_simaster(simaster_shadow & (~SI_DO_MSK));
    }
}

static void output_byte (uchar byte_val) small {
    uchar i;

    for (i = 0; i < 8; i++) {
        set_output_bit(byte_val & 0x80);
        byte_val <<= 1;
        trigger_clock();
    }
}

static uchar input_byte (void) small {
    uchar i, byte_val;

    byte_val = 0;
    for (i = 0; i < 8; i++) {
        byte_val <<= 1;
        byte_val |= get_input_byte();
        trigger_clock();
    }

    return byte_val;
}

static void flash_enable_write (void) small {
    set_cs(FLASH_VOLTAGE_LOW);
    output_byte(FLASH_WREN);
    set_cs(FLASH_VOLTAGE_HIGH);
}

static uchar flash_read_status (void) small {
    uchar status;

    set_cs(FLASH_VOLTAGE_LOW);
    output_byte(FLASH_RDSR);
    status = input_byte();
    set_cs(FLASH_VOLTAGE_HIGH);
    return status;
}

static uchar flash_wait_wip (ulong timeout_value) small {
    ushort status, sec = 0, ms = 0;

    if (timeout > MSEC_1000) {
        sec = timeout_value / MSEC_1000 - 1;
        ms  = timeout_value % MSEC_1000;
        start_timer(MSEC_1000);
    } else {
        start_timer(timeout_value);
    }

    do {
        status = flash_read_status() & FLASH_STATUS_WIP;
        if (timeout()) {
            if (sec != 0) {
                sec--;
                start_timer(MSEC_1000);
            } else if (ms != 0) {
                ms = 0;
                start_timer(ms);
            } else {
#if FLASH_DEBUG
                print_str("WIP timout");
                print_cr_lf();
#endif
                return 1;
            }
        }
    } while(status);

    return 0;
}

#if 0
static uchar flash_erase_bulk (void) small {
    uchar status;

    spi_ctrl_enter();
    flash_enable_write();
    set_cs(FLASH_VOLTAGE_LOW);
    output_byte(cur_flash_info.be);
    set_cs(FLASH_VOLTAGE_HIGH);
    status = flash_wait_wip(FLASH_BE_MAX_TIMEOUT);
    spi_ctrl_exit();

    return status;
}
#endif

static uchar flash_erase_sector (ulong addr) small {
    uchar sector_cnt = 1, i , status;

    spi_ctrl_enter();
    if (addr == 0) {
        sector_cnt = cur_flash_info.zs_cnt;
    }
    for (i = 0; i < sector_cnt; i++) {
        flash_enable_write();
        set_cs(FLASH_VOLTAGE_LOW);
        output_byte(cur_flash_info.se);
        output_byte(addr >> 16 & 0xFF);
        output_byte(addr >> 8 & 0xFF);
        output_byte(addr & 0xFF);
        set_cs(FLASH_VOLTAGE_HIGH);
        status = flash_wait_wip(FLASH_SE_MAX_TIMEOUT);
        addr += cur_flash_info.zs_mapping[i];
    }
    spi_ctrl_exit();

    return status;
}

static uchar flash_page_program (ulong addr, uchar *data_ptr, ulong len)
{
    ulong cnt, i;

    while (len > 0) {
        if (len > FLASH_PAGE_SIZE) {
            cnt = FLASH_PAGE_SIZE;
            len -= FLASH_PAGE_SIZE;
        } else {
            cnt = len;
            len = 0;
        }

        spi_ctrl_enter();
        flash_enable_write();
        set_cs(FLASH_VOLTAGE_LOW);
        output_byte(FLASH_PP);
        output_byte(addr >> 16 & 0xFF);
        output_byte(addr >> 8 & 0xFF);
        output_byte(addr & 0xFF);
        for (i = 0; i < cnt; i++) {
            output_byte(*data_ptr++);
        }
        set_cs(FLASH_VOLTAGE_HIGH);
        if (flash_wait_wip(FLASH_PP_MAX_TIMEOUT)) {
            spi_ctrl_exit();
            return 1;
        }
        spi_ctrl_exit();

        addr += FLASH_PAGE_SIZE;
    }

    return 0;
}

#if FLASH_SUPPORT_IMAGE_CHECKSUM
static uchar flash_checksum (ulong start_addr, ulong len)
{
    uchar csum;
    ulong i;

    spi_ctrl_enter();
    set_cs(FLASH_VOLTAGE_LOW);
    output_byte(FLASH_FAST_READ);
    output_byte(start_addr >> 16 & 0xFF);
    output_byte(start_addr >> 8 & 0xFF);
    output_byte(start_addr & 0xFF);
    output_byte(0); // Dummy byte
    for (csum = 0, i = 0; i < len; i++) {
        csum += input_byte();
    }
    set_cs(FLASH_VOLTAGE_HIGH);
    spi_ctrl_exit();

    return csum;
}
#endif /* FLASH_SUPPORT_IMAGE_CHECKSUM */

static uchar flash_change_rt_idx (void)
{
    config_shadow.signature = FLASH_CFG_SIGNATURE;
    config_shadow.rt_idx = config_shadow.rt_idx?0:1;

    flash_erase_sector(cur_flash_info.sa_cfg);
    return flash_page_program(cur_flash_info.sa_cfg, (uchar *) &config_shadow, sizeof(config_shadow));
}

/* ************************************************************************ */
uchar flash_erase_code (uchar img_type)
/* ------------------------------------------------------------------------ --
 * Purpose     : Erase the sectors/block for code
 * Remarks     :
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    ulong i, img_size, addr;

    switch (img_type) {
    case FLASH_IMG_TYPE_BL:
        img_size = FLASH_IMG_SIZE_BL;
        addr     = FLASH_IMG_START_ADDR_BL;
        break;
    case FLASH_IMG_TYPE_RT:
        img_size = FLASH_IMG_SIZE_RT;
        if (cur_flash_info.is_single_img) {
            addr = FLASH_IMG_START_ADDR_BL;
        } else {
            if (config_shadow.rt_idx == 1) {
                addr = cur_flash_info.sa_rt0;
            } else {
                addr = cur_flash_info.sa_rt1;
            }
        }
        break;
    default:
        return 1;
    }

    /* Erase flash sectors */
    for (i = 0; i < ((img_size < cur_flash_info.ss ? cur_flash_info.ss : img_size) / cur_flash_info.ss); i++) {
        if (flash_erase_sector(addr + i * cur_flash_info.ss)) {
            return 1;
        }
    }

    return 0;
}

/* ************************************************************************ */
uchar flash_download_image (uchar img_type, ulong len, uchar csum)
/* ------------------------------------------------------------------------ --
 * Purpose     : Update Flash with image being loaded from serial port
 * Remarks     : Always load from address 0 and flash must have been erased
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    ulong img_size, addr, i, cnt;
    ulong timeout_cnt = 5; // 5 seconds
#if FLASH_SUPPORT_IMAGE_CHECKSUM
    ulong img_addr, img_len;
#endif

    if (len == 0) {
        return 1;
    }

    switch (img_type) {
    case FLASH_IMG_TYPE_BL:
        if (len > FLASH_IMG_SIZE_BL) {
            return 1;
        }
        img_size = FLASH_IMG_SIZE_BL;
        addr     = FLASH_IMG_START_ADDR_BL;
        break;
    case FLASH_IMG_TYPE_RT:
        if (len > FLASH_IMG_SIZE_RT) {
            return 1;
        }
        img_size = FLASH_IMG_SIZE_RT;
        if (cur_flash_info.is_single_img) {
            addr = FLASH_IMG_START_ADDR_BL;
        } else {
            if (config_shadow.rt_idx == 1) {
                addr = cur_flash_info.sa_rt0;
            } else {
                addr = cur_flash_info.sa_rt1;
            }
        }
        break;
    default:
        return 1;
    }

#if FLASH_SUPPORT_IMAGE_CHECKSUM
    img_addr = addr;
    img_len = len;
#endif

    /* Erase flash sectors */
    /* We should call flash_erase_code() first.
    for (i = 0; i < ((img_size < cur_flash_info.ss ? cur_flash_info.ss : img_size) / cur_flash_info.ss); i++) {
        if (flash_erase_sector(addr + i * cur_flash_info.ss)) {
            retrun 1;
        }
    }
    */

    /* Program image */
    while (len > 0) {
        if (len > FLASH_PAGE_SIZE) {
            cnt = FLASH_PAGE_SIZE;
            len -= FLASH_PAGE_SIZE;
        } else {
            cnt = len;
            len = 0;
        }

        spi_ctrl_enter();
        flash_enable_write();
        set_cs(FLASH_VOLTAGE_LOW);
        output_byte(FLASH_PP);
        output_byte(addr >> 16 & 0xFF);
        output_byte(addr >> 8 & 0xFF);
        output_byte(addr & 0xFF);
        for (i = 0; i < cnt; i++) {
            start_timer(MSEC_1000);
            while (! uart_byte_ready()) {
                if (timeout()) {
                    if (timeout_cnt != 0) {
                        timeout_cnt--;
                        start_timer(MSEC_1000);
                    } else {
                        set_cs(FLASH_VOLTAGE_HIGH);
                        spi_ctrl_exit();
#if FLASH_DEBUG
                        print_str("UART timout");
                        print_cr_lf();
#endif
                        return -1;
                    }
                }
            }
            output_byte(uart_get_byte());
        }
        set_cs(FLASH_VOLTAGE_HIGH);
        if (flash_wait_wip(FLASH_PP_MAX_TIMEOUT)) {
            spi_ctrl_exit();
            return 1;
        }
        spi_ctrl_exit();

        addr += FLASH_PAGE_SIZE;
    }

#if FLASH_SUPPORT_IMAGE_CHECKSUM
    /* Check image checksum */
    if (csum != flash_checksum(img_addr, img_len)) {
#if FLASH_DEBUG
        print_str("csum = ");
        print_dec(csum);
        print_cr_lf();
#endif
        return 1;
    }
#else
    csum = 0;
#endif /* FLASH_SUPPORT_IMAGE_CHECKSUM */

    /* Change runtime code index */
    if (!cur_flash_info.is_single_img && img_type == FLASH_IMG_TYPE_RT) {
        return flash_change_rt_idx();
    }

    return 0;
}

#ifndef UNMANAGED_REDUCED_DEBUG_IF
/* ************************************************************************ */
void flash_read_bytes (ulong start_addr, ulong len)
/* ------------------------------------------------------------------------ --
 * Purpose     : Read len bytes from flash starting from start_addr and print
 * Remarks     : them to UART
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    ulong i;

    if (len == 0) {
        return;
    }

    print_cr_lf();
    spi_ctrl_enter();
    set_cs(FLASH_VOLTAGE_LOW);
    output_byte(FLASH_FAST_READ);
    output_byte(start_addr >> 16 & 0xFF);
    output_byte(start_addr >> 8 & 0xFF);
    output_byte(start_addr & 0xFF);
    output_byte(0); // Dummy byte
    for (i = 0; i < len; i++) {
        print_hex_b(input_byte());
        print_spaces(1);
        if ((i & 0x0f) == 0x0f) {
            print_cr_lf(); /* CR/LF every 16 byte */
        }
    }
    set_cs(FLASH_VOLTAGE_HIGH);
    spi_ctrl_exit();
    print_cr_lf();
}
#endif /* UNMANAGED_REDUCED_DEBUG_IF */

/*****************************************************************************
 *
 *
 * Help functions
 *
 *
 *
 ****************************************************************************/

/* ************************************************************************ */
static ulong flash_read_id (void) small
/* ------------------------------------------------------------------------ --
 * Purpose     : Read flash identification
 * Remarks     :
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    ulong id;

    spi_ctrl_enter();
    set_cs(FLASH_VOLTAGE_LOW);
    output_byte(FLASH_RDID);
    id = (ulong) input_byte() << 16;
    if (id == 0x7F0000) { // Specifical for A25L80, the flash ID is 4 byte
        id = id << 8;
        id |= (ulong) input_byte() << 16;
    }
    id |= ((ulong) input_byte() << 8);
    id |= input_byte();
    set_cs(FLASH_VOLTAGE_HIGH);
    spi_ctrl_exit();
    return id;
}

/*
 * Get flash information
 *  return: 0 - success, 1 - fail
 */
static int flash_info_get (struct flash_info *info)
{
    ulong flash_id;
    uchar is_unknown = 0;

    memset(info, 0x0, sizeof(*info));
    flash_id = flash_read_id();
    switch (flash_id) {
    case 0x00C22010:    //MX25L512C
    case 0x00C22011:    //MX25L1005C
    case 0x001C3110:    //EN25F05
    case 0x00373010:    //A25L512
        info->ss            = 0x1000;   // Sector Size (4K Bytes)
        info->se            = 0x20;     // Sector Erase
        info->be            = 0xD8;     // Bulk Erase
        info->is_single_img = 1;        // Is Only Single Image Supported?
        info->zs_cnt        = 1;        // Zero Sector Count
        break;
    case 0x00202011:    //M25P10A
        info->ss            = 0x8000;   // Sector Size (32K Bytes)
        info->se            = 0xD8;     // Sector Erase
        info->be            = 0xC7;     // Bulk Erase
        info->is_single_img = 1;        // Is Only Single Image Supported?
        info->zs_cnt        = 1;        // Zero Sector Count
        break;
    case 0x00C22014:    //MX25L8006E
    case 0x00C22018:    //MX25L12845E
    case 0x00202012:    //M25P20
    case 0x00202014:    //M25P80
        info->ss            = 0x10000;  // Sector Size (64K Bytes)
        info->se            = 0xD8;     // Sector Erase
        info->be            = 0xC7;     // Bulk Erase
        info->is_single_img = 0;        // Is Only Single Image Supported?
        info->zs_cnt        = 1;        // Zero Sector Count
        break;
    case 0x00202018:    //M25P128
        info->ss            = 0x40000;  // Sector Size (256K Bytes)
        info->se            = 0xD8;     // Sector Erase
        info->be            = 0xC7;     // Bulk Erase
        info->is_single_img = 0;        // Is Only Single Image Supported?
        info->zs_cnt        = 1;        // Zero Sector Count
        break;
    case 0x7F372014:    //A25L80
        info->ss            = 0x10000;  // Sector Size (64K Bytes)
        info->se            = 0xD8;     // Sector Erase
        info->be            = 0xC7;     // Bulk Erase
        info->is_single_img = 0;        // Is Only Single Image Supported?
        info->zs_cnt        = 5;        // Zero Sector Count

        info->zs_mapping[0] = 0x1000;   // 0-0,  4K Bytes
        info->zs_mapping[1] = 0x1000;   // 0-1,  4K Bytes
        info->zs_mapping[2] = 0x2000;   // 0-2,  8K Bytes
        info->zs_mapping[3] = 0x4000;   // 0-3, 16K Bytes
        info->zs_mapping[4] = 0x8000;   // 0-4, 32K Bytes
        break;
    case 0x00c22019:    //MX25L25635
    case 0x00C22315:    //MX25V1635F
    case 0x00C22015:    //MX25L1606E
        info->ss            = 0x1000;   // Sector Size (4K Bytes)
        info->se            = 0x20;     // Sector Erase
        info->be            = 0xD8;     // Bulk Erase
        info->is_single_img = 0;        // Is Only Single Image Supported?
        info->zs_cnt        = 1;        // Zero Sector Count
        break;
    default:
        print_str("Flash ID (0x");
        print_hex_dw(flash_id);
        print_str(") not supported");
        print_cr_lf();
        print_str("Assume 2M Bytes, 4K-sector flash is used\r\n");
        info->ss            = 0x1000;   // Sector Size (4K Bytes)
        info->se            = 0x20;     // Sector Erase
        info->be            = 0xD8;     // Bulk Erase
        info->is_single_img = 1;        // Is Only Single Image Supported?
        info->zs_cnt        = 1;        // Zero Sector Count
        is_unknown          = 1;
    }

    if((flash_id == 0x00C22010) ||    //MX25L512C
        (flash_id == 0x00C22011) ||    //MX25L1005C
        (flash_id == 0x001C3110) ||
        (flash_id == 0x00373010)) {
        info->sa_cfg  = 0xC000U;
    } else {
        info->sa_cfg    = info->ss >= FLASH_IMG_SIZE_BL ? info->ss : FLASH_IMG_SIZE_BL;                 // Start Address of Configuration
    }

#if 0//defined(VTSS_ARCH_OCELOT)
	info->ss = 0x1000;
	info->se = 0x20;
	info->be = 0xd8;
	info->zs_cnt = 1;
	info->sa_cfg = 0x1fff000;
#endif


    info->sa_rt0    = info->sa_cfg + (info->ss >= FLASH_IMG_SIZE_CFG ? info->ss : FLASH_IMG_SIZE_CFG);  // Start Address of Runtime Code 0
    info->sa_rt1    = info->sa_rt0 + (info->ss >= FLASH_IMG_SIZE_RT ? info->ss : FLASH_IMG_SIZE_RT);    // Start Address of Runtime Code 1

#if FLASH_DEBUG
    print_str("Flash ID = 0x");
    print_hex_dw(flash_id);
    print_cr_lf();
#endif
    return 0;
}

/*
 * Flash initialization
 *  return: 0 - success, 1 - fail
 */
int flash_init (void)
{
    return flash_info_get(&cur_flash_info);
}
#endif /* TRANSIT_UNMANAGED_SWUP */
#endif /* NO_DEBUG_IF */

/*****************************************************************************
 *
 *
 * Configuration functions
 *
 *
 *
 ****************************************************************************/

#if TRANSIT_SPI_FLASH
/* ************************************************************************ */
uchar flash_program_config (void)
/* ------------------------------------------------------------------------ --
 * Purpose     : Write configuration data into flash; The configuration in flash
 * Remarks     : starts at sector cur_flash_info.sa_cfg
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{

    config_shadow.signature = FLASH_CFG_SIGNATURE;
#if TRANSIT_UNMANAGED_SWUP
    if (flash_erase_sector(cur_flash_info.sa_cfg)) {
        return 1;
    }
    return flash_page_program(cur_flash_info.sa_cfg, (uchar *) &config_shadow, sizeof(config_shadow));
#else
    return TRUE
#endif
}
#endif

/* ************************************************************************ */
void flash_load_config (void)
/* ------------------------------------------------------------------------ --
 * Purpose     : Read configuration and copies into RAM shadow
 * Remarks     :
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{

    uchar xdata *conf_p = (uchar *) &config_shadow;
#if TRANSIT_SPI_FLASH
    ushort len;

    spi_ctrl_enter();
    set_cs(FLASH_VOLTAGE_LOW);
    output_byte(FLASH_FAST_READ);
    output_byte(cur_flash_info.sa_cfg >> 16 & 0xFF);
    output_byte(cur_flash_info.sa_cfg >> 8 & 0xFF);
    output_byte(cur_flash_info.sa_cfg & 0xFF);
    output_byte(0); // Dummy byte
    for (len = 0; len < sizeof(config_shadow); len++) {
        *conf_p++ = input_byte();
    }
    set_cs(FLASH_VOLTAGE_HIGH);
    spi_ctrl_exit();

    if (config_shadow.signature != FLASH_CFG_SIGNATURE) {
        /* Set all configuration to all 0 and use the default MAC address
           when the signature is not valid */
        mac_copy(&config_shadow.sys_mac, spiflash_mac_addr);
    }
#else
    mac_copy(&config_shadow.sys_mac, spiflash_mac_addr);
#endif
}

void flash_read_mac_addr (uchar xdata *mac_addr)
{
    mac_copy(mac_addr, &config_shadow.sys_mac);
}

#if TRANSIT_UNMANAGED_SYS_MAC_CONF
/* Only update RAM copy; call flash_pp_configuration to write into flash */
uchar flash_write_mac_addr (uchar xdata *mac_addr)
{
#if TRANSIT_UNMANAGED_MAC_OPER_SET
    mac_tab_t mac_tab_entry;
#endif // NPI_CHIP_PORT && TRANSIT_UNMANAGED_MAC_OPER_SET

    /* Filter multicast MAC address */
    if (mac_addr[0] & 0x1) {
        return 1;
    }

#if TRANSIT_UNMANAGED_MAC_OPER_SET
    // Delete original MAC entry
    mac_tab_entry.vid = 0;
    mac_tab_entry.port_mask = 1 << CPU_CHIP_PORT;
    mac_copy(mac_tab_entry.mac_addr, &config_shadow.sys_mac);
    h2_mactab_set(&mac_tab_entry, FALSE);

    // Add new MAC entry
    mac_copy(mac_tab_entry.mac_addr, mac_addr);
    h2_mactab_set(&mac_tab_entry, TRUE);
#endif // NPI_CHIP_PORT && TRANSIT_UNMANAGED_MAC_OPER_SET

    /* Update new system MAC address in RAM */
    mac_copy(&config_shadow.sys_mac, mac_addr);

    return 0;
}
#endif
