//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#include "common.h"     /* Always include common.h at the first place of user-defined herder files */
#include "phymap.h"
#include "phytsk.h"
#include "vtss_api_base_regs.h"
#include "h2io.h"
#include "main.h"
#include "hwport.h"
#include "misc2.h"

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
#if defined(VTSS_ARCH_OCELOT)
#define MULTIPLIER_BIT 256
static u16 wm_enc(u16 value)
{
    if (value >= MULTIPLIER_BIT) {
        return MULTIPLIER_BIT + value / 16;
    }
    return value;
}

/*****************************************************************************
 *
 *
 * Local data
 *
 *
 *
 ****************************************************************************/
/* Buffer constants */
#define FERRET_BUFFER_MEMORY    229380
#define FERRET_BUFFER_REFERENCE 1911
#define FERRET_BUFFER_CELL_SZ   60
#define VTSS_PRIOS              8

#if 0 //BZ#21585 - Ferret watermark setting is in h2.c/_ferret_buf_conf_set()
/* Priority-based flow control configuration
 * It based on the main trunk Serval-1 vtss_serval_port.c/srvl_port_pfc() implementation
 */
static void _ferret_port_pfc(vtss_cport_no_t chip_port, uchar link_mode)
{
    u32 buf_q_rsrv_i = 3000;
    u32 buf_q_rsrv_i_def = 3000; // Default
    u32 q, pfc_mask = 0x0, pfc = 0;
    u32 speed = (link_mode & LINK_MODE_SPEED_MASK);
    u32 spd = (speed == LINK_MODE_SPEED_10 ? 3 :
               speed == LINK_MODE_SPEED_100 ? 2 :
               speed == LINK_MODE_SPEED_1000 ? 1 : 0);

    // pfc_mask = 0xff; // Enable 8 queues
    // pfc = 1;         // Enable priority-based flow control

    for (q = 0; q < VTSS_PRIOS; q++) {
        H2_WRITE(VTSS_QSYS_RES_CTRL_RES_CFG(chip_port * VTSS_PRIOS + q + 0),
                 wm_enc((pfc ? buf_q_rsrv_i : buf_q_rsrv_i_def) / FERRET_BUFFER_CELL_SZ));
    }

    /* Rx enable/disable */
    H2_WRITE_MASKED(VTSS_ANA_PFC_PFC_CFG(chip_port), 
                    VTSS_F_ANA_PFC_PFC_CFG_RX_PFC_ENA(pfc_mask) |
                    VTSS_F_ANA_PFC_PFC_CFG_FC_LINK_SPEED(spd),
                    VTSS_M_ANA_PFC_PFC_CFG_RX_PFC_ENA |
                    VTSS_M_ANA_PFC_PFC_CFG_FC_LINK_SPEED);
    
    /* Forward 802.1Qbb pause frames to analyzer */
    H2_WRITE_MASKED(VTSS_DEV_PORT_MODE_PORT_MISC(VTSS_TO_DEV(chip_port)), 
                    VTSS_BOOL(pfc_mask) ? VTSS_F_DEV_PORT_MODE_PORT_MISC_FWD_CTRL_ENA(1) : 0,
                    VTSS_M_DEV_PORT_MODE_PORT_MISC_FWD_CTRL_ENA);
 
    /*  PFC Tx enable is done after the core is enabled */
}
#endif // BZ#21585

/* Port flow control configuration
 * It based on the main trunk Serval-1 vtss_serval_port.c/srvl_port_fc_setup() implementation
 */
static void _ferret_port_fc_setup(vtss_cport_no_t chip_port, uchar link_mode, uchar fc_enabled)
{
    BOOL            pfc = 0, fc_gen = 0, fc_obey = 0;
    vtss_iport_no_t iport_idx;
    u32             rsrv_raw, rsrv_total = 0, atop_wm, tgt = VTSS_TO_DEV(chip_port);
    u32             pause_start = 0x7ff;
    u32             pause_stop  = 0xfff;
    u32             rsrv_raw_fc_jumbo = 40000;
    u32             rsrv_raw_no_fc_jumbo = 12000;
    u32             rsrv_raw_fc_no_jumbo = 13662; /* 9 x 1518 */
    u32             speed = (link_mode & LINK_MODE_SPEED_MASK);
    u32             spd = (speed == LINK_MODE_SPEED_10 ? 3 :
                           speed == LINK_MODE_SPEED_100 ? 2 :
                           speed == LINK_MODE_SPEED_1000 ? 1 : 0);
    mac_addr_t mac_addr;

    /* Notice: 802.3X FC and 802.1Qbb PFC cannot both be enabled */
    //pfc = 1; // Enable priority-based flow control
    fc_gen = fc_enabled, fc_obey = fc_enabled;

    /* Configure 802.1Qbb PFC */
#if 0 //BZ#21585 - Ferret watermark setting is in h2.c/_ferret_buf_conf_set()
    _ferret_port_pfc(chip_port, link_mode);
#endif // BZ#21585

    /* Configure Standard Flowcontrol */
#if (MAX_FRAME_SIZE > VTSS_MAX_FRAME_LENGTH_STANDARD)
    pause_start = 228;     /* 9 x 1518 / 60 */
    if (fc_gen) {
        /* FC and jumbo enabled*/
        pause_stop  = 177;     /* 7 x 1518 / 60 */
        rsrv_raw    = rsrv_raw_fc_jumbo;   
    } else {
        /* FC disabled, jumbo enabled */
        rsrv_raw = rsrv_raw_no_fc_jumbo;
    }
#else
    pause_start = 152;    /* 6 x 1518 / 60 */
    if (fc_gen) {
    /* FC enabled, jumbo disabled */
        pause_stop  = 101;    /* 4 x 1518 / 60 */
        rsrv_raw    = rsrv_raw_fc_no_jumbo;  
    } else {
        rsrv_raw    = 0;
    }
#endif // (MAX_FRAME_SIZE > VTSS_MAX_FRAME_LENGTH_STANDARD)

    if (pfc) {
        rsrv_raw = 80000; // Each port can use this as max before tail dropping starts
    }

    /* Set Pause WM hysteresis*/
    H2_WRITE(VTSS_SYS_PAUSE_CFG_PAUSE_CFG(chip_port),
             VTSS_F_SYS_PAUSE_CFG_PAUSE_CFG_PAUSE_START(pause_start) |
             VTSS_F_SYS_PAUSE_CFG_PAUSE_CFG_PAUSE_STOP(pause_stop) |
             VTSS_F_SYS_PAUSE_CFG_PAUSE_CFG_PAUSE_ENA(fc_gen));
    
    /* Set SMAC of Pause frame */
    get_mac_addr(chip_port, mac_addr);
    H2_WRITE(VTSS_DEV_MAC_CFG_STATUS_MAC_FC_MAC_HIGH_CFG(tgt),(mac_addr[0]<<16) | (mac_addr[1]<<8) | mac_addr[2]);
    H2_WRITE(VTSS_DEV_MAC_CFG_STATUS_MAC_FC_MAC_LOW_CFG(tgt), (mac_addr[3]<<16) | (mac_addr[4]<<8) | mac_addr[5]);

    /* Enable/disable FC incl. pause value and zero pause */

    H2_WRITE(VTSS_SYS_PAUSE_CFG_MAC_FC_CFG(chip_port),
             VTSS_F_SYS_PAUSE_CFG_MAC_FC_CFG_FC_LINK_SPEED(spd) |
             VTSS_F_SYS_PAUSE_CFG_MAC_FC_CFG_FC_LATENCY_CFG(7) |
             VTSS_F_SYS_PAUSE_CFG_MAC_FC_CFG_ZERO_PAUSE_ENA(1) |
             VTSS_F_SYS_PAUSE_CFG_MAC_FC_CFG_TX_FC_ENA(fc_gen) |
             VTSS_F_SYS_PAUSE_CFG_MAC_FC_CFG_RX_FC_ENA(fc_obey) |
             VTSS_F_SYS_PAUSE_CFG_MAC_FC_CFG_PAUSE_VAL_CFG(pfc ? 0xff : 0xffff));


    H2_WRITE_MASKED(VTSS_QSYS_SYSTEM_SWITCH_PORT_MODE(chip_port),
                    VTSS_F_QSYS_SYSTEM_SWITCH_PORT_MODE_INGRESS_DROP_MODE(!fc_gen),
                    VTSS_M_QSYS_SYSTEM_SWITCH_PORT_MODE_INGRESS_DROP_MODE);

  
    /* Calculate the total reserved space for all ports */
    for (iport_idx = MIN_PORT; iport_idx < MAX_PORT; iport_idx++) {
        uchar lm = port_link_mode_get(iport2cport(iport_idx));
        fc_gen = (lm & LINK_MODE_PAUSE_MASK) ? 1 : 0;

#if (MAX_FRAME_SIZE > VTSS_MAX_FRAME_LENGTH_STANDARD)
        if (fc_gen) {
            rsrv_total += rsrv_raw_fc_jumbo;
        } else {
            rsrv_total += rsrv_raw_no_fc_jumbo;
        }
#else
        if (fc_gen) {
            rsrv_total += rsrv_raw_fc_no_jumbo;
        }
#endif // (MAX_FRAME_SIZE > VTSS_MAX_FRAME_LENGTH_STANDARD)
    }
    atop_wm = (FERRET_BUFFER_MEMORY - rsrv_total)/FERRET_BUFFER_CELL_SZ;

    /*  When 'port ATOP' and 'common ATOP_TOT' are exceeded, tail dropping is activated on port */
        /* HOL blocking issue on Ferret
     * According to Morten's suggestion below.
     * I think both should be set to their max value, when the port isn¡¦t running
     * with flow control enabled. Or ¡V simply always set atop_tot_cfg to the max
     * value. The ATOP watermark is only there to protect the memory from extreme
     * use by ports that are enabled for flow control, but don¡¦t obey the pause
     * frames. 
     * My suggestion; keep the source code as is, but set atop_tot_cfg to max
     * unconditionally.
    H2_WRITE(VTSS_SYS_PAUSE_CFG_ATOP_TOT_CFG, pfc ? 0 : wm_enc(atop_wm)); */
    H2_WRITE(VTSS_SYS_PAUSE_CFG_ATOP_TOT_CFG, pfc ? 0 : 0x1FF);
    H2_WRITE(VTSS_SYS_PAUSE_CFG_ATOP(chip_port), wm_enc(rsrv_raw / FERRET_BUFFER_CELL_SZ));
}
#endif // VTSS_ARCH_OCELOT

/* ************************************************************************ */
void h2_setup_flow_control(vtss_cport_no_t chip_port, uchar link_mode)
/* ------------------------------------------------------------------------ --
 * Purpose     : Setup flow control according to configuration and link
 *               partner's advertisement.
 * Remarks     : Please see main.h for a description of link_mode.
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
#if defined(VTSS_ARCH_LUTON26)
    vtss_iport_no_t iport_idx;
    mac_addr_t mac_addr;
    ulong pause_start, pause_stop, rsrv_raw, rsrv_total, atop_wm;
    ulong tgt;
    uchar lm;
#endif // VTSS_ARCH_LUTON26

#if (!VTSS_ATOM12_A) && (!VTSS_ATOM12_B)
    /* check pause flag */
    uchar mask = LINK_MODE_PAUSE_MASK;
#else
    /* check BOTH pause flag and full duplex flag */
    /* Note: in some chipset revisions, half duplex backpressure do not work
       on SERDES port. */
    uchar mask = LINK_MODE_FDX_AND_PAUSE_MASK;
#endif // (!VTSS_ATOM12_A) && (!VTSS_ATOM12_B)

    BOOL local;
    uchar remote, fc;

    local  = phy_flowcontrol_get(chip_port); /* Local has always advertised support of pause frames */
    remote = ((link_mode & mask) == mask);
    fc     = (local && remote);

#if defined(VTSS_ARCH_LUTON26)
    rsrv_total  = 0;
    pause_start = 0x7ff;
    pause_stop  = 0x7ff;

#if defined(JUMBO)
    /* FC disabled and jumbo */
    rsrv_raw  = 250;      /* 12000 / 48 */

    if (fc) {
        /* FC enabled and jumbo */
        pause_start = 221;    /* 7 x 1518 / 48 */
        pause_stop  = 158;    /* 5 x 1518 / 48 */
        rsrv_raw    = 284;    /* 9 x 1518 / 48 */
    }
#else
    /* FC disabled and no jumbo */
    rsrv_raw = 0;

    if (fc) {
        /* FC enabled and no jumbo */
        pause_start = 190;    /* 6 x 1518 / 48 */
        pause_stop  = 127;    /* 4 x 1518 / 48 */
        rsrv_raw    = 253;    /* 8 x 1518 / 48 */
    }
#endif

    /* Calculate the total reserved space for all ports */
    for (iport_idx = MIN_PORT; iport_idx < MAX_PORT; iport_idx++) {
        lm     = port_link_mode_get(iport2cport(iport_idx));
        remote = ((lm & mask) == mask);
#if defined(JUMBO)
        if (local && remote) {
            rsrv_total +=  13662; /* 9*1518 */
        } else {
            rsrv_total +=  12000;
        }
#else
        if (local && remote) {
            rsrv_total +=  12144; /* 8*1518 */
        }
#endif
    }

    /* Set Pause WM hysteresis*/
    H2_WRITE(VTSS_SYS_PAUSE_CFG_PAUSE_CFG(chip_port),
             VTSS_F_SYS_PAUSE_CFG_PAUSE_CFG_PAUSE_START(pause_start) |
             VTSS_F_SYS_PAUSE_CFG_PAUSE_CFG_PAUSE_STOP(pause_stop) |
             (fc ? VTSS_F_SYS_PAUSE_CFG_PAUSE_CFG_PAUSE_ENA : 0));

    atop_wm = (512000 - rsrv_total)/48;
    if (atop_wm >= 1024UL) {
        atop_wm = 1024UL + atop_wm/16;
    }

    /*  When 'port ATOP' and 'common ATOP_TOT' are exceeded, tail dropping is activated on a port */
    H2_WRITE(VTSS_SYS_PAUSE_CFG_ATOP_TOT_CFG, atop_wm);
    H2_WRITE(VTSS_SYS_PAUSE_CFG_ATOP(chip_port), rsrv_raw);


    /* Set SMAC of Pause frame */
    get_mac_addr(chip_port, mac_addr);
    tgt = VTSS_TO_DEV(chip_port); 
    H2_WRITE(VTSS_DEV_CMN_MAC_CFG_STATUS_MAC_FC_MAC_HIGH_CFG(tgt),(mac_addr[0]<<16) | (mac_addr[1]<<8) | mac_addr[2]);
    H2_WRITE(VTSS_DEV_CMN_MAC_CFG_STATUS_MAC_FC_MAC_LOW_CFG(tgt), (mac_addr[3]<<16) | (mac_addr[4]<<8) | mac_addr[5]);

    /* Enable/disable FC incl. pause value and zero pause */
    H2_WRITE(VTSS_DEV_CMN_MAC_CFG_STATUS_MAC_FC_CFG(tgt),
             VTSS_F_DEV_MAC_CFG_STATUS_MAC_FC_CFG_ZERO_PAUSE_ENA |
             (fc ? VTSS_F_DEV_MAC_CFG_STATUS_MAC_FC_CFG_TX_FC_ENA : 0) |
             (fc ? VTSS_F_DEV_MAC_CFG_STATUS_MAC_FC_CFG_RX_FC_ENA  : 0) |
             VTSS_F_DEV_MAC_CFG_STATUS_MAC_FC_CFG_PAUSE_VAL_CFG(0xff) |
             VTSS_F_DEV_MAC_CFG_STATUS_MAC_FC_CFG_FC_LATENCY_CFG(63));/* changed from 7 to 63 : disable flow control latency */

#elif defined(VTSS_ARCH_OCELOT)
    _ferret_port_fc_setup(chip_port, link_mode, fc);
#endif // VTSS_ARCH_OCELOT
}
