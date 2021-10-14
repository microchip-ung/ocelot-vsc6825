//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#include "common.h"     /* Always include common.h at the first place of user-defined herder files */

#if MAC_TO_MEDIA

#include "vtss_api_base_regs.h"
#include "h2pcs1g.h"
#include "main.h"
#include "timer.h"
#include "misc2.h"
#include "h2io.h"
#include "h2sdcfg.h"
#include "print.h"
#include "phymap.h"
#include "phydrv.h"
#include "phytsk.h"

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
#if defined(H2_PCS1G_DEBUG_ENABLE)
#include "print.h"
#endif /* H2_PCS1G_DEBUG_ENABLE */


/**
 * Test whether a bitfield is set in value.
 */
#define BF(__field__, __value__) (((__field__ & __value__) == __field__) ? 1 : 0)

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

static uchar xdata clause_37_flowcontrol[NO_OF_CHIP_PORTS];

/* ************************************************************************ */
void h2_pcs1g_clause_37_control_set(const vtss_cport_no_t chip_port, BOOL flow_ctrl_mode)
/* ------------------------------------------------------------------------ --
 * Purpose     : Set 1000Base-X Fiber Auto-negotiation (Clause 37)
 * Remarks     :
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    ulong tgt = VTSS_TO_DEV(chip_port);
    uchar mac_if = phy_map_miim_no(chip_port);

    clause_37_flowcontrol[chip_port] = flow_ctrl_mode;

    /* Set aneg capabilities */
#if defined(VTSS_ARCH_LUTON26)
    if (MAC_IF_SGMII == mac_if) {
        H2_WRITE_MASKED(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG(tgt), 0x00010003, 0xffff0003);
    } else {
	    H2_WRITE_MASKED(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG(tgt), 0x00a00003, 0xffff0003);
    }
    delay_1(50);

#elif defined(VTSS_ARCH_OCELOT) && !defined(SGMII_SERDES_FORCE_1G_DEBUG_ENABLE)
    if (mac_if == MAC_IF_SGMII) {
        H2_WRITE_MASKED(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG(tgt),
                        VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG_ADV_ABILITY(1) |
                        VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG_SW_RESOLVE_ENA(1) |
                        VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG_ANEG_RESTART_ONE_SHOT(1) |
                        VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG_ANEG_ENA(1),
                        VTSS_M_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG_ADV_ABILITY |
                        VTSS_M_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG_SW_RESOLVE_ENA |
                        VTSS_M_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG_ANEG_RESTART_ONE_SHOT |
                        VTSS_M_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG_ANEG_ENA);
    } else {
        H2_WRITE_MASKED(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG(tgt),
                    VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG_ADV_ABILITY(flow_ctrl_mode ? 0xa0 : 0x20) |
                    VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG_SW_RESOLVE_ENA(0) |
                    VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG_ANEG_RESTART_ONE_SHOT(mac_if == MAC_IF_SERDES_1G ? 1 : 0) |
                    VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG_ANEG_ENA(mac_if == MAC_IF_SERDES_1G ? 1 : 0),
                    VTSS_M_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG_ADV_ABILITY |
                    VTSS_M_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG_SW_RESOLVE_ENA |
                    VTSS_M_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG_ANEG_RESTART_ONE_SHOT |
                    VTSS_M_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG_ANEG_ENA);
    }

    delay_1(50);
#endif
}
#ifdef SGMII_SERDES_FORCE_1G_DEBUG_ENABLE
/* ************************************************************************ */
uchar h2_pcs1g_status_get(const vtss_cport_no_t chip_port)
/* ------------------------------------------------------------------------ --
 * Purpose     : Get PCS1G Status ANEG disabled
 * Remarks     :
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    ulong   value;
    uchar   link,lm = LINK_MODE_DOWN;
    ulong   tgt = VTSS_TO_DEV(chip_port);

    /* 1. Read PCS link status */
    H2_READ(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_LINK_STATUS(tgt), value);

    /* Get PCS link status bit(4)*/
    link = VTSS_X_DEV_PCS1G_CFG_STATUS_PCS1G_LINK_STATUS_LINK_STATUS(value);

    if (link) {
        lm = LINK_MODE_FDX_1000;
		lm |= LINK_MODE_PAUSE_MASK;
    } else if (!link) {
		lm = LINK_MODE_DOWN;
	}

    return lm;
}
#endif

/* ************************************************************************ */
uchar h2_pcs1g_clause_37_status_get(const vtss_cport_no_t chip_port)
/* ------------------------------------------------------------------------ --
 * Purpose     : Get 1000Base-X Fiber Auto-negotiation status (Clause 37)
 * Remarks     :
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    ulong   value, lp_adv_ability;
    uchar   link, complete, lm = LINK_MODE_DOWN;
    ulong   tgt = VTSS_TO_DEV(chip_port);
    uchar   mac_if = phy_map_miim_no(chip_port);
    bool    synced_status = 0, workaround_applied = 0;
#if defined(VTSS_ARCH_LUTON26)
    uchar in_progress, ii;
#endif

get_status_again:

    /* 1. Get the link state 'down' sticky bit  */
#if defined(VTSS_ARCH_LUTON26)
    H2_READ(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_STICKY(tgt), value);
    link = BF(VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_STICKY_LINK_DOWN_STICKY, value) ? 0 : 1;
#elif defined(VTSS_ARCH_OCELOT)
    H2_READ(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_STICKY(tgt), value);
    link = !VTSS_X_DEV_PCS1G_CFG_STATUS_PCS1G_STICKY_LINK_DOWN_STICKY(value) && !VTSS_X_DEV_PCS1G_CFG_STATUS_PCS1G_STICKY_OUT_OF_SYNC_STICKY(value);
#endif
#if defined(H2_PCS1G_DEBUG_ENABLE)
    print_str("h2_pcs1g_clause_37_status_get(), chip_port = ");
    print_dec(chip_port);
    print_str(", PCS1G_STICKY value = 0x");
    print_hex_dw(value);
    print_cr_lf();
#endif /* H2_PCS1G_DEBUG_ENABLE */


    /* Read PCS link status */
    H2_READ(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_LINK_STATUS(tgt), value);

    /* Get the link state 'down' sticky bit  */
#if defined(VTSS_ARCH_LUTON26)
    if (link) {
        H2_WRITE(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_STICKY(tgt),
                 VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_STICKY_LINK_DOWN_STICKY |
                 VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_STICKY_OUT_OF_SYNC_STICKY);
    } else { // Link-down
        link = BF(VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_LINK_STATUS_LINK_STATUS, value) &&
               BF(VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_LINK_STATUS_SYNC_STATUS, value);
    }

#elif defined(VTSS_ARCH_OCELOT)
    // Notice that 'LINK_DOWN_STICKY' bit is set if signal-detect or synchronization has been lost.
    // If the bit value is 1 when link-up state, we need to check the setting of
    // signal-detect(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_SD_CFG) in API h2.c/_setup_pcs()
    if (link) {
        /* Get PCS sync/link status */
        link = VTSS_X_DEV_PCS1G_CFG_STATUS_PCS1G_LINK_STATUS_SYNC_STATUS(value) ?
               VTSS_X_DEV_PCS1G_CFG_STATUS_PCS1G_LINK_STATUS_LINK_STATUS(value) :
               0 /* Assigned to link-down when the SYNC_STATUS is out of synchronized. */;
    } else { // Link-down
        /* The link has been down. Clear sticky bit by writing value 1 */
        H2_WRITE_MASKED(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_STICKY(tgt),
                        VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_STICKY_LINK_DOWN_STICKY(1) |
                        VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_STICKY_OUT_OF_SYNC_STICKY(1),
                        VTSS_M_DEV_PCS1G_CFG_STATUS_PCS1G_STICKY_LINK_DOWN_STICKY |
                        VTSS_M_DEV_PCS1G_CFG_STATUS_PCS1G_STICKY_OUT_OF_SYNC_STICKY);
    }
#endif // defined(VTSS_ARCH_LUTON26)


#if defined(VTSS_ARCH_LUTON26)
    synced_status = BF(VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_LINK_STATUS_SYNC_STATUS, value);
#elif defined(VTSS_ARCH_OCELOT)
    synced_status = BF(VTSS_M_DEV_PCS1G_CFG_STATUS_PCS1G_LINK_STATUS_SYNC_STATUS, value);
#endif

#if defined(VTSS_ARCH_LUTON26)
    for (ii = 0; ii < 3; ii++) {
        /* Get PCS ANEG status register */
        H2_READ(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_STATUS(tgt), value);

        /* Get 'Aneg in_progress'   */
        in_progress = BF(VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_STATUS_PR, value);
        if (in_progress) {
            break;
        }
        delay_1(5);
    }

    /* Get 'Aneg complete' */
    complete = BF(VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_STATUS_ANEG_COMPLETE, value);

#elif defined(VTSS_ARCH_OCELOT)
    /* Get PCS ANEG status register */
    H2_READ(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_STATUS(tgt), value);

    /* Get 'Aneg complete' */
    complete = VTSS_X_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_STATUS_ANEG_COMPLETE(value);
#endif // defined(VTSS_ARCH_LUTON26)

    /* Workaround 1: for a Serdes issue, when aneg completes with FDX capability=0 */
    /* Workaround 2: for a Serdes issue, when aneg UN-completes but synced status is up */
    if (!workaround_applied && MAC_IF_SERDES_1G == mac_if) {
#if defined(VTSS_ARCH_LUTON26)
        if ((complete && (((value >> 21) & 0x1) == 0)) || (synced_status && !complete)) {
                H2_WRITE_MASKED(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_CFG(tgt),
                                0,
                                VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_CFG_PCS_ENA);
                H2_WRITE(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_CFG(tgt),
                         VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_CFG_PCS_ENA);
#elif defined(VTSS_ARCH_OCELOT)
		if ((complete && (((value >> 21) & 0x1) == 0))) {
                /* Re-enable PCS */
                H2_WRITE_MASKED(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_CFG(tgt),
                                VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_CFG_PCS_ENA(0),
                                VTSS_M_DEV_PCS1G_CFG_STATUS_PCS1G_CFG_PCS_ENA);
                delay_1(50);
                H2_WRITE_MASKED(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_CFG(tgt),
                                VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_CFG_PCS_ENA(1),
                                VTSS_M_DEV_PCS1G_CFG_STATUS_PCS1G_CFG_PCS_ENA);
#endif

                /* Restart Aneg */
                h2_pcs1g_clause_37_control_set(chip_port,clause_37_flowcontrol[chip_port]);

                /* Clear sticky bit by writing value 1.
                   This step is important, otherwise the LINK_DOWN_STICKY bit may keep the link-down result.
                 */
#if defined(VTSS_ARCH_LUTON26)
                H2_WRITE_MASKED(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_STICKY(tgt),
                                VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_STICKY_LINK_DOWN_STICKY |
                                VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_STICKY_OUT_OF_SYNC_STICKY,
                                VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_STICKY_LINK_DOWN_STICKY |
                                VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_STICKY_OUT_OF_SYNC_STICKY);
#elif defined(VTSS_ARCH_OCELOT)
                H2_WRITE_MASKED(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_STICKY(tgt),
                                VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_STICKY_LINK_DOWN_STICKY(1) |
                                VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_STICKY_OUT_OF_SYNC_STICKY(1),
                                VTSS_M_DEV_PCS1G_CFG_STATUS_PCS1G_STICKY_LINK_DOWN_STICKY |
                                VTSS_M_DEV_PCS1G_CFG_STATUS_PCS1G_STICKY_OUT_OF_SYNC_STICKY);
#endif

            workaround_applied = 1;
            goto get_status_again;
        }
    } // End of Workaround

    /* Return partner advertisement ability */
    lp_adv_ability = VTSS_X_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_STATUS_LP_ADV_ABILITY(value);
    if (MAC_IF_SGMII == mac_if) {
        uchar sgmii_link = (lp_adv_ability & 0x8000) ? 1 : 0;

        lm = ((lp_adv_ability >> 10) & 3);
        if (VTSS_BOOL(lp_adv_ability & (1 << 12))) {
            lm |= LINK_MODE_FDX_MASK;
        }
        if (VTSS_BOOL(value & (1 << 7))) {
            lm |= LINK_MODE_PAUSE_MASK;
        }

        if (link) {
            link = sgmii_link;
        }

        if (!link) {
            lm = LINK_MODE_DOWN;
        }
    } else if (link && complete) {
        lm = LINK_MODE_FDX_1000;
        if (VTSS_BOOL(lp_adv_ability & (1 << 7))) {
            lm |= LINK_MODE_PAUSE_MASK;
        }
    }

#if defined(H2_PCS1G_DEBUG_ENABLE)
    print_str("h2_pcs1g_clause_37_status_get(), chip_port = ");
    print_dec(chip_port);
    print_str(", lm = 0x");
    print_hex_b(lm);
    print_str(", lp_adv_ability = 0x");
    print_hex_dw(lp_adv_ability);
    print_cr_lf();
#endif /* H2_PCS1G_DEBUG_ENABLE */

    return lm;
}

/* ************************************************************************ */
uchar h2_pcs1g_100fx_status_get(const vtss_cport_no_t chip_port)
/* ------------------------------------------------------------------------ --
 * Purpose     : Get the 100FX (fiber) port status (no autonegotiation)
 * Remarks     :
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    ulong value;
    ulong tgt = VTSS_TO_DEV(chip_port);
    uchar link_mode = LINK_MODE_DOWN;

    /* Get the PCS status  */
    H2_READ(VTSS_DEV_PCS_FX100_STATUS_PCS_FX100_STATUS(tgt), value);

#if defined(VTSS_ARCH_LUTON26)
    if (BF(VTSS_F_DEV_PCS_FX100_STATUS_PCS_FX100_STATUS_SYNC_LOST_STICKY, value) ||
        BF(VTSS_F_DEV_PCS_FX100_STATUS_PCS_FX100_STATUS_SSD_ERROR_STICKY, value) ||
        BF(VTSS_F_DEV_PCS_FX100_STATUS_PCS_FX100_STATUS_FEF_FOUND_STICKY, value) ||
        BF(VTSS_F_DEV_PCS_FX100_STATUS_PCS_FX100_STATUS_PCS_ERROR_STICKY, value))
#elif defined(VTSS_ARCH_OCELOT)
    if (BF(VTSS_M_DEV_PCS_FX100_STATUS_PCS_FX100_STATUS_SYNC_LOST_STICKY, value) ||
        BF(VTSS_M_DEV_PCS_FX100_STATUS_PCS_FX100_STATUS_SSD_ERROR_STICKY, value) ||
        BF(VTSS_M_DEV_PCS_FX100_STATUS_PCS_FX100_STATUS_FEF_FOUND_STICKY, value) ||
        BF(VTSS_M_DEV_PCS_FX100_STATUS_PCS_FX100_STATUS_PCS_ERROR_STICKY, value) ||
        BF(VTSS_M_DEV_PCS_FX100_STATUS_PCS_FX100_STATUS_FEF_STATUS, value))
#endif
    {
#if defined(H2_PCS1G_DEBUG_ENABLE)
        if (port_link_mode_get(chip_port) != LINK_MODE_DOWN) {
            print_str("Calling h2_pcs1g_100fx_status_get(), chip_port=0x");
            print_hex_b(chip_port);
            print_str(", value=0x");
            print_hex_dw(value);
            print_cr_lf();
        }
#endif // H2_PCS1G_DEBUG_ENABLE

        /* The link has been down. Clear the sticky bit */
        H2_WRITE_MASKED(VTSS_DEV_PCS_FX100_STATUS_PCS_FX100_STATUS(tgt),
                        VTSS_F_DEV_PCS_FX100_STATUS_PCS_FX100_STATUS_SYNC_LOST_STICKY(1) |
                        VTSS_F_DEV_PCS_FX100_STATUS_PCS_FX100_STATUS_SSD_ERROR_STICKY(1) |
                        VTSS_F_DEV_PCS_FX100_STATUS_PCS_FX100_STATUS_FEF_FOUND_STICKY(1) |
                        VTSS_F_DEV_PCS_FX100_STATUS_PCS_FX100_STATUS_PCS_ERROR_STICKY(1),
                        VTSS_M_DEV_PCS_FX100_STATUS_PCS_FX100_STATUS_SYNC_LOST_STICKY |
                        VTSS_M_DEV_PCS_FX100_STATUS_PCS_FX100_STATUS_SSD_ERROR_STICKY |
                        VTSS_M_DEV_PCS_FX100_STATUS_PCS_FX100_STATUS_FEF_FOUND_STICKY |
                        VTSS_M_DEV_PCS_FX100_STATUS_PCS_FX100_STATUS_PCS_ERROR_STICKY);
        delay_1(1); // BZ18779
        H2_READ(VTSS_DEV_PCS_FX100_STATUS_PCS_FX100_STATUS(tgt), value);
    }

#if defined(VTSS_ARCH_LUTON26)
    if (BF(VTSS_F_DEV_PCS_FX100_STATUS_PCS_FX100_STATUS_SIGNAL_DETECT, value) &&
        !BF(VTSS_F_DEV_PCS_FX100_STATUS_PCS_FX100_STATUS_SYNC_LOST_STICKY, value) &&
        !BF(VTSS_F_DEV_PCS_FX100_STATUS_PCS_FX100_STATUS_SSD_ERROR_STICKY, value) &&
        !BF(VTSS_F_DEV_PCS_FX100_STATUS_PCS_FX100_STATUS_FEF_FOUND_STICKY, value) &&
        !BF(VTSS_F_DEV_PCS_FX100_STATUS_PCS_FX100_STATUS_PCS_ERROR_STICKY, value))
#elif defined(VTSS_ARCH_OCELOT)
    if (BF(VTSS_M_DEV_PCS_FX100_STATUS_PCS_FX100_STATUS_SIGNAL_DETECT, value) &&
        BF(VTSS_M_DEV_PCS_FX100_STATUS_PCS_FX100_STATUS_SYNC_STATUS, value) &&
        !BF(VTSS_M_DEV_PCS_FX100_STATUS_PCS_FX100_STATUS_SYNC_LOST_STICKY, value) &&
        !BF(VTSS_M_DEV_PCS_FX100_STATUS_PCS_FX100_STATUS_SSD_ERROR_STICKY, value) &&
        !BF(VTSS_M_DEV_PCS_FX100_STATUS_PCS_FX100_STATUS_FEF_FOUND_STICKY, value) &&
        !BF(VTSS_M_DEV_PCS_FX100_STATUS_PCS_FX100_STATUS_PCS_ERROR_STICKY, value) &&
        !BF(VTSS_M_DEV_PCS_FX100_STATUS_PCS_FX100_STATUS_FEF_STATUS, value))
    {
        link_mode = LINK_MODE_FDX_100;
    }
#endif

    return link_mode;
}

/* ************************************************************************ */
void h2_pcs1g_clock_set(vtss_cport_no_t chip_port, BOOL enable)
/* ------------------------------------------------------------------------ --
 * Purpose     : Set PCS clock
 * Remarks     :
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
#if defined(VTSS_ARCH_LUTON26)
    H2_WRITE_MASKED(VTSS_DEV_PORT_MODE_CLOCK_CFG( VTSS_TO_DEV(chip_port)),
                    enable ? 0 : VTSS_F_DEV_PORT_MODE_CLOCK_CFG_PCS_TX_RST |
                    enable ? 0 : VTSS_F_DEV_PORT_MODE_CLOCK_CFG_PCS_RX_RST |
                    enable ? 0 : VTSS_F_DEV_PORT_MODE_CLOCK_CFG_PORT_RST,
                    VTSS_F_DEV_PORT_MODE_CLOCK_CFG_PCS_TX_RST |
                    VTSS_F_DEV_PORT_MODE_CLOCK_CFG_PCS_RX_RST |
                    VTSS_F_DEV_PORT_MODE_CLOCK_CFG_PORT_RST);

#elif defined(VTSS_ARCH_OCELOT)
    // EA = 0; // Disable interrupt while doing the clock reset.
    H2_WRITE_MASKED(VTSS_DEV_PORT_MODE_CLOCK_CFG( VTSS_TO_DEV(chip_port)),
                    VTSS_F_DEV_PORT_MODE_CLOCK_CFG_PCS_TX_RST(!enable) |
                    VTSS_F_DEV_PORT_MODE_CLOCK_CFG_PCS_RX_RST(!enable) |
                    VTSS_F_DEV_PORT_MODE_CLOCK_CFG_PORT_RST(!enable),
                    VTSS_M_DEV_PORT_MODE_CLOCK_CFG_PCS_TX_RST |
                    VTSS_M_DEV_PORT_MODE_CLOCK_CFG_PCS_RX_RST |
                    VTSS_M_DEV_PORT_MODE_CLOCK_CFG_PORT_RST);
    // EA = 1; // Enable interrupt
#endif

    delay_1(1); // Small delay after clock reset
}

/* ************************************************************************ */
void h2_pcs1g_setup(vtss_cport_no_t chip_port, mac_if_type_t if_type)
/* ------------------------------------------------------------------------ --
 * Purpose     : Enable psc1g serdes, sgmii or 100fx
 * Remarks     :
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    ulong tgt = VTSS_TO_DEV(chip_port);

    switch(if_type) {
    case MAC_IF_SERDES_2_5G:
    case MAC_IF_SERDES_1G: {
#if defined(VTSS_ARCH_LUTON26)
        H2_WRITE_MASKED(VTSS_DEV_PORT_MODE_CLOCK_CFG(tgt), 0x00000001, 0x0000003b); // Restart clock
        H2_WRITE_MASKED(VTSS_DEV_CMN_MAC_CFG_STATUS_MAC_MODE_CFG(tgt), 0x00000011, 0x00000011); //giga & fdx
        H2_WRITE(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_CFG(tgt), 0x00000001);
        H2_WRITE(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_MODE_CFG(tgt), 0x00000000);
        H2_WRITE(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_SD_CFG(tgt), 0x00000000);
        H2_WRITE_MASKED(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG(tgt), 0, 0x00000100);
        H2_WRITE_MASKED(VTSS_DEV_PCS_FX100_CONFIGURATION_PCS_FX100_CFG(tgt), 0x00000000, 0x00000001); // Disable 100FX PCS

#elif defined(VTSS_ARCH_OCELOT)
        /* Speed setup and enable PCS clock */
        H2_WRITE_MASKED(VTSS_DEV_PORT_MODE_CLOCK_CFG(tgt),
                        VTSS_F_DEV_PORT_MODE_CLOCK_CFG_LINK_SPEED(1) /* 1000/2500 Mbps */,
                        VTSS_M_DEV_PORT_MODE_CLOCK_CFG_LINK_SPEED);
        h2_pcs1g_clock_set(chip_port, TRUE);

        /* Set MAC Mode Configuration */
        H2_WRITE_MASKED(VTSS_DEV_MAC_CFG_STATUS_MAC_MODE_CFG(tgt),
                        VTSS_F_DEV_MAC_CFG_STATUS_MAC_MODE_CFG_GIGA_MODE_ENA(1) |
                        VTSS_F_DEV_MAC_CFG_STATUS_MAC_MODE_CFG_FDX_ENA(1),
                        VTSS_M_DEV_MAC_CFG_STATUS_MAC_MODE_CFG_GIGA_MODE_ENA |
                        VTSS_M_DEV_MAC_CFG_STATUS_MAC_MODE_CFG_FDX_ENA); // giga & fdx

        /* Enable PCS */
        H2_WRITE_MASKED(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_CFG(tgt),
                        VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_CFG_PCS_ENA(1),
                        VTSS_M_DEV_PCS1G_CFG_STATUS_PCS1G_CFG_PCS_ENA);

        /* Set PCS1G mode configuration: SERDES mode */
        H2_WRITE_MASKED(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_MODE_CFG(tgt),
                        VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_MODE_CFG_UNIDIR_MODE_ENA(0) |
                        VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_MODE_CFG_SGMII_MODE_ENA(0),
                        VTSS_M_DEV_PCS1G_CFG_STATUS_PCS1G_MODE_CFG_UNIDIR_MODE_ENA |
                        VTSS_M_DEV_PCS1G_CFG_STATUS_PCS1G_MODE_CFG_SGMII_MODE_ENA);

        /* Set PCS1G Auto-negotiation configuration: Software Resolve Abilities */
        H2_WRITE_MASKED(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG(tgt),
                        VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG_SW_RESOLVE_ENA(0),
                        VTSS_M_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG_SW_RESOLVE_ENA);

        /* Set Configuration bit groups for 100Base-FX PCS: Disable 100FX PCS */
        H2_WRITE_MASKED(VTSS_DEV_PCS_FX100_CONFIGURATION_PCS_FX100_CFG(tgt),
                        VTSS_F_DEV_PCS_FX100_CONFIGURATION_PCS_FX100_CFG_PCS_ENA(0),
                        VTSS_M_DEV_PCS_FX100_CONFIGURATION_PCS_FX100_CFG_PCS_ENA);
#endif

        return;
    }

    case MAC_IF_SGMII: {
#if defined(VTSS_ARCH_LUTON26)
        H2_WRITE_MASKED(VTSS_DEV_PORT_MODE_CLOCK_CFG(tgt), 0x00000001, 0x0000003b); // Restart clock
        /* --left to setup depends on link speed mode
        H2_WRITE_MASKED(VTSS_DEV_CMN_MAC_CFG_STATUS_MAC_MODE_CFG(tgt), 0x00000011, 0x00000011); //giga & fdx
        */
        H2_WRITE(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_CFG(tgt), 0x00000001);
        H2_WRITE(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_MODE_CFG(tgt), 0x00000001);
        H2_WRITE_MASKED(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG(tgt), 0x00000100, 0x00000100);
        H2_WRITE_MASKED(VTSS_DEV_PCS_FX100_CONFIGURATION_PCS_FX100_CFG(tgt), 0x00000000, 0x00000001); // Disable 100FX PCS

#elif defined(VTSS_ARCH_OCELOT)
        /* Speed setup and enable PCS clock */
        H2_WRITE_MASKED(VTSS_DEV_PORT_MODE_CLOCK_CFG(tgt),
                        VTSS_F_DEV_PORT_MODE_CLOCK_CFG_LINK_SPEED(1) /* 1000/2500 Mbps */,
                        VTSS_M_DEV_PORT_MODE_CLOCK_CFG_LINK_SPEED);
        h2_pcs1g_clock_set(chip_port, TRUE);

        /* Enable PCS */
        H2_WRITE_MASKED(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_CFG(tgt),
                        VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_CFG_PCS_ENA(1),
                        VTSS_M_DEV_PCS1G_CFG_STATUS_PCS1G_CFG_PCS_ENA);

        /* Set PCS1G mode configuration: SGMII mode */
        H2_WRITE_MASKED(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_MODE_CFG(tgt),
                        VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_MODE_CFG_UNIDIR_MODE_ENA(0) |
                        VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_MODE_CFG_SGMII_MODE_ENA(1),
                        VTSS_M_DEV_PCS1G_CFG_STATUS_PCS1G_MODE_CFG_UNIDIR_MODE_ENA |
                        VTSS_M_DEV_PCS1G_CFG_STATUS_PCS1G_MODE_CFG_SGMII_MODE_ENA);

        /* Set PCS1G Auto-negotiation configuration: Software Resolve Abilities */
        H2_WRITE_MASKED(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG(tgt),
                        VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG_SW_RESOLVE_ENA(1),
                        VTSS_M_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG_SW_RESOLVE_ENA);

        /* Set Configuration bit groups for 100Base-FX PCS: Disable 100FX PCS */
        H2_WRITE_MASKED(VTSS_DEV_PCS_FX100_CONFIGURATION_PCS_FX100_CFG(tgt),
                        VTSS_F_DEV_PCS_FX100_CONFIGURATION_PCS_FX100_CFG_PCS_ENA(0),
                        VTSS_M_DEV_PCS_FX100_CONFIGURATION_PCS_FX100_CFG_PCS_ENA);
#endif

        return;
    }

    case MAC_IF_100FX: {
#if defined(VTSS_ARCH_LUTON26)
        H2_WRITE_MASKED(VTSS_DEV_PORT_MODE_CLOCK_CFG(tgt), 0x00000002, 0x0000003b);
        //H2_WRITE_MASKED(VTSS_DEV_MAC_CFG_STATUS_MAC_MODE_CFG(tgt), 0x00000000, 0x00000010);
        H2_WRITE_MASKED(VTSS_DEV_CMN_MAC_CFG_STATUS_MAC_MODE_CFG(tgt), 0x00000001, 0x00000011);
        H2_WRITE_MASKED(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG(tgt), 0, 0x00000100);
        H2_WRITE_MASKED(VTSS_DEV_PCS_FX100_CONFIGURATION_PCS_FX100_CFG(tgt), 0x00000001, 0x07000001);

#elif defined(VTSS_ARCH_OCELOT)
        /* Speed setup and enable PCS clock */
        H2_WRITE_MASKED(VTSS_DEV_PORT_MODE_CLOCK_CFG(tgt),
                        VTSS_F_DEV_PORT_MODE_CLOCK_CFG_LINK_SPEED(2) /* 100 Mbps */,
                        VTSS_M_DEV_PORT_MODE_CLOCK_CFG_LINK_SPEED);
        h2_pcs1g_clock_set(chip_port, TRUE);

        /* Set MAC Mode Configuration */
        H2_WRITE_MASKED(VTSS_DEV_MAC_CFG_STATUS_MAC_MODE_CFG(tgt),
                        VTSS_F_DEV_MAC_CFG_STATUS_MAC_MODE_CFG_GIGA_MODE_ENA(0) |
                        VTSS_F_DEV_MAC_CFG_STATUS_MAC_MODE_CFG_FDX_ENA(1),
                        VTSS_M_DEV_MAC_CFG_STATUS_MAC_MODE_CFG_GIGA_MODE_ENA |
                        VTSS_M_DEV_MAC_CFG_STATUS_MAC_MODE_CFG_FDX_ENA);  // 10/100 & fdx

        /* Set PCS1G Auto-negotiation configuration: Software Resolve Abilities */
        H2_WRITE_MASKED(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG(tgt),
                        VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG_SW_RESOLVE_ENA(0),
                        VTSS_M_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG_SW_RESOLVE_ENA);

        /* Set Configuration bit groups for 100Base-FX PCS: Enable 100FX PCS */
        H2_WRITE_MASKED(VTSS_DEV_PCS_FX100_CONFIGURATION_PCS_FX100_CFG(tgt),
                        VTSS_F_DEV_PCS_FX100_CONFIGURATION_PCS_FX100_CFG_PCS_ENA(1),
                        VTSS_M_DEV_PCS_FX100_CONFIGURATION_PCS_FX100_CFG_PCS_ENA);
#endif

        return;
    }

    default:
#if defined(H2_PCS1G_DEBUG_ENABLE)
        print_str("%% Error: Wrong parameter when calling h2_pcs1g_setup(), chip_port=0x");
        print_hex_b(chip_port);
        print_str(", mode=0x");
        print_hex_b(if_type);
        print_cr_lf();
#endif /* H2_PCS1G_DEBUG_ENABLE */
        break;
    }
}

/* Get the PCS1G link status */
uchar h2_pcs1g_2_5g_link_status_get(vtss_cport_no_t chip_port)
{
    ulong tgt = VTSS_TO_DEV(chip_port);
    ulong reg_val;
    uchar link_mode = LINK_MODE_DOWN;

#if defined(VTSS_ARCH_LUTON26)
    // Luton26, TODO
#elif defined(VTSS_ARCH_OCELOT)
    /* Read PCS1G sticky register */
    H2_READ(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_STICKY(tgt), reg_val);
    if (VTSS_X_DEV_PCS1G_CFG_STATUS_PCS1G_STICKY_OUT_OF_SYNC_STICKY(reg_val)) {
        /* Clear sticky bit then re-enable PCS */
        H2_WRITE_MASKED(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_STICKY(tgt),
                        VTSS_X_DEV_PCS1G_CFG_STATUS_PCS1G_STICKY_OUT_OF_SYNC_STICKY(1),
                        VTSS_M_DEV_PCS1G_CFG_STATUS_PCS1G_STICKY_OUT_OF_SYNC_STICKY);
        H2_WRITE_MASKED(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_CFG(tgt),
                        VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_CFG_PCS_ENA(0),
                        VTSS_M_DEV_PCS1G_CFG_STATUS_PCS1G_CFG_PCS_ENA);
        delay_1(5);
        H2_WRITE_MASKED(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_CFG(tgt),
                        VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_CFG_PCS_ENA(1),
                        VTSS_M_DEV_PCS1G_CFG_STATUS_PCS1G_CFG_PCS_ENA);
    }

    H2_READ(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_STICKY(tgt), reg_val);
    if (VTSS_X_DEV_PCS1G_CFG_STATUS_PCS1G_STICKY_LINK_DOWN_STICKY(reg_val)) {
        /* The link has been down. Clear sticky bit by writing value 1 */
        H2_WRITE_MASKED(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_STICKY(tgt),
                        VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_STICKY_LINK_DOWN_STICKY(1),
                        VTSS_M_DEV_PCS1G_CFG_STATUS_PCS1G_STICKY_LINK_DOWN_STICKY);
    }

    /* Read PCS1G link status register */
    H2_READ(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_LINK_STATUS(tgt), reg_val);
    if (VTSS_X_DEV_PCS1G_CFG_STATUS_PCS1G_LINK_STATUS_LINK_STATUS(reg_val)) {
        link_mode = LINK_MODE_FDX_2500;
    }
#endif

    return link_mode;
}

#endif // MAC_TO_MEDIA
