//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#include "common.h"     /* Always include common.h at the first place of user-defined herder files */

#include "hwport.h"
#include "phydrv.h"
#include "main.h"
#include "vtss_api_base_regs.h"
#include "spiflash.h"
#include "h2gpios.h"
#include "h2io.h"
#include "timer.h"
#include "version.h"

#if defined(HWPORT_DEBUG_ENABLE)
#include "print.h"
#endif /* HWPORT_DEBUG_ENABLE */

/*****************************************************************************
 *
 *
 * Defines
 *
 *
 *
 ****************************************************************************/

/*****************************************************************************
 *
 *
 * Typedefs and enums
 *
 *
 *
 ****************************************************************************/

/*****************************************************************************
 *
 *
 * Prototypes for local functions
 *
 *
 *
 ****************************************************************************/

/*****************************************************************************
 *
 *
 * Public data
 *
 *
 *
 ****************************************************************************/

/*****************************************************************************
 *
 *
 * Local data
 *
 *
 ****************************************************************************/
static code uchar port2ext_tab [NO_OF_CHIP_PORTS] = CPORT_MAPTO_UPORT;
static code uchar port2int_tab [NO_OF_BOARD_PORTS] = UPORT_MAPTO_CPORT;

/*****************************************************************************
 *
 *
 * Public Functions
 *
 *
 ****************************************************************************/

// Chip/Switch port (zero-based) --> User/Front port (one-based)
vtss_uport_no_t cport2uport(vtss_cport_no_t chip_port)
{
    if (chip_port < NO_OF_CHIP_PORTS) { // Avoid memory leak issue
        return port2ext_tab[chip_port];
    }

#if defined(HWPORT_DEBUG_ENABLE)
    print_str("%% Error: Wrong parameter when calling cport2uport(), chip_port=0x");
    print_hex_b(chip_port);
    print_cr_lf();
#endif /* HWPORT_DEBUG_ENABLE */
    return 0xFF; // Failed case
}

// User/Front port (one-based) --> Chip/Switch port (zero-based)
vtss_cport_no_t uport2cport(vtss_uport_no_t uport)
{
    if (uport > 0 && uport <= NO_OF_BOARD_PORTS) { // Avoid memory leak issue
        return port2int_tab[uport - 1];
    }

#if defined(HWPORT_DEBUG_ENABLE)
    print_str("%% Error: Wrong parameter when calling uport2cport(), uport=0x");
    print_hex_b(uport);
    print_cr_lf();
#endif /* HWPORT_DEBUG_ENABLE */
    return 0xFF; // Failed case
}


char get_hw_version(void)
{
    static char hw_ver = '\0'; // Initial value
    /*
     * Important !!!
     *
     * Don't access the switch register via APIs H2_READ()/H2_WRITE()/H2_WRITE_MASKED()
     * before the interrupt mechanism is ready
     */
#if defined(FERRET_F11) || defined(FERRET_F10P)
     /*Power: grid = 0x1d, level = 1, polarity = 1, lane = 2  */
    h2_write(VTSS_DEVCPU_GCB_CHIP_REGS_FEA_DIS, 0x1d<<10 | 0x1<<8 | 0x1<<4 | 0x2);
#else
     /*Power: grid = 0x1d, level = 1, polarity = 1, lane = 1 */
	h2_write(VTSS_DEVCPU_GCB_CHIP_REGS_FEA_DIS, 0x1d<<10 | 0x1<<8 | 0x1<<4 | 0x1);
#endif
    /* Read GPIO_14 value to decide the HW Revision
     *  On current (rev.1) boards there is no external pull on Ferret's MII_MDC output
     * (GPIO_14), hence when used as a GPIO (e.g. before using it as MII_MDC / alternate mode 01)
     * it will read as high due to the internal pullup resistor on the pin.
     *
     * On future (e.g. rev.2) boards I will add a 1K pulldown (R92 in the attached schematics)
     * to GPIO_14, so that it will read as low (until the pin is configured as MII_MDC output,
     * of course).
     */
    if (hw_ver == '\0') { // Only detect in the initial stage
        ulong mask = VTSS_BIT(14);
        ulong orig_reg_val, reg_val;
        ulong busy_loop = 10000; // Use busy loop to avoid timer not ready yet
        orig_reg_val = h2_read(VTSS_DEVCPU_GCB_GPIO_GPIO_ALT(0));
        h2_write(VTSS_DEVCPU_GCB_GPIO_GPIO_ALT(0), 0x0);
        while(busy_loop--);
        reg_val = h2_read(VTSS_DEVCPU_GCB_GPIO_GPIO_IN);
        h2_write(VTSS_DEVCPU_GCB_GPIO_GPIO_ALT(0), orig_reg_val);
        hw_ver = VTSS_BOOL(reg_val & mask) ? '1' : '2';
    }
    return hw_ver;
}


void get_mac_addr (uchar port_no, uchar *mac_addr)
{
    /*
    ** Only one MAC address will be supported in Luton26 so
    ** return this MAC address anyway
    */
    flash_read_mac_addr(mac_addr);

    if (port_no == SYSTEM_MAC_ADDR) {
        return;
    }

    port_no = port_no + 1 - MIN_PORT;

    if ((mac_addr[5] += port_no) < port_no) {
        if (++mac_addr[4] == 0) {
            mac_addr[3]++;
        }
    }
}


void phy_hw_init (void)
{
    ulong cmd;

#if defined(VTSS_ARCH_OCELOT)
    u8 phy_mask = 0xF;

#if defined(FERRET_F11) || defined(FERRET_F10P)
    /* Enable QSGMII mode for devices DEV1G_4, DEV1G_5, DEV1G_6, and DEV1G_7 via SerDes6G_0.
     * (needs to refer to hardware schematic)
     * On FERRET_F11 and FERRET_F10P reference boards, we need to configure QSGMII mode on
     * DEV1G_4, DEV1G_5, DEV1G_6, and DEV1G_7 via SerDes6G_0.
     *
     * Notice that we do the following setup.
     * 1. Setup SerDes6G IB-Calibration in h2_serdes_macro_phase_loop_locked()
     * 2. Setup serdes mode in h2_serdes_macro_config()
     * 3. Enable QSGMII mode on SerDes6G_0 via setting Port Muxing mode
     */
    H2_WRITE_MASKED(VTSS_HSIO_HW_CFGSTAT_HW_CFG,
                    VTSS_F_HSIO_HW_CFGSTAT_HW_CFG_QSGMII_ENA(1),
                    VTSS_M_HSIO_HW_CFGSTAT_HW_CFG_QSGMII_ENA);

    // For FERRET_F11, DEV[9] mapping to SERDES1G_4 (NPI)
    // For FERRET_F10, DEV[9] mapping to SERDES1G_4 (SFP2)
    H2_WRITE_MASKED(VTSS_HSIO_HW_CFGSTAT_HW_CFG,
                    VTSS_F_HSIO_HW_CFGSTAT_HW_CFG_DEV1G_9_MODE(1),
                    VTSS_M_HSIO_HW_CFGSTAT_HW_CFG_DEV1G_9_MODE);
#endif // FERRET_F11 && FERRET_F10P

#if defined(PCIE_CHIP_PORT)
    H2_WRITE_MASKED(VTSS_HSIO_HW_CFGSTAT_HW_CFG,
                    VTSS_F_HSIO_HW_CFGSTAT_HW_CFG_PCIE_ENA(1),
                    VTSS_M_HSIO_HW_CFGSTAT_HW_CFG_PCIE_ENA);
#endif // PCIE_CHIP_PORT

    /* Release common reset */
    H2_WRITE_MASKED(VTSS_DEVCPU_GCB_PHY_PHY_CFG,
                    VTSS_F_DEVCPU_GCB_PHY_PHY_CFG_PHY_COMMON_RESET(1),
                    VTSS_M_DEVCPU_GCB_PHY_PHY_CFG_PHY_COMMON_RESET);

  /* Wait until SUPERVISOR_COMPLETE */
    start_timer(MSEC_2000);
    do {
        H2_READ(VTSS_DEVCPU_GCB_PHY_PHY_STAT, cmd);
    }while (!VTSS_X_DEVCPU_GCB_PHY_PHY_STAT_SUPERVISOR_COMPLETE(cmd) && !timeout());

    if (timeout()) {
#if defined(HWPORT_DEBUG_ENABLE)
        println_str("%% Timeout when calling phy_hw_init()");
#endif // HWPORT_DEBUG_ENABLE
        return;
    }
    
    delay_1(20);
    /* Release individual phy resets and enable phy interfaces */
    H2_WRITE_MASKED(VTSS_DEVCPU_GCB_PHY_PHY_CFG,
                    VTSS_F_DEVCPU_GCB_PHY_PHY_CFG_PHY_RESET(phy_mask) |
                    VTSS_F_DEVCPU_GCB_PHY_PHY_CFG_PHY_ENA(phy_mask),
                    VTSS_M_DEVCPU_GCB_PHY_PHY_CFG_PHY_RESET |
                    VTSS_M_DEVCPU_GCB_PHY_PHY_CFG_PHY_ENA);
    
    H2_READ(VTSS_DEVCPU_GCB_PHY_PHY_CFG, cmd);
    if (cmd != (VTSS_M_DEVCPU_GCB_PHY_PHY_CFG_PHY_RESET |
                VTSS_M_DEVCPU_GCB_PHY_PHY_CFG_PHY_ENA |
                VTSS_M_DEVCPU_GCB_PHY_PHY_CFG_PHY_COMMON_RESET)) {
#if defined(MAIN_DEBUG_ENABLE)
        print_str("Internal PHYs reset failure"); print_hex_dw(cmd); print_cr_lf();
#endif /* MAIN_DEBUG_ENABLE */
    }

#elif defined(VTSS_ARCH_LUTON26)
    /* Release reset of built-in PHYs */
    H2_WRITE_MASKED(VTSS_DEVCPU_GCB_DEVCPU_RST_REGS_SOFT_CHIP_RST, 0x00, \
                    VTSS_F_DEVCPU_GCB_DEVCPU_RST_REGS_SOFT_CHIP_RST_SOFT_PHY_RST);
    do {
        H2_READ(VTSS_DEVCPU_GCB_MISC_MISC_STAT, cmd);
    } while ((cmd & VTSS_F_DEVCPU_GCB_MISC_MISC_STAT_PHY_READY) == 0);

#ifdef LUTON26_L16
#ifdef LUTON26_L16_QSGMII_EXT_PHY
    h2_gpio_mode_set(15, VTSS_GPIO_OUT);
    h2_gpio_write(15, 0);
    delay_1(120); // Datasheet says 120 ms

    h2_gpio_write(15, 1);
    delay_1(120); // Datasheet says 120 ms
#endif // LUTON26_L16_QSGMII_EXT_PHY
#endif // LUTON26_L16
#endif
}

/* GPIO/SGPIO initialization */
void gpio_init(void)
{
    h2_sgpio_enable();
}

/*****************************************************************************
 *                                                                           *
 *  End of file.                                                             *
 *                                                                           *
 *****************************************************************************/

