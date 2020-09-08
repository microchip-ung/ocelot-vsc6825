//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#include "common.h"     /* Always include common.h at the first place of user-defined herder files */
#include "timer.h"
#include "print.h"

#include "phydrv.h"
#include "phy_base.h"
#include "phy_family.h"

#define PHY_DEBUG (0)

#if VTSS_ELISE

#if VTSS_ELISE_A
/************************************************************************/
/* ELISE family functions                                               */
/************************************************************************/
// Initialization needed for ELISE. (For whole chip and must be done before init. of the individual ports)
//
// In : port_no : Port number (MUST be the first port for the chip)..
//
// Return : VTSS_RC_OK if configuration done else error code.
static vtss_rc vtss_phy_pre_init_seq_elise_rev_a(vtss_port_no_t port_no)
{
#if PHY_DEBUG
    println_str("vtss_phy_pre_init_seq_elise_rev_a");
#endif

    VTSS_RC(vtss_phy_wr        (port_no, 31, 0 )); // STD page
    VTSS_RC(vtss_phy_wr_masked (port_no, 22, 0x0001, 0x0001));
    VTSS_RC(vtss_phy_wr        (port_no, 31, 0x2A30 )); // Test page
    VTSS_RC(vtss_phy_wr_masked (port_no,  8, 0x8000, 0x8000));
    VTSS_RC(vtss_phy_wr        (port_no, 31, 0x52B5 )); // Token ring page
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0068));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x8980));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x8f90));
   
#if 1/*- The following code was added for bug 13269 */
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0000));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x0003));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x8796));
#endif

#ifndef VTSS_10BASE_TE
#else //using 10BASE-Te
    VTSS_RC(vtss_phy_wr        (port_no, 31, 2 )); // EXT2 page
    VTSS_RC(vtss_phy_wr_masked (port_no, 17, 0x8000, 0x8000));
    VTSS_RC(vtss_phy_wr        (port_no, 31, 0x52B5 )); // Token ring page
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0008));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0xa499));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x8486));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0075));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0xf759));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x8488));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0000));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x0914));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x848a));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x00f7));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0xff7b));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x848c));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0000));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x0eb9));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x848e));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0061));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x85d6));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x8490));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0055));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x44d2));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x8492));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0044));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0xa8aa));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x8494));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0000));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x0cb9));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x8496));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x00f7));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0xff79));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x8498));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0000));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x0caa));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x849a));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0061));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x8618));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x849c));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0000));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x0618));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x849e));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0000));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x0018));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x84a0));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0061));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x848a));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x84a2));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0000));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x0000));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x84a4));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0000));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x0000));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x84a6));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0000));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x0000));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x84a8));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0000));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x0000));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x84aa));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0029));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x265d));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x84ac));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x007d));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0xd658));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x84ae));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0061));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x8618));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x84b0));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0061));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x8618));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x84b2));
    VTSS_RC(vtss_phy_wr        (port_no, 18, 0x0061));
    VTSS_RC(vtss_phy_wr        (port_no, 17, 0x8618));
    VTSS_RC(vtss_phy_wr        (port_no, 16, 0x84b4));
#endif
    VTSS_RC(vtss_phy_wr        (port_no, 31, 0x2A30 )); // Test page
    VTSS_RC(vtss_phy_wr_masked (port_no,  8, 0x0000, 0x8000));
    VTSS_RC(vtss_phy_wr        (port_no, 31, 0 )); // STD page
    VTSS_RC(vtss_phy_wr_masked (port_no, 22, 0x0000, 0x0001));

    return VTSS_RC_OK;
}
#endif // VTSS_ELISE_A


// Initialization needed for Elise. (For whole chip and must be done before init. of the individual ports)
//
// In : port_no : Port number (MUST be the first port for the chip)..
//
// Return : VTSS_RC_OK if configuration done else error code.
vtss_rc elise_init_seq_pre(
    vtss_port_no_t         port_no,
    phy_id_t               *phy_id
)
{
#if PHY_DEBUG
    println_str("elise_init_seq_pre");
#endif

#if PHY_DEBUG
    println_str("step 6. apply patch");
#endif

#if VTSS_ELISE_A
    VTSS_RC(vtss_phy_pre_init_seq_elise_rev_a(port_no));
#endif
    phy_id = 0;
    port_no = 0;
    return VTSS_RC_OK;
}


/**
 * ELISE has no internal CPU, hence no code needed here.
 */
vtss_rc elise_init_seq(
    vtss_port_no_t         port_no,
    phy_id_t               *phy_id
)
{
#if PHY_DEBUG
    println_str("elise_init_seq");
#endif

    phy_id = 0;
    port_no = 0;

    return VTSS_RC_OK;
}


vtss_rc elise_mac_media_if_setup
(
    vtss_port_no_t          port_no,
    vtss_phy_reset_conf_t   *conf
) {
    u16 reg_val;
    u16 reg_mask;

#if PHY_DEBUG
    println_str("elise_mac_media_if_setup");
#endif

    // Setup MAC Configuration
    VTSS_RC(vtss_phy_page_gpio( port_no));

    switch (conf->mac_if) {
    case VTSS_PORT_INTERFACE_QSGMII:
#if PHY_DEBUG
        println_str("step 7. configure 19G for MAC mode");
#endif
        VTSS_RC(vtss_phy_wr_masked (port_no, 19, 0x4000, 0xC000));
        break;
    case VTSS_PORT_INTERFACE_NO_CONNECTION:
        break;
    default:
#if PHY_DEBUG
        print_str("Unknown MAC IF: ");
        print_dec(conf->mac_if);
        print_cr_lf();
#endif
        return VTSS_RC_ERROR;
    }

    // Setup media interface
    switch (conf->media_if) {
    case VTSS_PHY_MEDIA_IF_CU: {
        int loops = 30;
        u16 val;

#if PHY_DEBUG
        println_str("step 8. configure 18G for MAC on PHY");
#endif

        VTSS_RC(vtss_phy_wr (port_no, 18, 0x80E0));

        do {
            delay_1(5);
            VTSS_RC(vtss_phy_rd( port_no, 18, &val ));
            loops--;
        } while (loops && (val & 0x8000));

#if PHY_DEBUG
        if (val & 0x8000) println_str("err: 18G still has value");
        if (val & 0x4000) println_str("err: internal process reports error");
#endif

        break;
    }

    default:
#if PHY_DEBUG
        print_str("Unknown Media IF: ");
        print_dec(conf->media_if);
        print_cr_lf();
#endif
        return VTSS_RC_ERROR;
    }

#if PHY_DEBUG
    println_str("step 9. configure MAC and media mode");
#endif

    // Setup Media interface
    VTSS_RC(vtss_phy_page_std(port_no));
    reg_val  = 0x0000;
    reg_mask = 0x0700;
    VTSS_RC(vtss_phy_wr_masked (port_no, 23, reg_val, reg_mask));

    // Port must be reset in order to update the media operating mode for register 23
    VTSS_RC(vtss_phy_soft_reset_port(port_no));

    return VTSS_RC_OK;
}

#if TRANSIT_FAN_CONTROL || TRANSIT_THERMAL
vtss_rc elise_read_temp_reg(
    vtss_port_no_t  port_no,
    ushort          *temp
)
{
#if PHY_DEBUG
    println_str("elise_read_temp_reg");
#endif
    *temp = vtss_phy_read_temp(port_no);
    return VTSS_RC_OK;
}
#endif // TRANSIT_FAN_CONTROL || TRANSIT_THERMAL

#endif // VTSS_ELISE

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
