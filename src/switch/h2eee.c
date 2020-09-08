//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#include "common.h"     /* Always include common.h at the first place of user-defined herder files */
#include "timer.h"
#include "misc2.h"
#include "h2io.h"
#include "vtss_api_base_regs.h"
#include "h2eee.h"
#include "print.h"
#include "phydrv.h"

#if TRANSIT_EEE

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
 * Public data
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
#if 0
static uchar dec16hex(ushort value);
#endif
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
 * Luton26 802.3az functions
 *
 *
 *
 ****************************************************************************/


/* =================================================================
 *  EEE - Energy Efficient Ethernet
 * =================================================================
 * Set EEE configuration
 *
 * In :  port - The iport number for which to set new configuration.
 *       conf  - New configuration for the port
 *
 * Return : None
 * =================================================================
 */
#if defined(VTSS_ARCH_OCELOT) 
void h2_eee_port_conf_set(const vtss_cport_no_t chip_port,
                          const eee_port_conf_t * const conf) {

    ulong E, time;
    ulong eee_cfg_reg = 0x0; // SYS::EEE_CFG register value.

    // EEE enable
    if (conf->eee_ena) {
        //LPI is really an old error code redefined to tell the link partner to go to
        // lower power or when removed to wakeup.
        // Some devices are seeing the error code and assuming there is a
        // problem causing the link to go down. A work around is to only enable EEE in the MAC (No LPI signal to the PHY)
        // when the PHY has auto negotiated and have found that the link partner supports EEE.
        if (conf->lp_advertisement == 0) {
            // Link partner doesn't support EEE - Keeping EEE disabled
        } else {
            eee_cfg_reg |= VTSS_F_DEV_PORT_MODE_EEE_CFG_EEE_ENA(1);
        }
    }

    // EEE wakeup timer (See datasheet for understanding calculation)
    time = conf->tx_tw;
    E = 0;
    while (time > 16) {
        E++;
        time /= 4;
    }

    time += 1; // Round up to make sure that we always have longer wakeup time.
    eee_cfg_reg |= VTSS_F_DEV_PORT_MODE_EEE_CFG_EEE_TIMER_WAKEUP(E*16+time);


    // EEE holdoff timer
    eee_cfg_reg |= VTSS_F_DEV_PORT_MODE_EEE_CFG_EEE_TIMER_HOLDOFF(0x5) | VTSS_F_DEV_PORT_MODE_EEE_CFG_EEE_TIMER_AGE(0x23);

    // EEE fast queues
    eee_cfg_reg |= VTSS_F_QSYS_SYSTEM_EEE_CFG_EEE_FAST_QUEUES(conf->eee_fast_queues);

    // Registers write
    H2_WRITE(VTSS_QSYS_SYSTEM_EEE_CFG(chip_port),eee_cfg_reg);

    // Setting Buffer size to 3 Kbyte & 255 frames.
    H2_WRITE(VTSS_QSYS_SYSTEM_EEE_THRES, 0x3EFF);

}
#elif defined(VTSS_ARCH_LUTON26)
void h2_eee_port_conf_set(const vtss_cport_no_t chip_port,
                          const eee_port_conf_t * const conf) {

    ulong E, time;
    ulong eee_cfg_reg = 0x0; // SYS::EEE_CFG register value.

    // EEE enable
    if (conf->eee_ena) {
        //LPI is really an old error code redefined to tell the link partner to go to
        // lower power or when removed to wakeup.
        // Some devices are seeing the error code and assuming there is a
        // problem causing the link to go down. A work around is to only enable EEE in the MAC (No LPI signal to the PHY)
        // when the PHY has auto negotiated and have found that the link partner supports EEE.
        if (conf->lp_advertisement == 0) {
            // Link partner doesn't support EEE - Keeping EEE disabled
        } else {
            eee_cfg_reg |= VTSS_F_SYS_SYSTEM_EEE_CFG_EEE_ENA;
        }
    }

    // EEE wakeup timer (See datasheet for understanding calculation)
    time = conf->tx_tw;
    E = 0;
    while (time > 16) {
        E++;
        time /= 4;
    }

    time += 1; // Round up to make sure that we always have longer wakeup time.
    eee_cfg_reg |= VTSS_F_SYS_SYSTEM_EEE_CFG_EEE_TIMER_WAKEUP(E*16+time);


    // EEE holdoff timer
    eee_cfg_reg |= VTSS_F_SYS_SYSTEM_EEE_CFG_EEE_TIMER_HOLDOFF(0x5) | VTSS_F_SYS_SYSTEM_EEE_CFG_EEE_TIMER_AGE(0x23);

    // EEE fast queues
    eee_cfg_reg |= VTSS_F_SYS_SYSTEM_EEE_CFG_EEE_FAST_QUEUES(conf->eee_fast_queues);

    // Registers write
    H2_WRITE(VTSS_SYS_SYSTEM_EEE_CFG(chip_port),eee_cfg_reg);

    // Setting Buffer size to 3 Kbyte & 255 frames.
    H2_WRITE(VTSS_SYS_SYSTEM_EEE_THRES, 0x3EFF);

}
#endif

#if 0
static uchar dec16hex(ushort value)
{
    const ushort perm4[8] = {1, 4, 16, 64, 256, 1024, 4096, 16384};
    uchar i, j;
    uchar idx = 0, jdx = 0;
    for(i = 0; i < 8; i++) {
        if(perm4[i] >= value) {
            idx = i - 1;
            break;
        }
    }
    for(j = 0; j < 16; j++) {
        if(perm4[idx]*j == value) {
            jdx = j;
            break;
        }
        if(perm4[idx]*j > value) {
            jdx = j - 1;
            break;
        }
    }
    return (idx << 4) | jdx;
}
#endif
#endif
