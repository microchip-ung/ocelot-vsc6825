//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



//Jams #include <dw8051.h>
#include "common.h"     /* Always include common.h at the first place of user-defined herder files */
#include "vtss_api_base_regs.h"
#include "h2io.h"


#if defined(FERRET_MAIN_ENABLE)

// Debug enable/disable
#define FERRET_MAIN_DEBUG_ENABLE
#if defined(FERRET_MAIN_DEBUG_ENABLE)
#include "print.h"
#endif /* FERRET_MAIN_DEBUG_ENABLE */


/* Include the below two lines to enable debugging on SPI */
//#define DEF_AUTO_RELOCATE_RELEASE_SPI
//#include <dw8051_func.c>

/* Include the below line to include debug functions. */
// #define DEBUG

/* Include the below line to configure the two SFP port for SGMII instead of XAUI */
#define SFP_SGMII

/* VCore system clock frequency (for UART) */
#define VCORE_CLK 250000000

/* UART bit rate */
#define UART_CLK 115200

/* MIIM port map */
/*__*/code u8 miim_ctrl_map[] = {0, 0, 0, 0,  /* Internal PHYs */
                           1, 1, 1, 1}; /* Elise PHY */
/*__*/code u8 miim_port_map[] = {0, 1, 2, 3,  /* Internal PHYs */
                           4, 5, 6, 7}; /* Elise PHY */

/******************************************************************************
 * Common functions
 *****************************************************************************/

u32 reg_rd(u32 reg)
{
#if 1
    return (h2_read(reg));
#else
    /* Read 32bit from the AHB */
    RA_CTRL = 0x02; /* 32b, stall until done */
    RA_AD3 = reg >> 24;
    RA_AD2 = reg >> 16;
    RA_AD1 = reg >> 8;
    RA_AD0_RD = reg; /* This write initiate the AHB access */
    return RA_DA;
#endif
}

void reg_wr(u32 reg, u32 value)
{
#if 1
    h2_write(reg, value);
#else
    /* Write 32bit to AHB */
    RA_DA = value;
    RA_CTRL = 0x02; /* 32b, stall until done */
    RA_AD3 = reg >> 24;
    RA_AD2 = reg >> 16;
    RA_AD1 = reg >> 8;
    RA_AD0_WR = reg; /* This write initiate the AHB access */
#endif
}

void reg_wrm(u32 reg, u32 value, u32 mask)
{
#if 1
    h2_write_masked(reg, value, mask);
#else
    reg_rd(reg);
    RA_DA &= ~mask;
    RA_DA |= value;
    /* Write back AHB access that is currently in SFR */
    RA_AD0_WR = RA_AD0_WR; /* This write initiate the AHB access */
#endif
}

u32 phy_rd(u8 miim, u8 phy, u8 reg)
{
    reg_wr(VTSS_DEVCPU_GCB_MIIM_MII_CMD(miim),
           VTSS_F_DEVCPU_GCB_MIIM_MII_CMD_MIIM_CMD_PHYAD(phy) |
           VTSS_F_DEVCPU_GCB_MIIM_MII_CMD_MIIM_CMD_REGAD(reg) |
           VTSS_F_DEVCPU_GCB_MIIM_MII_CMD_MIIM_CMD_OPR_FIELD(2) | /* Read (clause 22) */
           VTSS_F_DEVCPU_GCB_MIIM_MII_CMD_MIIM_CMD_VLD(1));

    return VTSS_X_DEVCPU_GCB_MIIM_MII_DATA_MIIM_DATA_RDDATA(reg_rd(VTSS_DEVCPU_GCB_MIIM_MII_DATA(miim)));
}

void phy_wr(u8 miim, u8 phy, u8 reg, u32 value)
{
    reg_wr(VTSS_DEVCPU_GCB_MIIM_MII_CMD(miim),
           VTSS_F_DEVCPU_GCB_MIIM_MII_CMD_MIIM_CMD_PHYAD(phy) |
           VTSS_F_DEVCPU_GCB_MIIM_MII_CMD_MIIM_CMD_REGAD(reg) |
           VTSS_F_DEVCPU_GCB_MIIM_MII_CMD_MIIM_CMD_WRDATA(value) |
           VTSS_F_DEVCPU_GCB_MIIM_MII_CMD_MIIM_CMD_OPR_FIELD(1) | /* Write (clause 22) */
           VTSS_F_DEVCPU_GCB_MIIM_MII_CMD_MIIM_CMD_VLD(1));
    sleep_ms(10);
}

void phy_wrm(u8 miim, u8 phy, u8 reg, u32 value, u32 mask)
{
    u32 read = phy_rd(miim, phy, reg);
    read &= ~mask;
    read |= value;
    phy_wr(miim, phy, reg, read);
}

void sleep_ms(u32 ms)
{
    /* Set the timer value (the default timer ticks every 100 us) */
    reg_wr(VTSS_ICPU_CFG_TIMERS_TIMER_VALUE(0), ms * 10);

    /* Enable timer 0 for one-shot */
    reg_wr(VTSS_ICPU_CFG_TIMERS_TIMER_CTRL(0),
           VTSS_F_ICPU_CFG_TIMERS_TIMER_CTRL_ONE_SHOT_ENA(1) |
           VTSS_F_ICPU_CFG_TIMERS_TIMER_CTRL_TIMER_ENA(1));

    /* Wait for timer 0 to reach 0 */
    while (reg_rd(VTSS_ICPU_CFG_TIMERS_TIMER_VALUE(0)));
}

/******************************************************************************
 * Debug functions
 *****************************************************************************/

#ifdef DEBUG

#include <stdio.h>

//#define printf(...) printf_fast(__VA_ARGS__)

void debug_clear_cnt(u8 port)
{
    reg_wrm(VTSS_SYS_SYSTEM_STAT_CFG,
            VTSS_F_SYS_SYSTEM_STAT_CFG_STAT_VIEW(port) |
            VTSS_F_SYS_SYSTEM_STAT_CFG_STAT_CLEAR_SHOT(1),
            VTSS_M_SYS_SYSTEM_STAT_CFG_STAT_VIEW |
            VTSS_M_SYS_SYSTEM_STAT_CFG_STAT_CLEAR_SHOT);
    sleep_ms(1);
}

u32 debug_get_rx_cnt(u8 port)
{
    reg_wrm(VTSS_SYS_SYSTEM_STAT_CFG,
            VTSS_F_SYS_SYSTEM_STAT_CFG_STAT_VIEW(port),
            VTSS_M_SYS_SYSTEM_STAT_CFG_STAT_VIEW);
    return (reg_rd(VTSS_SYS_STAT_CNT(0x01)) +
            reg_rd(VTSS_SYS_STAT_CNT(0x02)) +
            reg_rd(VTSS_SYS_STAT_CNT(0x03)));
}

u32 debug_get_tx_cnt(u8 port)
{
    reg_wrm(VTSS_SYS_SYSTEM_STAT_CFG,
            VTSS_F_SYS_SYSTEM_STAT_CFG_STAT_VIEW(port),
            VTSS_M_SYS_SYSTEM_STAT_CFG_STAT_VIEW);
    return (reg_rd(VTSS_SYS_STAT_CNT(0x41)) +
            reg_rd(VTSS_SYS_STAT_CNT(0x42)) +
            reg_rd(VTSS_SYS_STAT_CNT(0x43)));
}

void debug_uart_init()
{
    /* Set the UART clock  */
    u32 clk_div = ((u32)VCORE_CLK) / (16UL * ((u32)UART_CLK));
    reg_wr(VTSS_UART_UART_LCR(VTSS_TO_UART), VTSS_F_UART_UART_LCR_DLAB(1));
    reg_wr(VTSS_UART_UART_RBR_THR(VTSS_TO_UART), (clk_div & 0xFF));
    reg_wr(VTSS_UART_UART_IER(VTSS_TO_UART), ((clk_div >> 8) & 0xFF));
    reg_wr(VTSS_UART_UART_LCR(VTSS_TO_UART), VTSS_F_UART_UART_LCR_DLAB(0));

    /* Set the data length to 8 */
    reg_wr(VTSS_UART_UART_LCR(VTSS_TO_UART), VTSS_F_UART_UART_LCR_DLS(0x3));

    /* Enable FIFO */
    reg_wr(VTSS_UART_UART_IIR_FCR(VTSS_TO_UART), VTSS_F_UART_UART_IIR_FCR_FIFOE(1));

    /* Enable FIFO threshold mode */
    reg_wr(VTSS_UART_UART_IER(VTSS_TO_UART), VTSS_F_UART_UART_IER_PTIME(1));

    sleep_ms(1);
}

#if 0
void putchar(char c)
{
    /* Wait if FIFO is full */
    while (VTSS_X_UART_UART_LSR_THRE(reg_rd(VTSS_UART_UART_LSR(VTSS_TO_UART))));

    /* Write Tx data */
    reg_wr(VTSS_UART_UART_RBR_THR(VTSS_TO_UART), c);
}

void debug_print_chip_id()
{
    u32 chip_id = reg_rd(VTSS_DEVCPU_GCB_CHIP_REGS_CHIP_ID);
    print_str("%04x Rev. %c",
           (unsigned int)VTSS_X_DEVCPU_GCB_CHIP_REGS_CHIP_ID_PART_ID(chip_id),
           'A' + (char)VTSS_X_DEVCPU_GCB_CHIP_REGS_CHIP_ID_REV_ID(chip_id));
    print_cr_lf();
}
#endif

#else

#define debug_clear_cnt(x)
#define debug_get_rx_cnt(x)
#define debug_get_tx_cnt(x)

#define debug_uart_init()
//#define printf(...)
#define debug_print_chip_id()

#endif

/******************************************************************************
 * Definitions to support the unified tcl environment (UTE) SerDes setup
 *****************************************************************************/

//#define VTSS_E(x) printf("Error: " x)
#define VTSS_E(x) {print_str("Error: "); print_str(x);}
#define CSR_RD(reg, value) (*(value) = reg_rd(reg))
#define CSR_WR(reg, value, mask) reg_wrm(reg, value, mask)
#define VTSS_MSLEEP(ms) sleep_ms(ms)

#if 0
typedef u8 vtss_rc;
#define VTSS_RC_OK    0
#define VTSS_RC_ERROR 1
#endif

/* The following files are generated from the UTE by Alexander Koch. */
#include "ferret_serdes1g_setup.c"
#include "ferret_serdes6g_setup.c"
#include "ferret_serdes6g_ib_cal.c"

/******************************************************************************
 * Functions to initialize the chip
 *****************************************************************************/

void switch_cfg()
{
    /* Initialize memory */
    reg_wr(VTSS_SYS_SYSTEM_RESET_CFG,
           VTSS_F_SYS_SYSTEM_RESET_CFG_MEM_ENA(1) |
           VTSS_F_SYS_SYSTEM_RESET_CFG_MEM_INIT(1));

    /* Initialize VCAPs */
    reg_wr(VTSS_VCAP_CORE_VCAP_CORE_CFG_VCAP_MV_CFG(VTSS_TO_VCAP_ES0),
           VTSS_F_VCAP_CORE_VCAP_CORE_CFG_VCAP_MV_CFG_MV_SIZE(0xffff));
    reg_wr(VTSS_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL(VTSS_TO_VCAP_ES0),
           VTSS_F_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL_UPDATE_CMD(4) |
           VTSS_F_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL_UPDATE_SHOT(1));

    reg_wr(VTSS_VCAP_CORE_VCAP_CORE_CFG_VCAP_MV_CFG(VTSS_TO_VCAP_IS1),
           VTSS_F_VCAP_CORE_VCAP_CORE_CFG_VCAP_MV_CFG_MV_SIZE(0xffff));
    reg_wr(VTSS_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL(VTSS_TO_VCAP_IS1),
           VTSS_F_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL_UPDATE_CMD(4) |
           VTSS_F_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL_UPDATE_SHOT(1));

    reg_wr(VTSS_VCAP_CORE_VCAP_CORE_CFG_VCAP_MV_CFG(VTSS_TO_VCAP_IS2),
           VTSS_F_VCAP_CORE_VCAP_CORE_CFG_VCAP_MV_CFG_MV_SIZE(0xffff));
    reg_wr(VTSS_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL(VTSS_TO_VCAP_IS2),
           VTSS_F_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL_UPDATE_CMD(4) |
           VTSS_F_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL_UPDATE_SHOT(1));

    /* Wait for memory initialization to complete */
    while (reg_rd(VTSS_SYS_SYSTEM_RESET_CFG) & VTSS_F_SYS_SYSTEM_RESET_CFG_MEM_INIT(1));

    /* Enable core */
    reg_wr(VTSS_SYS_SYSTEM_RESET_CFG,
           VTSS_F_SYS_SYSTEM_RESET_CFG_MEM_ENA(1) |
           VTSS_F_SYS_SYSTEM_RESET_CFG_CORE_ENA(1));
}

/* Function to initialize device */
void dev_init(u8 port)
{
    /* Get device address */
    u32 dev_addr = DEV(port);

    /* Enable PCS */
    reg_wr(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_CFG(dev_addr),
           VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_CFG_PCS_ENA(1));

    /* Disable Signal Detect */
    reg_wr(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_SD_CFG(dev_addr), 0);

    /* Enable MAC RX and TX */
    reg_wr(VTSS_DEV_MAC_CFG_STATUS_MAC_ENA_CFG(dev_addr),
           VTSS_F_DEV_MAC_CFG_STATUS_MAC_ENA_CFG_RX_ENA(1) |
           VTSS_F_DEV_MAC_CFG_STATUS_MAC_ENA_CFG_TX_ENA(1));

    /* Clear sgmii_mode_ena */
    reg_wr(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_MODE_CFG(dev_addr), 0);

    /* Clear sw_resolve_ena and set adv_ability to something meaningful just in case */
    reg_wr(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG(dev_addr),
           VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG_ADV_ABILITY(0x0020));

    /* Set RX and TX IFG */
    reg_wr(VTSS_DEV_MAC_CFG_STATUS_MAC_IFG_CFG(dev_addr),
           VTSS_F_DEV_MAC_CFG_STATUS_MAC_IFG_CFG_TX_IFG(5) |
           VTSS_F_DEV_MAC_CFG_STATUS_MAC_IFG_CFG_RX_IFG1(5) |
           VTSS_F_DEV_MAC_CFG_STATUS_MAC_IFG_CFG_RX_IFG2(1));

    /* Set link speed and release all resets */
    reg_wr(VTSS_DEV_PORT_MODE_CLOCK_CFG(dev_addr),
           VTSS_F_DEV_PORT_MODE_CLOCK_CFG_LINK_SPEED(1));

    /* Enable the port in the core */
    reg_wrm(VTSS_QSYS_SYSTEM_SWITCH_PORT_MODE(port),
            VTSS_F_QSYS_SYSTEM_SWITCH_PORT_MODE_PORT_ENA(1),
            VTSS_M_QSYS_SYSTEM_SWITCH_PORT_MODE_PORT_ENA);
}

void int_phy_init(u8 phy_mask)
{
    /* Release common reset */
    reg_wrm(VTSS_DEVCPU_GCB_PHY_PHY_CFG,
            VTSS_F_DEVCPU_GCB_PHY_PHY_CFG_PHY_COMMON_RESET(1),
            VTSS_M_DEVCPU_GCB_PHY_PHY_CFG_PHY_COMMON_RESET);

    /* Wait until SUPERVISOR_COMPLETE */
    while (!VTSS_X_DEVCPU_GCB_PHY_PHY_STAT_SUPERVISOR_COMPLETE(reg_rd(VTSS_DEVCPU_GCB_PHY_PHY_STAT)));

    /* Release individual phy resets and enable phy interfaces */
    reg_wrm(VTSS_DEVCPU_GCB_PHY_PHY_CFG,
            VTSS_F_DEVCPU_GCB_PHY_PHY_CFG_PHY_RESET(phy_mask) |
            VTSS_F_DEVCPU_GCB_PHY_PHY_CFG_PHY_ENA(phy_mask),
            VTSS_M_DEVCPU_GCB_PHY_PHY_CFG_PHY_RESET |
            VTSS_M_DEVCPU_GCB_PHY_PHY_CFG_PHY_ENA);
}

void elise_phy_init(u8 miim, u8 first_port)
{
    /* Configure register 19G for MAC mode */
    phy_wr(miim, first_port, 31, 0x10);
    phy_wrm(miim, first_port, 19, (1UL << 14), (3UL << 14));

    /* Configure register 18G for MAC on all 4 PHYs */
    phy_wr(miim, first_port, 18, 0x80E0);
    while ((phy_rd(miim, first_port, 18) & (1UL << 15)) != 0);

    /* Configure register 23 for MAC and Media mode */
    phy_wr(miim, first_port, 31, 0x00);
    phy_wrm(miim, first_port, 23, 0, (7UL << 8));

    /* Software reset */
    phy_wrm(miim, first_port, 0, (1UL << 15), (1UL << 15));
    while ((phy_rd(miim, first_port, 0) & (1UL << 15)) != 0);

    /* Turn all PHY LEDs off */
    phy_wr(miim, first_port, 29, 15UL | 15UL << 4 | 15UL << 8 | 15UL << 12);

    /* Release coma */
    phy_wr(miim, first_port, 31, 0x10);
    phy_wrm(miim, first_port, 14, 0x0000, 0x3000);

    /* Return to standard page */
    phy_wr(miim, first_port, 31, 0x00);
}

u8 has_link(u8 port)
{
    /* Check PHY link */
    if (port < 8) {
      /* Read PHY link status (register 1 bit 2)
         Link down is latched, so it has to be read twice if link is down in first read */
      return ((phy_rd(miim_ctrl_map[port], miim_port_map[port], 1) & 0x4) ||
              (phy_rd(miim_ctrl_map[port], miim_port_map[port], 1) & 0x4));
    } else {
      /* Check link to SerDes/PHY */
      uchar gpio_value = h2_gpio_read(port == 8 ? GPIO_SFP1_LOS : GPIO_SFP2_LOS);
      return (!gpio_value && VTSS_X_DEV_PCS1G_CFG_STATUS_PCS1G_LINK_STATUS_LINK_STATUS(reg_rd(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_LINK_STATUS(DEV(port)))));
    }
}

/******************************************************************************
 * Main loop
 *****************************************************************************/

void ferret_main(void)
{
    u8 i;

    /* Enable alternate functions (01) for GPIOs 0,1,4,6,7,14,15,16 */
    reg_wr(VTSS_DEVCPU_GCB_GPIO_GPIO_ALT(0), 0x01C0D3);

    /* Enable UART when debugging */
    debug_uart_init();

    /* Debug purpose
    print_str("Booting Ferret0613-2 (");
    //debug_print_chip_id();
    //print_str(")");
    //print_cr_lf();
    */

    /* Toggle DISABLE_FSM to re-trigger the Startup-FSM of the LCPLL */
    reg_wrm(VTSS_HSIO_PLL5G_CFG_PLL5G_CFG2,
            VTSS_F_HSIO_PLL5G_CFG_PLL5G_CFG2_DISABLE_FSM(1),
            VTSS_M_HSIO_PLL5G_CFG_PLL5G_CFG2_DISABLE_FSM);
    reg_wrm(VTSS_HSIO_PLL5G_CFG_PLL5G_CFG2,
            VTSS_F_HSIO_PLL5G_CFG_PLL5G_CFG2_DISABLE_FSM(0),
            VTSS_M_HSIO_PLL5G_CFG_PLL5G_CFG2_DISABLE_FSM);

    vtss_ferret_sd6g_ib_cal();

    /* Configure switch */
    switch_cfg();

    /* Configure SerDes 6G 0 for QSGMII (mask 0x1) */
    vtss_sd6g65_mcb_readback(0x1);
    vtss_ferret_sd6g_cfg_qsgmii(0x1);

#ifdef SFP_SGMII
    /* Debug purpose
    print_str("Configuring SFP ports for SGMII...");
    print_cr_lf();
    */

    /* Configure SerDes 6G 1-2 for SGMII (mask 0x6) */
    vtss_sd6g65_mcb_readback(0x6);
    vtss_ferret_sd6g_cfg_sgmii(0x6);
#else
    print_str("Configuring SFP ports for XAUI...");
    print_cr_lf();
    /* Configure SerDes 6G 1-2 for XAUI (mask 0x6) */
    vtss_sd6g65_mcb_readback(0x6);
    vtss_ferret_sd6g_cfg_xaui(0x6);
#endif


    return; // Peter, SFPs works until now.


    /* Configure internal PHYs */
    int_phy_init(0xF);
#if defined(FERRET_MAIN_DEBUG_ENABLE)
    println_str("Boot completed1");
#endif /* FERRET_MAIN_DEBUG_ENABLE */
    //sleep_ms(1000);

    /* Configure Elise PHYs */
    elise_phy_init(1, 4);
#if defined(FERRET_MAIN_DEBUG_ENABLE)
    println_str("Boot completed 2");
#endif /* FERRET_MAIN_DEBUG_ENABLE */
    //sleep_ms(1000);

    /* Configure devices */
    dev_init(0); dev_init(1); dev_init(2); dev_init(3); /* Internal PHYs (SGMII)  */
    dev_init(4); dev_init(5); dev_init(6); dev_init(7); /* Elise PHY     (QSGMII) */
    dev_init(8); dev_init(10);                          /* SFPs          (SGMII)  */
#if defined(FERRET_MAIN_DEBUG_ENABLE)
    println_str("Boot completed 3");
#endif /* FERRET_MAIN_DEBUG_ENABLE */

    /* Configure device mode for port configuration 0 but without DEV1G_9 */
    reg_wr(VTSS_HSIO_HW_CFGSTAT_HW_CFG,
            VTSS_F_HSIO_HW_CFGSTAT_HW_CFG_QSGMII_ENA(1));
#if defined(FERRET_MAIN_DEBUG_ENABLE)
        println_str("Boot completed 4");
#endif /* FERRET_MAIN_DEBUG_ENABLE */

    /* Configure SGPIO port 11 (System LED) to solid green (10 => green) */
    reg_wrm(VTSS_DEVCPU_GCB_SIO_CTRL_SIO_PORT_CFG(11),
            VTSS_F_DEVCPU_GCB_SIO_CTRL_SIO_PORT_CFG_BIT_SOURCE(1),
            VTSS_M_DEVCPU_GCB_SIO_CTRL_SIO_PORT_CFG_BIT_SOURCE);
#if defined(FERRET_MAIN_DEBUG_ENABLE)
    println_str("Boot completed 5");
#endif /* FERRET_MAIN_DEBUG_ENABLE */

    /* Enable SGPIO ports 0-8,10-11 */
    reg_wr(VTSS_DEVCPU_GCB_SIO_CTRL_SIO_PORT_ENA, 0xDFF);
#if defined(FERRET_MAIN_DEBUG_ENABLE)
    println_str("Boot completed 6");
#endif /* FERRET_MAIN_DEBUG_ENABLE */

    /* Enable 2 SGPIOs per port */
    reg_wrm(VTSS_DEVCPU_GCB_SIO_CTRL_SIO_CFG,
            VTSS_F_DEVCPU_GCB_SIO_CTRL_SIO_CFG_SIO_PORT_WIDTH(1),
            VTSS_M_DEVCPU_GCB_SIO_CTRL_SIO_CFG_SIO_PORT_WIDTH);
#if defined(FERRET_MAIN_DEBUG_ENABLE)
        println_str("Boot completed 7");
#endif /* FERRET_MAIN_DEBUG_ENABLE */

    /* Set blink speed to 2.5Hz */
    reg_wrm(VTSS_DEVCPU_GCB_SIO_CTRL_SIO_CFG,
            VTSS_F_DEVCPU_GCB_SIO_CTRL_SIO_CFG_SIO_BMODE_0(1),
            VTSS_M_DEVCPU_GCB_SIO_CTRL_SIO_CFG_SIO_BMODE_0);
#if defined(FERRET_MAIN_DEBUG_ENABLE)
        println_str("Boot completed 8");
#endif /* FERRET_MAIN_DEBUG_ENABLE */

    /* Enable the SGPIO clock (needs to be between 1MHz and 10MHz) */
    reg_wrm(VTSS_DEVCPU_GCB_SIO_CTRL_SIO_CLOCK,
            VTSS_F_DEVCPU_GCB_SIO_CTRL_SIO_CLOCK_SIO_CLK_FREQ(100),
            VTSS_M_DEVCPU_GCB_SIO_CTRL_SIO_CLOCK_SIO_CLK_FREQ);
#if defined(FERRET_MAIN_DEBUG_ENABLE)
    println_str("Boot completed 9");
#endif /* FERRET_MAIN_DEBUG_ENABLE */

    /* Enable continuous SGPIO bursts */
    reg_wrm(VTSS_DEVCPU_GCB_SIO_CTRL_SIO_CFG,
            VTSS_F_DEVCPU_GCB_SIO_CTRL_SIO_CFG_SIO_AUTO_REPEAT(1),
            VTSS_M_DEVCPU_GCB_SIO_CTRL_SIO_CFG_SIO_AUTO_REPEAT);

#if defined(FERRET_MAIN_DEBUG_ENABLE)
    println_str("Boot completed 10 (End)");
#endif /* FERRET_MAIN_DEBUG_ENABLE */

  return;   // Basic feature works until now.

    while (1) {
      for (i = 0; i < 11; i++) {
        if (i == 9) continue;

        if (has_link(i)) {
          reg_wr(VTSS_DEVCPU_GCB_SIO_CTRL_SIO_PORT_CFG(i),
                 VTSS_F_DEVCPU_GCB_SIO_CTRL_SIO_PORT_CFG_BIT_SOURCE((4 << 3) | 1));
        } else {
          reg_wr(VTSS_DEVCPU_GCB_SIO_CTRL_SIO_PORT_CFG(i),
                 VTSS_F_DEVCPU_GCB_SIO_CTRL_SIO_PORT_CFG_BIT_SOURCE((1 << 3) | 1));
        }
      }
    }
}

/******************************************************************************
 * End of file
 *****************************************************************************/
#endif // FERRET_MAIN_ENABLE
