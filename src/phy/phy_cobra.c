//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT




#include "common.h"     /* Always include common.h at the first place of user-defined herder files */
#include "timer.h"
#include "print.h"
#include "hwport.h"
#include "phymap.h"
#include "phydrv.h"
#include "phy_base.h"
#include "phy_family.h"

#define PHY_DEBUG (0)

#if VTSS_COBRA

/****************************************************************************
 *
 *
 * Defines
 *
 *
 ****************************************************************************/

/****************************************************************************
 *
 *
 * Typedefs and enums
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


/*****************************************************************************
 *
 *
 * PHY initialization sequences
 *
 *
 *
 ****************************************************************************/
vtss_rc cobra_mac_media_if_setup(vtss_port_no_t port_no, const vtss_phy_reset_conf_t *const conf)
{
    u16 reg = 0;
    u16 mac_media_mode_sel_15_12 = 0x0; // Mac/Media interface mode selct bits 15:12 - Datasheet table 36
    u16 mac_media_mode_sel_2_1 = 0x0;   // Mac/Media interface mode selct bits 2:1 - Datasheet table 36
#if PHY_DEBUG
    print_str("cobra_mac_media_if_setup: conf->mac_if=");
    print_hex_dw(conf->mac_if);
    print_str(",conf->media_if=");
    print_hex_dw(conf->media_if);
    print_cr_lf();
#endif
    switch (conf->mac_if) {

    case VTSS_PORT_INTERFACE_SGMII:
        switch (conf->media_if) {
        case VTSS_PHY_MEDIA_IF_CU:
            mac_media_mode_sel_15_12 = 0xB;
            mac_media_mode_sel_2_1   = 0x0;
            break;
        case VTSS_PHY_MEDIA_IF_FI_1000BX:
        case VTSS_PHY_MEDIA_IF_SFP_PASSTHRU:
            mac_media_mode_sel_15_12 = 0x9;
            mac_media_mode_sel_2_1   = 0x1;
            break;
        default:
            goto conf_error;
        }
        break;


    case VTSS_PORT_INTERFACE_RGMII:
        switch (conf->media_if) {
        case VTSS_PHY_MEDIA_IF_CU:
            mac_media_mode_sel_15_12 = 0x1;
            mac_media_mode_sel_2_1   = 0x2;
            break;
        case VTSS_PHY_MEDIA_IF_FI_1000BX:
        case VTSS_PHY_MEDIA_IF_FI_100FX:
            mac_media_mode_sel_15_12 = 0x1;
            mac_media_mode_sel_2_1   = 0x1;
            break;
        case VTSS_PHY_MEDIA_IF_AMS_CU_1000BX:
        case VTSS_PHY_MEDIA_IF_AMS_CU_100FX:
            mac_media_mode_sel_15_12 = 0x0;
            mac_media_mode_sel_2_1   = 0x2;
            break;
        case VTSS_PHY_MEDIA_IF_AMS_FI_1000BX:
        case VTSS_PHY_MEDIA_IF_AMS_FI_100FX:
            mac_media_mode_sel_15_12 = 0x0;
            mac_media_mode_sel_2_1   = 0x1;
            break;
        default:
            goto conf_error;
        }
        break;

    case VTSS_PORT_INTERFACE_RTBI:
        switch (conf->media_if) {
        case VTSS_PHY_MEDIA_IF_CU:
            mac_media_mode_sel_15_12 = 0x4;
            mac_media_mode_sel_2_1   = 0x0;
            break;
        case VTSS_PHY_MEDIA_IF_FI_1000BX:
        case VTSS_PHY_MEDIA_IF_FI_100FX:
            mac_media_mode_sel_15_12 = 0x5;
            mac_media_mode_sel_2_1   = 0x1;
            break;
        default:
            goto conf_error;
        }
        break;


    case VTSS_PORT_INTERFACE_TBI:
        switch (conf->media_if) {
        case VTSS_PHY_MEDIA_IF_CU:
            mac_media_mode_sel_15_12 = 0x6;
            mac_media_mode_sel_2_1   = 0x0;
            break;
        case VTSS_PHY_MEDIA_IF_FI_1000BX:
        case VTSS_PHY_MEDIA_IF_FI_100FX:
            mac_media_mode_sel_15_12 = 0x7;
            mac_media_mode_sel_2_1   = 0x1;
            break;
        default:
            goto conf_error;
        }
        break;



    case VTSS_PORT_INTERFACE_GMII:
        switch (conf->media_if) {
        case VTSS_PHY_MEDIA_IF_CU:
            mac_media_mode_sel_15_12 = 0x3;
            mac_media_mode_sel_2_1   = 0x2;
            break;
        case VTSS_PHY_MEDIA_IF_FI_1000BX:
        case VTSS_PHY_MEDIA_IF_FI_100FX:
            mac_media_mode_sel_15_12 = 0x3;
            mac_media_mode_sel_2_1   = 0x1;
            break;
        case VTSS_PHY_MEDIA_IF_AMS_CU_1000BX:
        case VTSS_PHY_MEDIA_IF_AMS_CU_100FX:
            mac_media_mode_sel_15_12 = 0x2;
            mac_media_mode_sel_2_1   = 0x2;
            break;
        case VTSS_PHY_MEDIA_IF_AMS_FI_1000BX:
        case VTSS_PHY_MEDIA_IF_AMS_FI_100FX:
            mac_media_mode_sel_15_12 = 0x2;
            mac_media_mode_sel_2_1   = 0x1;
            break;
        default:
            goto conf_error;
        }
        break;

    case VTSS_PORT_INTERFACE_SERDES:
        switch (conf->media_if) {
        case VTSS_PHY_MEDIA_IF_CU:
            mac_media_mode_sel_15_12 = 0xF;
            mac_media_mode_sel_2_1   = 0x0;
            break;
        case VTSS_PHY_MEDIA_IF_SFP_PASSTHRU:
        case VTSS_PHY_MEDIA_IF_AMS_CU_1000BX:
            mac_media_mode_sel_15_12 = 0x9;
            mac_media_mode_sel_2_1   = 0x1;
            break;
        default:
            goto conf_error;
        }
        break;
    default:
        goto conf_error;
    }


    // Do the register (address 0x23)  write
    reg = 0x0a20; // Setup skew and RX idle clock, datasheet table 36.
    reg |= mac_media_mode_sel_15_12 << 12;
    reg |= mac_media_mode_sel_2_1 << 1;

    VTSS_RC(vtss_phy_wr(port_no, VTSS_PHY_EXTENDED_PHY_CONTROL, reg));
    return VTSS_RC_OK;


// Configuration error.
conf_error:
    println_str("Cobra family Media interface not supported for mac if");
    return VTSS_RC_ERROR;
}


/* Init Scripts for VSC8211/VSC8221 aka Cobra */
vtss_rc cobra_init_seq(vtss_port_no_t port_no)
{
#if PHY_DEBUG
    println_str("cobra_init_seq");	
#endif
    /* Enable token-ring register access for entire init script */
    VTSS_RC(phy_page_tp( port_no));
    VTSS_RC(vtss_phy_wr_masked( port_no, 8, 0x0200, 0x0200));
  //  VTSS_RC(vtss_phy_page_tr( port_no));
    VTSS_RC(vtss_phy_wr(port_no, 31, 0x52B5));

    /* BZ 1769 */
    /* Performance robustness improvement on 50m loops */
    VTSS_RC(vtss_phy_wr( port_no, 0, 0xafa4));
    VTSS_RC(vtss_phy_wr_masked( port_no, 2, 0x0000, 0x0000));
    VTSS_RC(vtss_phy_wr_masked( port_no, 1, 0x4000, 0xf000));
    VTSS_RC(vtss_phy_wr( port_no, 0, 0x8fa4));

    /* BZ 2337 */
    /* Inter-operability with Intel 82547E1 L322SQ96 */
    VTSS_RC(vtss_phy_wr( port_no, 0, 0xafae));
    VTSS_RC(vtss_phy_wr_masked( port_no, 2, 0x0000, 0x0000));
    VTSS_RC(vtss_phy_wr_masked( port_no, 1, 0x0600, 0x7f80));
    VTSS_RC(vtss_phy_wr( port_no, 0, 0x8fae));
    VTSS_RC(vtss_phy_page_std( port_no));
    VTSS_RC(vtss_phy_wr_masked( port_no, 18, 0x0040, 0x0040));
//    VTSS_RC(vtss_phy_page_tr( port_no));
    VTSS_RC(vtss_phy_wr(port_no, 31, 0x52B5));
    /* BZ 1800 */
    /* 100BT Jumbo packet mode script */
    VTSS_RC(vtss_phy_wr( port_no, 0, 0xb786));
    VTSS_RC(vtss_phy_wr_masked( port_no, 2, 0x0000, 0x0000));
    VTSS_RC(vtss_phy_wr_masked( port_no, 1, 0x0e20, 0x1fe0));
    VTSS_RC(vtss_phy_wr( port_no, 0, 0x9786));

    /* BZ 2331 */
    /* Insufficient RGMII drive strength on long traces */
    VTSS_RC(phy_page_tp( port_no));
    VTSS_RC(vtss_phy_wr_masked( port_no, 3, 0x2000, 0x2000));
    VTSS_RC(vtss_phy_wr_masked( port_no, 3, 0x1080, 0x1ff8));
//    VTSS_RC(vtss_phy_page_tr( port_no));
    VTSS_RC(vtss_phy_wr(port_no, 31, 0x52B5));
    /* BZ 2232 */
    /* DSP Optimization script */
    VTSS_RC(vtss_phy_wr( port_no, 0, 0xaf8a));
    VTSS_RC(vtss_phy_wr_masked( port_no, 2, 0x0000, 0x0000));
    VTSS_RC(vtss_phy_wr_masked( port_no, 1, 0x0008, 0x000c));
    VTSS_RC(vtss_phy_wr( port_no, 0, 0x8f8a));
    VTSS_RC(vtss_phy_wr( port_no, 0, 0xaf86));
    VTSS_RC(vtss_phy_wr_masked( port_no, 2, 0x0008, 0x000c));
    VTSS_RC(vtss_phy_wr_masked( port_no, 1, 0x0000, 0x0000));
    VTSS_RC(vtss_phy_wr( port_no, 0, 0x8f86));
    VTSS_RC(vtss_phy_wr( port_no, 0, 0xaf82));
    VTSS_RC(vtss_phy_wr_masked( port_no, 2, 0x0000, 0x0000));
    VTSS_RC(vtss_phy_wr_masked( port_no, 1, 0x0100, 0x0180));
    VTSS_RC(vtss_phy_wr( port_no, 0, 0x8f82));

    /* New bugzilla 11377 */
    /* Forced-10BASE-T PHY falsely linking up with forced-100BASE-TX link partner */
    VTSS_RC(vtss_phy_wr( port_no, 0, 0xa7f4));
    VTSS_RC(vtss_phy_wr_masked( port_no, 2, 0x0000, 0x0000));
    VTSS_RC(vtss_phy_wr_masked( port_no, 1, 0x0002, 0x0006));
    VTSS_RC(vtss_phy_wr( port_no, 0, 0x87f4));
    VTSS_RC(vtss_phy_wr( port_no, 0, 0xa7f8));
    VTSS_RC(vtss_phy_wr_masked( port_no, 2, 0x0000, 0x0000));
    VTSS_RC(vtss_phy_wr_masked( port_no, 1, 0x0800, 0x0800));
    VTSS_RC(vtss_phy_wr( port_no, 0, 0x87f8));

    /* Restore normal clock gating and return to main page after init script */
    VTSS_RC(phy_page_tp( port_no));
    VTSS_RC(vtss_phy_wr_masked( port_no, 8, 0x0000, 0x0200));
    VTSS_RC(vtss_phy_page_std( port_no));

    /* Enable Smart Pre-emphasis */
    VTSS_RC(vtss_phy_enab_smrt_premphasis(port_no));

    return VTSS_RC_OK;
}

void cobra_power_saving_disable(vtss_cport_no_t chip_port)
{
    phy_id_t phy_id;

    /* Disable Power Savings */
    phy_read_id(chip_port, &phy_id);
    if (phy_map(chip_port) && (phy_id.family == VTSS_PHY_FAMILY_COBRA)) {
        phy_page_tp(chip_port);
        phy_write_masked(chip_port, 12, 0x0000, 0xfc00);
        phy_write_masked(chip_port, 24, 0x2000, 0x2000);
        phy_page_std(chip_port);
        phy_write_masked(chip_port, 28, 0x0000, 0x0040);
        phy_write_masked(chip_port, 0, 0x8000, 0x8000);
        delay_1(2);
    }
}

#endif

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
