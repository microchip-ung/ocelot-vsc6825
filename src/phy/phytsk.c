//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#include "common.h"     /* Always include common.h at the first place of user-defined herder files */
#include "event.h"
#include "vtss_api_base_regs.h"
#include "vtss_common_os.h"
#include "h2io.h"
#include "h2.h"
#include "phy_family.h"
#include "phytsk.h"
#include "phymap.h"
#include "phydrv.h"
#include "timer.h"
#include "hwport.h"
#include "h2gpios.h"
#include "h2sdcfg.h"
#include "h2pcs1g.h"
#include "h2txrx.h"
#if FRONT_LED_PRESENT
#include "ledtsk.h"
#endif
#include "misc1.h"
#include "misc2.h"
#if TRANSIT_LLDP
#include "lldp.h"
#endif /* TRANSIT_LLDP */
#if TRANSIT_VERIPHY
#include "veriphy.h"
#endif
#if USE_HW_TWI
#include "i2c_h.h"
#endif
#if USE_SW_TWI
#include "i2c.h"
#endif

#if defined(PHYTSK_DEBUG_ENABLE)
#include "print.h"
#endif /* PHYTSK_DEBUG_ENABLE */

#if TRANSIT_LACP
#include "vtss_lacp.h"
#endif /* TRANSIT_LACP */

/*****************************************************************************
 *
 *
 * Defines
 *
 *
 *
 ****************************************************************************/

/* define states of state machine */
#define PORT_DISABLED                   0
#define PHY_SET_UP_SPEED_MODE           1
#define PHY_WAITING_FOR_LINK            2
#define PHY_LINK_UP                     3
#define SERDES_SIG_SET_UP_MODE         99   // Used for serdes initial state
#define SERDES_SET_UP_MODE            100
#define SERDES_WAITING_FOR_LINK       101
#define SERDES_LINK_UP                102

/* define periods in granularity of 10 msec */
#define POLL_PERIOD_FOR_LINK           10 /* 100 msec */

#define MAX_THERMAL_PROT_TIME 10 /* 10 sec */
#define MAX_JUNCTION_TEMP 122


/* The SFP port link mode is unstable when two Ferret boards are connected.
 * Make a workaround to confirm the link mode again.
 */
#define FERRET_SFP_LM_WORKAROUND

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
 * Local data
 *
 *
 *
 ****************************************************************************/

/* BZ#81401 Work-Around for 100BT Link break Issue, after restoring link it does not come up for a long time. */
#define PHY_WR_PAGE(port_no, page_addr, value) vtss_phy_wr_page(port_no, page_addr, value)
#define PHY_RD_PAGE(port_no, page_addr, value) vtss_phy_rd_page(port_no, page_addr, value)
#define VTSS_PHY_PAGE_TR_16      VTSS_PHY_PAGE_TR, 16
#define VTSS_PHY_PAGE_TR_17      VTSS_PHY_PAGE_TR, 17
#define VTSS_PHY_PAGE_TR_18      VTSS_PHY_PAGE_TR, 18
#define VTSS_PHY_PAGE_TR         0x52B5 /**< Token ring registers */
#define VTSS_PHY_TEST_PAGE_5     VTSS_PHY_TEST_PAGE, 5
#define VTSS_PHY_TEST_PAGE       0x2A30 /**< standard page registers */
#define VTSS_PHY_PAGE_ST0_4      VTSS_PHY_PAGE_ST0, 4
#define VTSS_PHY_PAGE_ST0_9      VTSS_PHY_PAGE_ST0, 9
#define VTSS_PHY_PAGE_ST0        0x0000 /**< standard page registers */
/* Link-up status chip port mask */
static port_bit_mask_t linkup_cport_mask = 0;

/* Port link mode */
static uchar xdata port_lm[NO_OF_CHIP_PORTS];

/* Port state machine */
static uchar xdata port_state_machine[NO_OF_CHIP_PORTS];

/* Port MAC media interface */
#if MAC_TO_MEDIA
static uchar xdata mac_if_changed[NO_OF_CHIP_PORTS];
#endif // MAC_TO_MEDIA

/* Config Flow control mode of each PHY/Serdes port */
#if defined(UNMANAGED_FLOW_CTRL_IF)
static uchar xdata port_fc_config[NO_OF_CHIP_PORTS];
#endif // UNMANAGED_FLOW_CTRL_IF

/* Flag for activate polling of PHYs */
static bit poll_phy_flag = 0;

#if TRANSIT_THERMAL
/* Flag for activate polling of PHYs temperature monitor */
static BOOL start_thermal_protect_timer = FALSE;
static ushort thermal_protect_cnt = 0;
static port_bit_mask_t led_err_stat = 0;
#endif // TRANSIT_THERMAL

/* Let all PHYs initially be powered down/disabled */
static port_bit_mask_t phy_enabled = ALL_PORTS;

#if TRANSIT_THERMAL
static uchar max_protect_temp = MAX_JUNCTION_TEMP;
#endif

#define POLARITY_DETECT_FOR_10HDX_MODE


/****************************************************************************
 *
 * Local functions
 *
 ****************************************************************************/
static vtss_rc vtss_phy_rd_wr_masked(BOOL                 read,
                                     const vtss_port_no_t port_no,
                                     const u32            addr,
                                     u16                  *const value,
                                     const u16            mask)
{
    vtss_rc           rc = VTSS_RC_OK;
    u16               reg, page, val;

    /* Page is encoded in address */
    page = (addr >> 5);
    reg = (addr & 0x1f);

    /* Change page */
    if (page)
        phy_write(port_no, 31, page);

    if (read) {
        /* Read */
        *value = phy_read(port_no, reg);
    } else if (mask != 0xffff) {
        /* Read-modify-write */
        val = phy_read(port_no, reg);
        phy_write(port_no, reg, (val & ~mask) | (*value & mask));
    } else {
        /* Write */
        phy_write(port_no, reg, *value);
    }

    /* Restore standard page */
    if (page)
        phy_write(port_no, 31, 0);

    return rc;
}
vtss_rc vtss_phy_wr_page(const vtss_port_no_t port_no,
                         const u16            page,
                         const u32            addr,
                         const u16            value)
{
    u16 val = value;
    if (page); /*make compiler happy*/
    return vtss_phy_rd_wr_masked(0, port_no, addr, &val, 0xFFFF);
}

// See comment at the do_page_chk
/* Read PHY register (include page) - Use this function in combination with the register defines for new code. */
static vtss_rc vtss_phy_rd_page(const vtss_port_no_t port_no,
                                const u16            page,
                                const u32            addr,
                                u16                  *const value)
{
    if (page); /*make compiler happy*/
    return vtss_phy_rd_wr_masked(1, port_no, addr, value, 0);
}


/**
 * Initialize all PHYs after a power up reset.
 */
static void phy_init (void)
{
    vtss_iport_no_t iport_idx;
    vtss_cport_no_t chip_port;
    u16 reg_val;

    delay(MSEC_30);

#if defined(LUTON26_L25)
    phy_pre_reset(0);
    phy_pre_reset(12);
    if (phy_map(25)) { //using MDC/MDIO
        phy_pre_reset(25); /*L25, port25 uses external single */
    }

#elif defined(LUTON26_L16)
    phy_pre_reset(0);
#ifdef LUTON26_L16_QSGMII_EXT_PHY
    phy_pre_reset(12);
#endif /* LUTON26_L16_QSGMII_EXT_PHY */

#elif defined(LUTON26_L10)
    phy_pre_reset(0);
    if (phy_map(24)) {//using MDC/MDIO
        phy_pre_reset(24); /* L10, port24 uses external single  PHY */
    }
    if (phy_map(25)) {//using MDC/MDIO
        phy_pre_reset(25); /* L10, port25 uses external single PHY*/
    }
#endif // LUTON26_L25


#if defined(LUTON26_L25)
    phy_page_std(12);
#endif

#if defined(FERRET_F11) || defined(FERRET_F10P)|| defined(FERRET_F5) || defined(FERRET_F4P)
    // Reset PHY when using multiple PHY types
    phy_pre_reset(0);

#if defined(FERRET_F11) || defined(FERRET_F10P)
    phy_pre_reset(4);
#endif // FERRET_F11 || FERRET_F10P

#if defined(NPI_CHIP_PORT) && NPI_CHIP_PORT != NPI_ACT_NORMAL_PORT
#if defined(FERRET_F11)
    phy_pre_reset(NPI_CHIP_PORT);
#endif // FERRET_F11
    {
        h2_npi_conf_t npi_conf;

        /* Here's just a sample of NPI port configuraiton
         * Choice the suitable prefix mdoe and queues as the default setting.
         */
        npi_conf.chip_port = NPI_CHIP_PORT;
        npi_conf.mode_enabled = TRUE;
        npi_conf.queue_mask = 0xFF & (~(1 << PACKET_XTR_QU_BPDU_LLDP));
        npi_conf.prefix_header_mode = H2_PREFIX_HEADER_MODE_LONG;
        npi_port_set(&npi_conf);
    }
#endif // NPI_CHIP_PORT && NPI_CHIP_PORT != NPI_ACT_NORMAL_PORT

#endif // FERRET_F11 || FERRET_F10P|| FERRET_F5 || FERRET_F4P

    delay(MSEC_2000); // delay_1(2000);
    phy_page_std(0);
    reg_val = 0;

    for (iport_idx = MIN_PORT; iport_idx < MAX_PORT; iport_idx++) {
        chip_port = iport2cport(iport_idx);

        if (phy_map(chip_port)) {
            if (TEST_PORT_BIT_MASK(chip_port, &phy_enabled)) {
#if defined(FERRET_F11) || defined(FERRET_F10P)|| defined(FERRET_F5) || defined(FERRET_F4P)
                if(iport_idx >= MIN_PORT && iport_idx <= MIN_PORT+3) {
                    /* Work around for Bz#21484 ,applicable for MINI
                     * Default =0; Address 0x07f8;  Field is 2-bits wide: 4:3; So it will be Modifications in the Lower 16 bits, ie. TR_17 and nothing in TR_18,  TR_16=0x87f8 for a Write to addr: 0x07f8*/

                    phy_page_tr(chip_port);
                    phy_write(chip_port,16,0xa7f8);
                    reg_val = phy_read(chip_port,17);
                    reg_val &= 0xffe7;
                    reg_val |= 3<<3;
                    phy_write(chip_port,17,reg_val);
                    phy_write(chip_port,16,0x87f8);
                    /* Work around for Bz#21485 ,applicable for MINI
                     * Default =19; Address 0x0fa4;  Field is 7-bits wide: 22:16; So it will be Modifications in the Upper 16 bits, ie. TR_18 and nothing in TR_17,  TR_16=0x8fa4 for a Write to addr: 0x0fa4
                     * VgaThresh100=24 from default  value 19*/
                    phy_write(chip_port,16,0xafa4);
                    reg_val = phy_read(chip_port,18);
                    reg_val &= 0xff80;
                    reg_val |= 24;
                    phy_write(chip_port,18,reg_val);
                    phy_write(chip_port,16,0x8fa4);
                    phy_page_std(chip_port);
                }
#endif
                phy_setup(chip_port);
                phy_post_reset(chip_port);
            } else {
#if defined(PHYTSK_DEBUG_ENABLE)
                print_str("port in power down state chip_port = ");
                print_dec(chip_port);
                print_cr_lf();
#endif /* PHYTSK_DEBUG_ENABLE */
                phy_power_down(chip_port);
            }
        }
    }

#if VTSS_COBRA && defined(FERRET_F11)
    cobra_power_saving_disable(9);
#endif

#if TRANSIT_FAN_CONTROL || TRANSIT_THERMAL
    phy_init_temp_mode_regs(0);
#endif // TRANSIT_FAN_CONTROL || TRANSIT_THERMAL

#if defined(LUTON26_L25)
#if TRANSIT_FAN_CONTROL || TRANSIT_THERMAL
    phy_init_temp_mode_regs(12);
#endif // TRANSIT_FAN_CONTROL || TRANSIT_THERMAL
#endif
}

static void _sfp_tx_disable_set(vtss_cport_no_t chip_port, BOOL is_disable)
{
#if defined(LUTON26_L25)
    chip_port = chip_port; // Quiet Keil compile warning
    h2_gpio_write(GPIO_SFP_TXDISABLE, is_disable);   // Enable SFP TX

#elif defined(FERRET_F11) || defined(FERRET_F10P)
    if (chip_port == SFP1_CHIP_PORT) {   // SFP1
        h2_gpio_write(GPIO_SFP1_TXDISABLE, is_disable);  // Enable SFP TX
    } else if (chip_port == SFP2_CHIP_PORT) { // SFP2
        h2_gpio_write(GPIO_SFP2_TXDISABLE, is_disable);  // Enable SFP TX
    }

#elif defined(FERRET_F5)
    if (chip_port == SFP2_CHIP_PORT) { // SFP2
        h2_gpio_write(GPIO_SFP2_TXDISABLE, is_disable);  // Enable SFP TX
    }

#else
    // Quiet Keil compile warning
    chip_port = chip_port;
    is_disable = is_disable;
    h2_gpio_write(9, is_disable);  // to avoid compile warning for FERRET_F4P
#endif
}

/**
 * Initialize all SFP gpio controls after a power up reset.
 */
static void sfp_init (void)
{
#if defined(LUTON26_L25)
    phy_page_std(SFP_MODULE_GPIO_PORT);

    /* Configure  PHY GPIO 2, 3 as input, and 4 as output */
    phy_page_gp(SFP_MODULE_GPIO_PORT);

    /* Step 1. setup GPIO 2, 3, 4 is controled by GPIO function */
    phy_write_masked(SFP_MODULE_GPIO_PORT, 13, 0x00fc, 0x00fc);

    /* Step 2, setup input and output pins */
    phy_write_masked(SFP_MODULE_GPIO_PORT, 17, 0x0010, 0x001c);
    phy_page_std(SFP_MODULE_GPIO_PORT);

    /* Use normal GPIO function */
    h2_gpio_mode_set(GPIO_SFP_TXDISABLE, VTSS_GPIO_OUT);
    h2_gpio_write(GPIO_SFP_TXDISABLE, TRUE);    // Disable SFP TX

#elif defined(LUTON26_L10)
    h2_sgpio_write(30, 0, 0); // Set tx_enable
    h2_sgpio_write(31, 0, 0); // Set tx_enable

#elif defined(LUTON26_L16) && defined(LUTON26_L16_QSGMII_EXT_PHY)
    h2_gpio_mode_set(GPIO_SFP_TXDISABLE, VTSS_GPIO_OUT);
    h2_gpio_write(GPIO_SFP_TXDISABLE, FALSE);   // Enable SFP TX
    h2_gpio_write(GPIO_SFP_TXDISABLE, TRUE);    // Disable SFP TX

#elif defined(FERRET_F11) || defined(FERRET_F10P)
    /* BZ#21826 - Drive SFP ports rate select (GPIO 18/19) to high */
    h2_gpio_mode_set(GPIO_SFP1_RATESEL, VTSS_GPIO_OUT);
    h2_gpio_write(GPIO_SFP1_RATESEL, TRUE);
    h2_gpio_mode_set(GPIO_SFP2_RATESEL, VTSS_GPIO_OUT);
    h2_gpio_write(GPIO_SFP2_RATESEL, TRUE);

    /* Set SFP_TXDISABLE GPIO pins as the output mode first,
     * and then disable TX feature in the initial state.
     * It will be enabled when the SFP module present.
     * See phytsk.c/handle_serdes()/case SERDES_SET_UP_MODE
     */
    // SFP1
    h2_gpio_mode_set(GPIO_SFP1_TXDISABLE, VTSS_GPIO_OUT);
    _sfp_tx_disable_set(SFP1_CHIP_PORT, TRUE);   // Disable SFP TX

    // SFP2
    h2_gpio_mode_set(GPIO_SFP2_TXDISABLE, VTSS_GPIO_OUT);
    _sfp_tx_disable_set(SFP2_CHIP_PORT, TRUE);  // Disable SFP TX

#elif defined(FERRET_F5)
    // SFP2
    /* BZ#21826 - Drive SFP ports rate select (GPIO 19) to high */
    h2_gpio_mode_set(GPIO_SFP2_RATESEL, VTSS_GPIO_OUT);
    h2_gpio_write(GPIO_SFP2_RATESEL, TRUE);

    h2_gpio_mode_set(GPIO_SFP2_TXDISABLE, VTSS_GPIO_OUT);
    _sfp_tx_disable_set(SFP2_CHIP_PORT, TRUE);  // Disable SFP TX

#endif
}

static uchar phy_init_state (vtss_cport_no_t chip_port)
{
    if (phy_map(chip_port)) {
        if (TEST_PORT_BIT_MASK(chip_port, &phy_enabled)) {
            phy_write_masked(chip_port, 18, 0x0040, 0x0040);
            return PHY_SET_UP_SPEED_MODE;
        } else {
            return PORT_DISABLED;
        }
    }
#if MAC_TO_MEDIA
    else if (phy_map_serdes(chip_port)) {
        return SERDES_SIG_SET_UP_MODE;
    }
#endif
    else {
        return PORT_DISABLED;
    }
}

/*
** Return link mode
** bit 1:0 = 00:   No link
** bit 1:0 = 01: 1000 Mbit/s
** bit 1:0 = 10:  100 Mbit/s
** bit 1:0 = 11:   10 Mbit/s
**
** bit 4 = 0: half duplex
** bit 4 = 1: full duplex
**
** bit 5 = 0: link partner doesn't support pause frames
** bit 5 = 1: link partner does support pause frames

** bit 7:6 = 00: No EEE advertised
** bit 7:6 = 01: 100Base EEE advertised
** bit 7:6 = 10: 1000Base EEE advertised
** bit 7:6 = 11: Reserved
*/
/* Fixme: Tune the two functions to fit VSC8522/12 */
static uchar phy_link_mode_get(vtss_cport_no_t chip_port)
{
    int eee_advertisement;
    uchar link_mode = LINK_MODE_DOWN;

    if (phy_link_status(chip_port)) {
        link_mode = phy_get_speed_and_fdx(chip_port);

        /* check if link partner supports pause frames */
        if (phy_read(chip_port, 5) & 0x0400) {
            link_mode |= LINK_MODE_PAUSE_MASK;
        }

        // 802.3az says EEE is supported only under full-duplex mode
        if (!(link_mode & LINK_MODE_FDX_MASK))
            return link_mode;

        // Get EEE advertisement
        eee_advertisement = phy_mmd_rd(chip_port, 7, 61);// 7.61 EEE Link Partner Advertisement.

        //Table 96, in data sheet
        if (eee_advertisement & 0x2) {
            link_mode |= LINK_MODE_POWER_MASK_100BASE;
        } else if (eee_advertisement & 0x4) {
            link_mode |= LINK_MODE_POWER_MASK_1000BASE;
        }
    }

    return link_mode;
}

#if MAC_TO_MEDIA
static uchar serdes_link_mode_get(vtss_cport_no_t chip_port)
{
    uchar mac_if;
    uchar lm = LINK_MODE_DOWN;

    if (phy_map_serdes(chip_port)) {
        mac_if = phy_map_miim_no(chip_port);
        if (mac_if == MAC_IF_SERDES_1G || mac_if == MAC_IF_SGMII) {
#ifdef SGMII_SERDES_FORCE_1G_DEBUG_ENABLE
            lm = h2_pcs1g_status_get(chip_port);
#else
            lm = h2_pcs1g_clause_37_status_get(chip_port);
#endif
        } else if (mac_if == MAC_IF_100FX) {
            lm = h2_pcs1g_100fx_status_get(chip_port);
        } else if (mac_if == MAC_IF_SERDES_2_5G) {
            lm = h2_pcs1g_2_5g_link_status_get(chip_port);
        }

#if defined(FERRET_SFP_LM_WORKAROUND) && !defined(SGMII_SERDES_FORCE_1G_DEBUG_ENABLE)
        // Double Confirm
        delay_1(2);
        if (((mac_if == MAC_IF_SERDES_1G || mac_if == MAC_IF_SGMII) && lm != h2_pcs1g_clause_37_status_get(chip_port)) ||
            (mac_if == MAC_IF_100FX && lm != h2_pcs1g_100fx_status_get(chip_port)) ||
            (mac_if == MAC_IF_SERDES_2_5G && lm != h2_pcs1g_2_5g_link_status_get(chip_port))) {
            lm = port_lm[chip_port]; // Keep the original link mode
        }
#endif // FERRET_SFP_LM_WORKAROUND
    }

    return lm;
}
#endif // MAC_TO_MEDIA

static void do_link_up(vtss_cport_no_t chip_port, char link_mode)
{
    WRITE_PORT_BIT_MASK(chip_port, 1, &linkup_cport_mask);

    VTSS_UPDATE_MASKS_DEBUG();
    vtss_update_masks();
    port_lm[chip_port] = link_mode;
    /* Work around for Bz #21483 */
    (void)phy_read(chip_port,10);
    callback_link_up(chip_port);
}

static void do_link_down(vtss_cport_no_t chip_port)
{
    WRITE_PORT_BIT_MASK(chip_port, 0, &linkup_cport_mask);

    if (phy_map(chip_port)) {
        /* Do any set-up of PHY due to link going down */
        phy_do_link_down_settings(chip_port);
    }

    h2_setup_port(chip_port, LINK_MODE_DOWN);
    port_lm[chip_port] = LINK_MODE_DOWN;
    callback_link_down(chip_port);
}

BOOL phy_flowcontrol_get(vtss_cport_no_t chip_port)
{
#if defined(TRANSIT_LACP) && defined(TRANSIT_LACP_FC_OP) // Disalbe FC on LACP-enabled port
    portno = cport2uport(chip_port);
    vtss_lacp_get_portconfig(portno, &cfg);

#if defined(PHYTSK_DEBUG_ENABLE)
    print_str("chip_port: "); print_dec(chip_port);
    print_str("u_port: "); print_dec(portno); print_cr_lf();
#endif //PHYTSK_DEBUG_ENABLE

    if (cfg.enable_lacp == TRUE) {
        // disable flow control to avoid LACP packets stopping by pause
        //frames, which causes LACP not working
        return FALSE;
    } else {
        return TRUE;
    }
    return TRANSIT_FLOW_CTRL_DEFAULT;

#elif defined(UNMANAGED_FLOW_CTRL_IF)
    return (port_fc_config[chip_port] ? TRUE : FALSE);

#else // The same as default
    chip_port = chip_port;  // quiet compiler
    return TRANSIT_FLOW_CTRL_DEFAULT;
#endif // TRANSIT_LACP
}

/**
 * State machine for copper phy. Monitor PHY and set up H2 port.
 */
static void handle_phy (vtss_cport_no_t chip_port)
{
    uchar  link_mode;
    ushort phy_data;
    ushort tr_reg17 = 0;
    ushort tr_reg18 = 0;
    ushort reg4val = 0;
    ushort reg9val = 0;
    ushort tp_reg5 = 0;
    switch (port_state_machine[chip_port]) {
    case PORT_DISABLED: {
        break;
    }

    case PHY_SET_UP_SPEED_MODE: {
        /* Update register 4 with 10/100 advertising, plus pause capability */

    if (phy_flowcontrol_get(chip_port) != TRUE) {
        // disable flow control to avoid LACP packets stopping by pause
        //frames, which causes LACP not working
        phy_data = 0x01e1;
        phy_write(chip_port, 4, phy_data);
    } else {
        phy_data = 0x05e1;
        phy_write(chip_port, 4, phy_data);
    }

#if defined(PHYTSK_DEBUG_ENABLE)
        if (phy_read(chip_port, 4) != phy_data) {
            print_str("%% Flow control setting failed on uport ");
            print_dec_8_right_2(cport2uport(chip_port));
            print_cr_lf();
        }
#endif /* PHYTSK_DEBUG_ENABLE */

        /*  Update register 9 with 1000 Mbps advertising */
        phy_write(chip_port, 9, PHY_REG_9_CONFIG);
        /* Restart auto-negotiation */
        phy_restart_aneg(chip_port);

        port_state_machine[chip_port] = PHY_WAITING_FOR_LINK;    // Change state
        break;
    }

    case PHY_WAITING_FOR_LINK: {
        if (!poll_phy_flag) {
            return;
        }

        /* Check if link is up */
        link_mode = phy_link_mode_get(chip_port);
        if (link_mode != LINK_MODE_DOWN) { // Link-up
#if defined(PHYTSK_DEBUG_ENABLE)
            uchar speed = link_mode & LINK_MODE_SPEED_MASK;

            print_str("PHY Link-Up on uport");
            print_dec_8_right_2(cport2uport(chip_port));
            if (speed == LINK_MODE_SPEED_10) {
                print_str(" - 10M");
            } else if (speed == LINK_MODE_SPEED_100) {
                print_str(" - 100M");
            } else if (speed == LINK_MODE_SPEED_1000) {
                print_str(" - 1G");
            } else if (speed == LINK_MODE_SPEED_2500) {
                print_str(" - 2.5G");
            } else {
                print_str("Unknown speed");
            }
            if (link_mode & LINK_MODE_FDX_MASK) {
                println_str("FDX");
            } else {
                println_str("HDX");
            }
#endif /* PHYTSK_DEBUG_ENABLE */

            /* Update switch chip according to link */
            h2_setup_port(chip_port, link_mode);

            /* Do any set-up of PHY due to link going up */
            phy_do_link_up_settings(chip_port, link_mode, 0);

            do_link_up(chip_port, link_mode);

#ifdef POLARITY_DETECT_FOR_10HDX_MODE
            if ((link_mode & (LINK_MODE_SPEED_MASK | LINK_MODE_FDX_MASK)) == LINK_MODE_HDX_10) {//10HDX
                //println_str("Link is up: checking polarit...");
                phy_data = phy_read(chip_port, 28);
                if (phy_data&0x0C00) { //if POL_INVERSE bits[11:10] is set : polarity swapped
                    phy_write(chip_port, 31, 0x2a30);  // switch to test-register page
                    phy_write_masked(chip_port, 5, 0x6, 0x6);    // write[2:1]={11}: force inversion
                    phy_write(chip_port, 31, 0x0);    // switch to std page
                } else { //bit[11:10] is clear
                    phy_write(chip_port, 31, 0x2a30);  // switch to test-register page
                    phy_write_masked(chip_port, 5, 0x4, 0x6);    // write[2:1]={10}: force normal polarity
                    phy_write(chip_port, 31, 0x0);   // switch to std page
                }

                /* disable polarity detection */
                phy_write_masked(chip_port, 0x12, 0x0010, 0x0010);
            }
#endif //POLARITY_DETECT_FOR_10HDX_MODE

            port_state_machine[chip_port] = PHY_LINK_UP; // Change state
        }
#if 1	/* BZ#81401 Work-Around for 100BT Link break Issue, after restoring link it does not come up for a long time. */
        else {
            phy_data = phy_read(chip_port, 0);
            phy_write(chip_port, 31, VTSS_PHY_TEST_PAGE );
            PHY_RD_PAGE(chip_port, VTSS_PHY_TEST_PAGE_5, &tp_reg5);
            vtss_phy_page_std(chip_port); //page std
            if((!(phy_data&0x1000)) && (phy_data&0x2000) && (tp_reg5 & 0x0800)) { /* If forced, 100BT mode */
                phy_write(chip_port, 31, VTSS_PHY_PAGE_TR );
                PHY_WR_PAGE(chip_port, VTSS_PHY_PAGE_TR_16, 0xaff4);
                PHY_RD_PAGE(chip_port, VTSS_PHY_PAGE_TR_18, &tr_reg18); /* Read bit 7, PreEmphEnabled status  */
                PHY_RD_PAGE(chip_port, VTSS_PHY_PAGE_TR_17, &tr_reg17);
                if (tr_reg18 & 0x0080) { /* Currently Have NO Link and PreEmphEnabledStatus set */
                    vtss_phy_page_std(chip_port); //page std
                    PHY_RD_PAGE(chip_port, VTSS_PHY_PAGE_ST0_4, &reg4val);
                    PHY_RD_PAGE(chip_port, VTSS_PHY_PAGE_ST0_9, &reg9val);
                    PHY_WR_PAGE(chip_port, VTSS_PHY_PAGE_ST0_4, 0x0001);
                    PHY_WR_PAGE(chip_port, VTSS_PHY_PAGE_ST0_9, 0x0000);
                    phy_write_masked(chip_port, 0, 0x1000, 0x1000);
                    phy_write_masked(chip_port, 0, 0x0000, 0x1000);
                    PHY_WR_PAGE(chip_port, VTSS_PHY_PAGE_ST0_4, reg4val);
                    PHY_WR_PAGE(chip_port, VTSS_PHY_PAGE_ST0_9, reg9val);
                }
            }
        }
#endif
        break;
    }

    case PHY_LINK_UP: {
        if (poll_phy_flag) {
            /* Check if link has been down or link mode is changed */
            link_mode = phy_link_mode_get(chip_port);

            if (link_mode == LINK_MODE_DOWN || link_mode != port_lm[chip_port]) {
                do_link_down(chip_port);
#ifdef POLARITY_DETECT_FOR_10HDX_MODE
#if defined(PHYTSK_DEBUG_ENABLE)
                print_str("PHY Link-Down on uport");
                print_dec_8_right_2(cport2uport(chip_port));
                print_cr_lf();
#endif /* PHYTSK_DEBUG_ENABLE */
                phy_write(chip_port, 31, 0x2a30);  // switch to test-register page
                phy_write_masked(chip_port, 5, 0x0, 0x6);    // write[2:1]={00}:Unforce polarity
                phy_write(chip_port, 31, 0x0); // switch to std page

                /* enable polarity detection */
                phy_write_masked(chip_port, 0x12, 0x0000, 0x0010);
#endif // POLARITY_DETECT_FOR_10HDX_MODE

                port_state_machine[chip_port] = PHY_WAITING_FOR_LINK;    // Change state
           }
        }
        break;
    }

    default:
        break;
    }
}

#if  MAC_TO_MEDIA
#define SFP_SONET_COMPLIANCE_CODE               0x04    // SONET Compliance Codes
/* byte     Bit     Ethernet Compliance Codes
 *    4       5     OC-192, short reach2 8 3 Active Cable 8
 *    4       4     SONET reach specifier bit 1 8 2 Passive Cable 8
 *    4       3     SONET reach specifier bit 2 Unallocated
 *    4       2     OC-48, long reach 2 8 1 Unallocated
 *    4       1     OC-48, intermediate reach
 *    5       7     Unallocated
 *    5       6     OC-12, single mode, long reach
 *    5       5     OC-12, single mode, inter. reach
 *    5       4     OC-12, short reach
 *    5       3     Unallocated
 *    5       2     OC-3, single mode, long reach
 *    5       1     OC-3, single mode, inter. reach
 *    5       0     OC-3, short reach
 */
#define SFP_ETHERNET_COMPLIANCE_CODE            0x06    // Ethernet Compliance Codes

/* byte     Bit     Fiber Channel Transmission Media
 *    9       7     Twin Axial Pair (TW)
 *    9       6     Twisted Pair (TP)
 *    9       5     Miniature Coax (MI)
 *    9       4     Video Coax (TV)
 *    9       3     Multimode, 62.5um (M6)
 *    9       2     Multimode, 50um (M5, M5E)
 *    9       1     Unallocated
 *    9       0     Single Mode (SM)
 */
#define SFP_FIBER_CHANNEL_TRANSMISSION_CODE     0x09    // Fiber Channel Transmission Media


#if TRANSIT_SFP_DETECT
#if USE_SW_TWI
static uchar i2c_read(uchar addr, uchar reg)
{
    uchar ack;
    uchar temp;

    i2c_start();
    ack = i2c_send_byte(addr << 1);
    ack = i2c_send_byte(reg & 0xff);

    i2c_start();
    ack = i2c_send_byte((addr << 1) | 1);

    /* read LS byte */
    temp = i2c_get_byte(0);

    i2c_stop();

    return temp;
}
#endif /* USE_SW_TWI */

/* Enable I2C access and perform a read */
static uchar sfp_i2c_read(uchar dev, vtss_port_no_t port_no, uchar addr, uchar *const value, uchar cnt)
{
    uchar ret;
    uchar i2c_scl_gpio;
#if USE_SW_TWI
    uchar c;
#endif
    if (!phy_map_serdes(port_no))
        return 0;
#if USE_HW_TWI
    i2c_scl_gpio = phy_map_phy_no(port_no);
    i2c_tx(i2c_scl_gpio, dev, &addr, 1);
    //delay(MSEC_20);
    ret = i2c_rx(i2c_scl_gpio, dev, value, cnt);
#endif
#if USE_SW_TWI
    ret = 0;
    for (c = 0; c < cnt; c++) {
        *(value + c) = i2c_read(dev, (addr+c));
        if (*(value + c) != 0xff) ret |= 1;
    }
#if 0
    for (c = 0; c < cnt; c++) {
        print_hex_b(*(value + c));
        print_ch(' ');
    }
    print_str(", ret = ");
    print_dec(ret);
    print_cr_lf();
#endif
#endif
    return ret;
}

#if defined(FERRET_F11) || defined(FERRET_F10P) || defined(FERRET_F5) || defined(FERRET_F4P)
// No support copper SFP modules in Ferret
#define CuSFP_DET   0
#else
#define CuSFP_DET   0 /* 1 */    /* Not implemented yet */
#endif

// Read SFP EEPROM information(MSA) via I2C bus
static uchar sfp_detect(vtss_cport_no_t chip_port)
{
#if TRANSIT_LLDP || LOOPBACK_TEST
    uchar *buf = &rx_packet[0];
#else
    uchar buf[30];
#endif

#if CuSFP_DET
{
    ulong vendor;
    const char *CuSFPModel[2] = { "AXGT-R1T", "AXGT-R15" };

    /* I2C slave address 0x50 is the SFP EEPORM address and 0x56 is the SFP copper phy */
    if (sfp_i2c_read(0x50, chip_port, 12, &buf[0], 1)) {//MSA EERPOM byte[12], rate in unit of 100Mbits/s
        //print_str("sfp rate:");
        //print_dec(buf[0]);   print_cr_lf();
        if (buf[0] >= 1 && buf[0] <= 2) {        /* 100Mb capabilities --> 100FX */
            return MAC_IF_100FX;
        } else if (buf[0] >= 25) { /* 2500Mb capabilities */
            //print_str(">2.5G SFP");
            //print_cr_lf();
            return MAC_IF_SERDES_2_5G; /* Currently default for 2.5G operation */
        }
        else if (buf[0] >= 10 && buf[0] <= 22) { /* 1Gb capabilities -->  Copper SFP or Serdes 1000Base-X */
            if (sfp_i2c_read(0x50, chip_port, 40, &buf[0], 8)) {   /* Try to recognize the SFP module via EEPROM */
                buf[8] = '\0';
                if (xmemcmp((char *)CuSFPModel[0],(char*)buf, 8) == 0) {        /* Axcen SFP_CU_SGMII */
                    return MAC_IF_SGMII;
                } else if (xmemcmp((char *)CuSFPModel[1], (char*)buf, 8) == 0) { /* Axcen SFP_CU_SERDES */
                    return MAC_IF_SERDES_1G;
                }
            }

            /* Vender is unknown, then read PHY registers */
            /* Try to determine the SFP host mode (serdes/sgmii) via the Phy settings */
            if (sfp_i2c_read(0x56, chip_port, 0, &buf[0], 8)) {
                /* The Phy is there, find the vendor id and if the phy is in SGMII or Serdes mode */
                vendor = (((buf[4] << 8) | buf[5]) << 6) | ((((buf[6] << 8) | buf[7]) >> 10) & 0x3f);
                if (vendor == 0x3f1) { /* VTSS phy in SERDES mode */
                    return MAC_IF_SERDES_1G;
                } else if (vendor == 0x5043) { /* Marvell phy */
                    if (sfp_i2c_read(0x56, chip_port, 0, &buf[0], 22) &&
                            sfp_i2c_read(0x56, chip_port, 22, &buf[0], 22)) {
                        if (buf[20] == 0x11) {    /* SGMII interface to host */
                            return MAC_IF_SGMII;
                        } else {
                            return MAC_IF_SERDES_1G;
                        }
                    }
                }
                return MAC_IF_SERDES_1G; /* PHY vendor is unknown */
            } else {
                return MAC_IF_SERDES_1G; /* No phy --> SERDES SFP */
            }
        } else {
            if (sfp_i2c_read(0x50, chip_port, SFP_ETHERNET_COMPLIANCE_CODE, &buf[0], 1)) {
                if ((buf[0] & 0xf) == 0)
                    return MAC_IF_100FX;
            }
        }
    } else {
#if USE_HW_TWI
#if 0 // Ferret, TODO. Missing first parameter
        i2c_tx(0, &buf[0], 1);
        i2c_rx(0, &buf[0], 1);
        delay(MSEC_20);
#endif // Ferret. TODO.
#endif // USE_HW_TWI
    }
    return MAC_IF_SERDES_1G;
}

#else   // CuSFP_DET = 0
{
    if (sfp_i2c_read(0x50, chip_port, SFP_ETHERNET_COMPLIANCE_CODE, &buf[0], 1)) {

        if (&buf[0] == 0xff) {
#if USE_HW_TWI
#if 0 // Ferret, TODO. Missing first parameter
            /* workaround of h2 twi function */

            uchar value=0;
            i2c_tx(0, &value, 1);
            i2c_rx(0, &value, 1);
            delay(MSEC_20);
#endif // Ferret. TODO.
#endif // USE_HW_TWI
            sfp_i2c_read(0x50, chip_port, SFP_ETHERNET_COMPLIANCE_CODE, &buf[0], 1);
        }

#if defined(PHYTSK_DEBUG_ENABLE)
        print_str("Ethenet Compliance codes(MSA Byte 6):0x");
        print_hex_b(buf[0]);
#endif /* PHYTSK_DEBUG_ENABLE */

        if ((buf[0] & 0x1) != 0) {
#if defined(PHYTSK_DEBUG_ENABLE)
            print_str("   1000BASE-SX"); print_cr_lf();
#endif /* PHYTSK_DEBUG_ENABLE */
            return MAC_IF_SERDES_1G;
        } else if ((buf[0] & 0x2) != 0) {
#if defined(PHYTSK_DEBUG_ENABLE)
            print_str("   1000BASE-LX"); print_cr_lf();
#endif /* PHYTSK_DEBUG_ENABLE */
            return MAC_IF_SERDES_1G;
        } else if ((buf[0] & 0x4) != 0) {
#if defined(PHYTSK_DEBUG_ENABLE)
            print_str("   1000BASE-CX"); print_cr_lf();
#endif /* PHYTSK_DEBUG_ENABLE */
            return MAC_IF_SERDES_1G;
        } else if ((buf[0] & 0x8) != 0) {
#if defined(PHYTSK_DEBUG_ENABLE)
            print_str("   1000BASE-T"); print_cr_lf();
#endif /* PHYTSK_DEBUG_ENABLE */
            return MAC_IF_SERDES_1G;
        } else if ((buf[0] & 0x10) != 0) {
#if defined(PHYTSK_DEBUG_ENABLE)
            print_str("   100BASE-LX"); print_cr_lf();
#endif /* PHYTSK_DEBUG_ENABLE */
            return MAC_IF_100FX;
        } else if ((buf[0] & 0x20) != 0) {
#if defined(PHYTSK_DEBUG_ENABLE)
            print_str("   100BASE-FX"); print_cr_lf();
#endif /* PHYTSK_DEBUG_ENABLE */
            return MAC_IF_100FX;
        }  else {
#if defined(PHYTSK_DEBUG_ENABLE)
            print_str("   MAC_IF_SERDES_2_5G(non-standard)");
#endif /* PHYTSK_DEBUG_ENABLE */
            return MAC_IF_SERDES_2_5G;
        }
    }

    return MAC_IF_SERDES_1G; /* Use MAC_IF_SERDES_1G as the default interface when I2C reading failed */
}
#endif  // CuSFP_DET
}
#endif /* TRANSIT_SFP_DETECT */

/* ************************************************************************ */
static uchar serdes_port_sfp_detect(vtss_cport_no_t chip_port)
/* ------------------------------------------------------------------------ --
 * Purpose     : Detect if SFP module present
 * Remarks     :
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    uchar gpio_value = 0;

    /* do signal detect */
    if (phy_map_serdes(chip_port)) {
#if defined(LUTON26_L25)
        ushort reg15g;

        phy_page_gp(SFP_MODULE_GPIO_PORT);
        reg15g = phy_read(SFP_MODULE_GPIO_PORT, 15);
        phy_page_std(SFP_MODULE_GPIO_PORT);
        gpio_value = test_bit_16(1, &reg15g);

#elif defined(LUTON26_L10)
        switch(chip_port) {
        case 24:
            gpio_value = h2_sgpio_read(26, 1);  //sfp0;
            break;
        case 25:
            gpio_value = h2_sgpio_read(27, 1);  //sfp1;
            break;
        default:
            break;
        }

#elif defined(FERRET_F11) || defined(FERRET_F10P)
        switch(chip_port) {
        case SFP1_CHIP_PORT:
            gpio_value = h2_gpio_read(GPIO_SFP1_PRESENT);  //sfp1;
            break;
        case SFP2_CHIP_PORT:
            gpio_value = h2_gpio_read(GPIO_SFP2_PRESENT);  //sfp2;
            break;
        default:
            break;
        }

#elif defined(FERRET_F5)
        switch(chip_port) {
        case SFP2_CHIP_PORT:
            gpio_value = h2_gpio_read(GPIO_SFP2_PRESENT);  //sfp2;
            break;
        default:
            break;
        }
#else
        gpio_value = h2_gpio_read(9);  //to avoid compiler warning for FERRET_F4P
#endif
    }

    return gpio_value ? FALSE : TRUE; // 0: not present, 1: present
}

/* ************************************************************************ */
static void handle_serdes(vtss_cport_no_t chip_port)
/* ------------------------------------------------------------------------ --
 * Purpose     : State machine for Serdes port. Monitor and set up switch port.
 * Remarks     :
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    uchar mac_if, lm = LINK_MODE_DOWN, sfp_existed, speed;

    switch (port_state_machine[chip_port]) {
    case PORT_DISABLED: {
        break;
    }

    case SERDES_SIG_SET_UP_MODE: {
        // Do nothing here, pass to 'SERDES_SET_UP_MODE' state
        port_state_machine[chip_port] = SERDES_SET_UP_MODE;  // Change state
        break;
    }

    case SERDES_SET_UP_MODE: {
        /* Detect if SFP module present or not.
         * Notice that the PCS clock need to be enabled first.
         */
        _sfp_tx_disable_set(chip_port, TRUE);  // Disable SFP TX
        h2_pcs1g_clock_set(chip_port, TRUE);    // Enable PCS clock
#ifdef SGMII_SERDES_FORCE_1G_DEBUG_ENABLE
        sfp_existed = 0; //No need to detect speed since 1G forced
#else
        sfp_existed = serdes_port_sfp_detect(chip_port);
#endif
#if 0 // To test serdes_port_sfp_detect function and i2c function
{
        static uchar debug_sfp_existed[2] = {0, 0};
        if (chip_port == SFP1_CHIP_PORT || chip_port == SFP2_CHIP_PORT) {
            if (debug_sfp_existed[chip_port == SFP1_CHIP_PORT ? 0 : 1] != sfp_existed) {
                debug_sfp_existed[chip_port == SFP1_CHIP_PORT ? 0 : 1] = sfp_existed;
                print_str("chip_port=");
                print_dec(chip_port);
                print_str(", sfp_existed=");
                print_dec(sfp_existed);
                print_cr_lf();

                if (sfp_existed) {
                    delay_1(250); // BZ#20186, delay for 250 mseconds
                    mac_if = sfp_detect(chip_port);
                } else {
                    mac_if = MAC_IF_UNKNOWN;
                }
                print_str(", mac_if=");
                print_dec(mac_if);
                print_cr_lf();
            }
        }
}
#endif // #if 0

#if TRANSIT_SFP_DETECT
        // Read SFP EEPROM information(MSA) via I2C bus
        if (sfp_existed) {
            /* BZ#20186, delay for 250 mseconds for some SFP modules needing
                some time to get i2c interface ready */
            delay_1(250);
            mac_if = sfp_detect(chip_port);
        } else {
            phy_map_serdes_if_restore(chip_port);
            return; // Keep the original state
        }

        if (mac_if != phy_map_miim_no(chip_port)) {
            phy_map_serdes_if_update(chip_port, mac_if);
            mac_if_changed[chip_port] = 1;
        }
#else
        mac_if = phy_map_miim_no(chip_port);
#endif // TRANSIT_SFP_DETECT

        if (mac_if_changed[chip_port]) {
            vtss_serdes_mode_t media_if;
#if defined(LUTON26_L25) || defined(LUTON26_L10) || defined(FERRET_F11) || defined(FERRET_F10P) || defined(FERRET_F5) || defined(FERRET_F4P)
            ulong sd1g_addr = 0xFF; // None SERDES1G
            ulong sd6g_addr = 0xFF; // None SERDES6G
#endif
            mac_if_changed[chip_port] = 0;

#if defined(LUTON26_L25)
            sd1g_addr = 0x1;
#elif defined(LUTON26_L10)
            switch(chip_port) {
            case 24:
                sd1g_addr = 0x2;
                break;
            case 25:
                sd1g_addr = 0x1;
                break;
            }

#elif defined(FERRET_F11)
            switch(chip_port) {
            case SFP1_CHIP_PORT:
                sd6g_addr = 0x2; // DEV[8] is mapping to SERDES6G_1
                break;
            case SFP2_CHIP_PORT:
                sd6g_addr = 0x4; // DEV[10] is mapping to SERDES6G_2
                break;
            }

#elif defined(FERRET_F10P)
            switch(chip_port) {
            case SFP1_CHIP_PORT:
                sd6g_addr = 0x2; // DEV[8] is mapping to SERDES6G_1
                break;
            case SFP2_CHIP_PORT:
                sd1g_addr = 0x10; // DEV[9] is mapping to SERDES1G_4
                break;
            }


#elif defined(FERRET_F5)
            switch(chip_port) {
            case SFP2_CHIP_PORT:
                sd6g_addr = 0x4; // DEV[10] is mapping to SERDES6G_2
                break;
            }
#endif // LUTON26_L25

            switch(mac_if) {
            case MAC_IF_SERDES_2_5G:
                media_if = VTSS_SERDES_MODE_2G5;
                break;
            case MAC_IF_SERDES_1G:
                media_if = VTSS_SERDES_MODE_1000BaseX;
                break;
            case MAC_IF_SGMII:
                media_if = VTSS_SERDES_MODE_SGMII;
                break;
            case MAC_IF_100FX:
                media_if = VTSS_SERDES_MODE_100FX;
                break;
            default:
                media_if = VTSS_SERDES_MODE_1000BaseX;
            }

            if (sd6g_addr != 0xFF) { // SERDES6G_X
                h2_sd6g_cfg_change(media_if, sd6g_addr);
            }
            if (sd1g_addr != 0xFF) { // SERDES1G_X
                h2_sd1g_cfg(media_if, sd1g_addr);
            }

            /* Set PCS1G configuration */
            if (mac_if == MAC_IF_SERDES_1G || mac_if == MAC_IF_SERDES_2_5G || mac_if == MAC_IF_SGMII || mac_if == MAC_IF_100FX) {
                h2_pcs1g_setup(chip_port, mac_if);
            }

            if (mac_if == MAC_IF_SERDES_1G || mac_if == MAC_IF_SERDES_2_5G || mac_if == MAC_IF_SGMII || mac_if == MAC_IF_100FX) {
                h2_pcs1g_clause_37_control_set(chip_port, phy_flowcontrol_get(chip_port));
            } else {
                /* 100 Full mode and Auto SFP mode, do nothing */
            }

            h2_setup_port(chip_port, LINK_MODE_DOWN);
        }

        port_state_machine[chip_port] = SERDES_WAITING_FOR_LINK; // Change state

        break;
    }

    case SERDES_WAITING_FOR_LINK: {
        if (poll_phy_flag) {
#if defined(FERRET_SFP_LM_WORKAROUND)
             uchar lm_same_cnt = 0, lm_retry_cnt = 0;
#endif // FERRET_SFP_LM_WORKAROUND

            _sfp_tx_disable_set(chip_port, FALSE);  // Enable SFP TX
#if TRANSIT_SFP_DETECT
            sfp_existed = serdes_port_sfp_detect(chip_port);  // Detect if SFP module present or not.
            if (!sfp_existed) {
                phy_map_serdes_if_restore(chip_port);

#if defined(PHYTSK_DEBUG_ENABLE)
                print_str("SFP module is unplugged on uport");
                print_dec_8_right_2(cport2uport(chip_port));
                print_cr_lf();
#endif /* PHYTSK_DEBUG_ENABLE */

                port_state_machine[chip_port] = SERDES_SET_UP_MODE;  // Change state
                break;
            }
#endif // TRANSIT_SFP_DETECT

            mac_if = phy_map_miim_no(chip_port);
            lm = serdes_link_mode_get(chip_port);
            speed = lm & LINK_MODE_SPEED_MASK;

            if (lm != LINK_MODE_DOWN) {  /* Link up */
                if (mac_if == MAC_IF_SGMII || mac_if == MAC_IF_SERDES_2_5G) {
                    ulong tgt = VTSS_TO_DEV(chip_port);

#if defined(VTSS_ARCH_LUTON26)
                    if ((lm & LINK_MODE_SPEED_AND_FDX_MASK) == LINK_MODE_FDX_1000) {
                        H2_WRITE_MASKED(VTSS_DEV_CMN_MAC_CFG_STATUS_MAC_MODE_CFG(tgt), 0x00000011, 0x00000011);
                    } else {
                        if (lm & LINK_MODE_FDX_MASK) {
                            H2_WRITE_MASKED(VTSS_DEV_CMN_MAC_CFG_STATUS_MAC_MODE_CFG(tgt), 0x00000001, 0x00000011);
                        } else {
                            H2_WRITE_MASKED(VTSS_DEV_CMN_MAC_CFG_STATUS_MAC_MODE_CFG(tgt), 0x00000000, 0x00000011);
                        }
                    }

#elif defined(FERRET_F11) || defined(FERRET_F10P) || defined(FERRET_F5) || defined(FERRET_F4P)
                    BOOL giga_mode_ena = (lm & LINK_MODE_SPEED_MASK) >= LINK_MODE_SPEED_1000 ? 1 : 0;
                    H2_WRITE_MASKED(VTSS_DEV_MAC_CFG_STATUS_MAC_MODE_CFG(tgt),
                                    VTSS_F_DEV_MAC_CFG_STATUS_MAC_MODE_CFG_GIGA_MODE_ENA(giga_mode_ena) |
                                    VTSS_F_DEV_MAC_CFG_STATUS_MAC_MODE_CFG_FDX_ENA(lm & LINK_MODE_FDX_MASK ? 1 : 0),
                                    VTSS_M_DEV_MAC_CFG_STATUS_MAC_MODE_CFG_GIGA_MODE_ENA |
                                    VTSS_M_DEV_MAC_CFG_STATUS_MAC_MODE_CFG_FDX_ENA);
#endif
                }

                h2_setup_port(chip_port, lm);

#if defined(FERRET_SFP_LM_WORKAROUND)
                do {
                    delay_1(2);
                    if (lm == serdes_link_mode_get(chip_port)) {
                        lm_same_cnt++;
                    }
                } while (lm_retry_cnt++ < 2);

                if (lm_same_cnt == lm_retry_cnt)
#endif // FERRET_SFP_LM_WORKAROUND
                {
#if defined(PHYTSK_DEBUG_ENABLE)
                    print_str("SFP Link-Up on uport");
                    print_dec_8_right_2(cport2uport(chip_port));
                    if (speed == LINK_MODE_SPEED_10) {
                        print_str(" - 10M");
                    } else if (speed == LINK_MODE_SPEED_100) {
                        print_str(" - 100M");
                    } else if (speed == LINK_MODE_SPEED_1000) {
                        print_str(" - 1G");
                    } else if (speed == LINK_MODE_SPEED_2500) {
                        print_str(" - 2.5G");
                    } else {
                        print_str("Unknown speed");
                    }
                    if (lm & LINK_MODE_FDX_MASK) {
                        println_str("FDX");
                    } else {
                        println_str("HDX");
                    }
#endif /* PHYTSK_DEBUG_ENABLE */
                    do_link_up(chip_port, lm);
                    port_state_machine[chip_port] = SERDES_LINK_UP;  // Change state
                }
            }

        }
        break;
    }

    case SERDES_LINK_UP: {
        if (poll_phy_flag) {
#if TRANSIT_SFP_DETECT
            sfp_existed = serdes_port_sfp_detect(chip_port);  // Detect if SFP module present or not.
            if (!sfp_existed) {
                phy_map_serdes_if_restore(chip_port);
                do_link_down(chip_port);

#if defined(PHYTSK_DEBUG_ENABLE)
                print_str("SFP module is unplugged on uport");
                print_dec_8_right_2(cport2uport(chip_port));
                print_cr_lf();
#endif /* PHYTSK_DEBUG_ENABLE */

                port_state_machine[chip_port] = SERDES_SET_UP_MODE;  // Change state
                break;
            }
#endif // TRANSIT_SFP_DETECT

            /* Check if link has been down or link mode is changed */
            lm = serdes_link_mode_get(chip_port);
            if (lm == serdes_link_mode_get(chip_port) &&
                (lm == LINK_MODE_DOWN || lm != port_lm[chip_port])) {
#if defined(PHYTSK_DEBUG_ENABLE)
                print_str("SFP Link-Down on uport");
                print_dec_8_right_2(cport2uport(chip_port));
                print_cr_lf();
#endif /* PHYTSK_DEBUG_ENABLE */

                do_link_down(chip_port);
                port_state_machine[chip_port] = SERDES_WAITING_FOR_LINK; // Change state
            }
        }
        break;
    }

    default:
        break;
    }
}
#endif /* MAC_TO_MEDIA */


/****************************************************************************
 *
 * Public functions
 *
 ****************************************************************************/


void phy_timer_10 (void)
{
    static uchar poll_phy_timer = 0;

    if (++poll_phy_timer >= 10) {
        poll_phy_timer = 0;
        poll_phy_flag = 1;
    }

}


uchar phy_check_all (void)
{
    vtss_iport_no_t iport_idx;
    vtss_cport_no_t chip_port;
    uchar           error = 0;

    for (iport_idx = MIN_PORT; iport_idx < MAX_PORT; iport_idx++) {
        chip_port = iport2cport(iport_idx);
        if (phy_map(chip_port)) {
#if PHY_ID_CHECK
            if (phy_read(chip_port, 2) != PHY_OUI_MSB) {
                error = 1;
                break;
            }
#endif
        }
    }

    return error;
}


#if LOOPBACK_TEST
void phy_restart (vtss_cport_no_t chip_port)
{
    if (phy_map(chip_port)) {
        phy_reset(chip_port);
        port_state_machine[chip_port] = PHY_SET_UP_SPEED_MODE;  // Change state
    }
#if MAC_TO_MEDIA
    else if (phy_map_serdes(chip_port)) {
        port_state_machine[chip_port] = SERDES_SET_UP_MODE;   // Change state
    }
#endif
    do_link_down(chip_port);
}
#endif /* LOOPBACK_TEST */


uchar phy_tsk_init (void)
{
    vtss_iport_no_t iport_idx;
    vtss_cport_no_t chip_port;

    // Local database initialization
    linkup_cport_mask = 0;

    for (iport_idx = MIN_PORT; iport_idx < MAX_PORT; iport_idx++) {
        chip_port = iport2cport(iport_idx);
        port_lm[chip_port] = LINK_MODE_DOWN;
        port_state_machine[chip_port] = phy_init_state(chip_port);   // State initialization

#if defined(UNMANAGED_FLOW_CTRL_IF)
        port_fc_config[chip_port] = TRANSIT_FLOW_CTRL_DEFAULT;
#endif // UNMANAGED_FLOW_CTRL_IF

        if (phy_map(chip_port)) {
            mac_if_changed[chip_port] = 0;
#if MAC_TO_MEDIA
        } else if (phy_map_serdes(chip_port)) {
            mac_if_changed[chip_port] = 1;
#endif
        }

    }

    phy_init(); // Phy Init must come after the h2_init_port, because the port clocks must be enabled.
    sfp_init();

    if (phy_check_all()) {
        return 1; // Failed
    }

    return 0; // OK
}

#if defined(UNMANAGED_FLOW_CTRL_IF)
void   phy_state_to_setup (vtss_cport_no_t chip_port)
{
    // Move port state back to either PHY_SET_UP_SPEED_MODE or SERDES_SET_UP_MODE
    if (phy_map(chip_port)) {
        if (port_state_machine[chip_port] == PHY_LINK_UP) {
            do_link_down(chip_port);

            phy_write(chip_port, 31, 0x2a30);  // switch to test-register page
            phy_write_masked(chip_port, 5, 0x0, 0x6);    // write[2:1]={00}:Unforce polarity
            phy_write(chip_port, 31, 0x0); // switch to std page

            /* enable polarity detection */
            phy_write_masked(chip_port, 0x12, 0x0000, 0x0010);
        }
        port_state_machine[chip_port] = PHY_SET_UP_SPEED_MODE;
    }
#if MAC_TO_MEDIA
     else if (phy_map_serdes(chip_port)) {
        port_state_machine[chip_port] = SERDES_SET_UP_MODE;
    }
#endif
}

void phy_flowcontrol_set(vtss_cport_no_t chip_port, BOOL mode_enabled)
{
    port_fc_config[chip_port] = mode_enabled;
}
#endif // UNMANAGED_FLOW_CTRL_IF

void phy_tsk (void)
{
    vtss_iport_no_t iport_idx;
    vtss_cport_no_t chip_port;

    for (iport_idx = MIN_PORT; iport_idx < MAX_PORT; iport_idx++) {
        chip_port = iport2cport(iport_idx);
        if (phy_map(chip_port)) {
            handle_phy(chip_port);
        }
#if MAC_TO_MEDIA
        else if (phy_map_serdes(chip_port)) {
            handle_serdes(chip_port);
        }
#endif
    }

    poll_phy_flag = 0;
}

uchar port_link_mode_get(vtss_cport_no_t chip_port)
{
    return port_lm[chip_port];
}

BOOL is_port_link(vtss_cport_no_t chip_port)
{
    if (TEST_PORT_BIT_MASK(chip_port, &linkup_cport_mask)) {
        return TRUE;
    }

    return FALSE;
}

#if TRANSIT_LAG || TRANSIT_LOOPDETECT
port_bit_mask_t linkup_cport_mask_get(void)
{
    return linkup_cport_mask;
}
#endif

#if TRANSIT_VERIPHY

/****************************************************************************
 *
 *
 * VeriPHY
 *
 *
 ****************************************************************************/

/**
 * Run veriphy on all ports.
 *
 * @todo Run only on PHY ports.
 */
void phy_veriphy_all (void)
{
    uchar                   j;
    uchar                   errors;
    /* veriphy_parms_t xdata veriphy_parms [NO_OF_CHIP_PORTS]; */
    veriphy_parms_t xdata   *veriphy_parms = (veriphy_parms_t *) rx_packet;
    BOOL                    all_done = FALSE;
    BOOL                    done;
    ushort                  timeout = 1500;
    port_bit_mask_t         iport_mask;
    vtss_iport_no_t         iport_idx;
    vtss_cport_no_t         chip_port;

    // Fill iport_mask
    for (iport_idx = MIN_PORT; iport_idx < MAX_PORT; iport_idx++) {
        WRITE_PORT_BIT_MASK(iport, 1, &iport_mask);
    }

    // Start VeriPhy
    for (iport_idx = MIN_PORT; iport_idx < MAX_PORT; iport_idx++) {
        chip_port = iport2cport(iport_idx);
        if (TEST_PORT_BIT_MASK(iport_idx, &iport_mask) && phy_map(chip_port)) {
            /* Read PHY id to determine action */
            veriphy_start(chip_port); // Starting veriphy for selected port(s)
        }
    }

    // Pulling Verphy until Veriphy is done
    for (iport_idx = MIN_PORT; iport_idx < MAX_PORT; iport_idx++) {
        chip_port = iport2cport(iport_idx);
        /* Different procedure ???
          The API in clihnd.c/cmd_run_veriphy() do the following
          phy_page_std(chip_port);
        */
        if (TEST_PORT_BIT_MASK(iport_idx, &iport_mask) && phy_map(chip_port)) {
            done = FALSE;
            while (!done) {
                veriphy_run(chip_port, (veriphy_parms + chip_port), &done);
            }
        }
    }

    // Check if any error occurred
    errors = 0;
    for (iport_idx = MIN_PORT; iport_idx < MAX_PORT; iport_idx++) {
        chip_port = iport2cport(iport_idx);
        if (TEST_PORT_BIT_MASK(iport_idx, &iport_mask) && phy_map(chip_port)) {
            /* Status is valid */
            if (!(veriphy_parms + chip_port)->flags) {
                errors = 1;
            }
            /* Status for each pair */
            for (j = 0; j < 4; j++) {
                if ((veriphy_parms + chip_port)->stat[j]) {
                    /* Set Error Leds */
                    errors = 1;
                }
            }
#if FRONT_LED_PRESENT
            if (errors) {
                led_port_event_set(iport_idx, VTSS_LED_EVENT_VERIPHY_ERR, VTSS_LED_MODE_BLINK_YELLOW);
            }
#endif // FRONT_LED_PRESENT
        }
    }

#if FRONT_LED_PRESENT
    if (errors) {
        delay_1(3000); // Show error state for 3 seconds
        for (iport_idx = MIN_PORT; iport_idx < MAX_PORT; iport_idx++) {
            chip_port = iport2cport(port_no);

            if (TEST_PORT_BIT_MASK(iport_idx, &iport_mask) && phy_map(chip_port)) {
                led_port_event_set(iport_idx, VTSS_LED_EVENT_VERIPHY_ERR, VTSS_LED_MODE_NORMAL);
            }
        }
    }
#endif // FRONT_LED_PRESENT
}
#endif /* TRANSIT_VERIPHY */

#if TRANSIT_ACTIPHY
/* Set ActiPHY mode operation */
void phy_actiphy_set(BOOL mode_enabled)
{
    vtss_iport_no_t iport_idx;
    vtss_cport_no_t chip_port;

    for (iport_idx = MIN_PORT; iport_idx < MAX_PORT; iport_idx++) {
        chip_port = iport2cport(iport_idx);
        if (phy_map(chip_port)) {
            phy_page_tp(chip_port);
            phy_write_masked(chip_port, 12, 0x0000, 0xfc00);
            phy_write_masked(chip_port, 24, 0x2000, 0x2000);
            phy_page_std(chip_port);
            phy_write_masked(chip_port, 28, mode_enabled ? 0x0040 : 0x0000, 0x0040);
            delay_1(1);
        }
    }
}
#endif /* TRANSIT_ACTIPHY */


#if TRANSIT_FAN_CONTROL || TRANSIT_THERMAL
ushort phy_get_sys_temp (void)
{
    ushort temp_0 = 0, temp_1 = 0;
    temp_0 = phy_read_temp_reg(0);
#if defined(LUTON26_L25)
    temp_1 = phy_read_temp_reg(12);
#else
    temp_1 = 0;
#endif

    return MAX(temp_1, temp_0);
}
#endif


#if TRANSIT_THERMAL
void phy_handle_temperature_protect()
{
    ushort temperatue, temp;
    uchar  port_ext;
    uchar  port_no;
    uchar  temp_id;

    uchar enable;

    const uchar code protect_port[] = {3, 7, 13, 19, 23, 26};
    const uchar code protect_temp[] = {0, 1, 2,  3,  4,  5}; /* Threshhold is 122C ~ 128C*/

    enable = 1;

    temperatue = phy_get_sys_temp();

    temp_id = 0xff;
    if (temperatue >= max_protect_temp) {  /* Temp. over threshold */
        start_thermal_protect_timer = TRUE;  /* Start the protectioin timer */
        thermal_protect_cnt = MAX_THERMAL_PROT_TIME; /* Refresh the timer value to default value 10 sec */

        temp = temperatue - max_protect_temp; /* Find the power down ports */
        for (temp_id = 0; temp_id < ARRAY_LENGTH(protect_temp) - 1; temp_id++) {
            if (temp >= protect_temp[temp_id] && temp < protect_temp[temp_id + 1])
                break;
        }

#if defined(PHYTSK_DEBUG_ENABLE)
        print_str("temp. is ");
        print_dec(temperatue);
        print_cr_lf();
#endif /* PHYTSK_DEBUG_ENABLE */

#if 0
        print_str("cur temp. diff = ");
        print_dec(temp);
        print_ch('(');
        print_dec(protect_temp[temp_id]);
        print_str("), power down ports = ");
        print_dec(protect_port[temp_id]);
        print_cr_lf();
#endif
    }

    if (temp_id != 0xff && temp_id < ARRAY_LENGTH(protect_temp)) {
        for (port_ext = 1; port_ext <= NO_OF_BOARD_PORTS; port_ext++) {
            uchar half = protect_port[temp_id] / 2;
            uchar half_port = (MAX_PORT + MIN_PORT) / 2;
            port_no = uport2cport(port_ext);
            if (phy_map(port_no)
#if MAC_TO_MEDIA
                    || phy_map_serdes(port_no)
#endif
              ) {
                if (port_ext < half_port) { /* power down ports */
                    if (port_ext <= half) {
                        if (!TEST_PORT_BIT_MASK(port_no, &led_err_stat)) {
                            if (enable) {
                                phy_set_enable(port_no, FALSE);
                            }
                            WRITE_PORT_BIT_MASK(port_no, 1, &led_err_stat);
#if FRONT_LED_PRESENT
                            led_state_set(port_ext, VTSS_LED_EVENT_PHY_OVERHEAT, VTSS_LED_MODE_BLINK_YELLOW);
#endif
                        }
                    }
                } else {
                    if (port_ext < (half_port + (protect_port[temp_id] - half))) {
                        if (!TEST_PORT_BIT_MASK(port_no, &led_err_stat)) {
                            if (enable) {
                                phy_set_enable(port_no, FALSE);
                            }
                            WRITE_PORT_BIT_MASK(port_no, 1, &led_err_stat);
#if FRONT_LED_PRESENT
                            led_state_set(port_ext, VTSS_LED_EVENT_PHY_OVERHEAT, VTSS_LED_MODE_BLINK_YELLOW);
#endif
                        }
                    }
                }
            }
        }
    }

    if (!start_thermal_protect_timer) { /* timer is stopped. */
        for (port_ext = 1; port_ext <= NO_OF_BOARD_PORTS; port_ext++) {
            port_no = uport2cport(port_ext);
            if (phy_map(port_no)
#if MAC_TO_MEDIA
                    || phy_map_serdes(port_no)
#endif
              ) {
                if (TEST_PORT_BIT_MASK(port_no, &led_err_stat)) { /* power on the ports */
                    if (enable) {
                        phy_set_enable(port_no, TRUE);
                    }
                    WRITE_PORT_BIT_MASK(port_no, 0, &led_err_stat);
#if FRONT_LED_PRESENT
                    led_state_set(port_ext, VTSS_LED_EVENT_PHY_OVERHEAT, VTSS_LED_MODE_NORMAL);
#endif
                }
            }
        }
    }

#if 0
    if (led_err_stat) {
        print_str("over heat led stat ");
        print_hex_prefix();
        print_hex_dw(led_err_stat);
        print_cr_lf();
    }
#endif
}


void phy_temperature_timer_1sec (void)
{
    if (start_thermal_protect_timer)
    {
        thermal_protect_cnt--;

        if (thermal_protect_cnt == 0)
        {
            start_thermal_protect_timer = FALSE;
        }
    }
}


#endif /* TRANSIT_THERMAL */


#if TRANSIT_THERMAL
void phy_set_enable (vtss_cport_no_t chip_port, uchar status)
{
    if (phy_map(chip_port) && TEST_PORT_BIT_MASK(chip_port, &phy_enabled) != status) {
        WRITE_PORT_BIT_MASK(chip_port, status, &phy_enabled);

        if (status) {
            phy_write_masked(chip_port, 0, 0, 0x800);
            phy_restart(chip_port);
        } else {
#if TRANSIT_LLDP
            /* make sure to transmit shutdown LLDPDU before link goes away */
            lldp_pre_port_disabled(cport2uport(chip_port));
#endif /* TRANSIT_LLDP */
            do_link_down(chip_port);
            if (phy_map(chip_port)) {
                phy_power_down(chip_port);
            }
#if  MAC_TO_MEDIA
#if defined(VTSS_ARCH_LUTON26)
            else if (phy_map_serdes(chip_port)) {
                uchar mac_if = phy_map_miim_no(chip_port);
                if (mac_if == MAC_IF_SERDES_1G || mac_if == MAC_IF_SGMII) {
                    h2_pcs1g_clock_set(chip_port, FALSE); // Disable PCS clock
                }
            }
#elif defined(FERRET_F11) || defined(FERRET_F10P) || defined(FERRET_F5) || defined(FERRET_F4P)
    // Ferret, TODO. Not implement yet
#endif
#endif // MAC_TO_MEDIA

            port_state_machine[chip_port] = PORT_DISABLED; // Change state
        }
    }
}
#endif


/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
