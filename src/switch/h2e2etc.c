//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#include "common.h"     /* Always include common.h at the first place of user-defined herder files */
#include "h2e2etc.h"
#include "h2io.h"
#include "vtss_api_base_regs.h"
#include "print.h"
#include "phydrv.h"
#include "hwport.h"
#include "h2tcam.h"

#if TRANSIT_E2ETC

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
/*****************************************************************************
 *
 *
 * Local data
 *
 *
 *
 ****************************************************************************/

/* Default latencies for different speeds */
static u16 ingress_latency[] = {
    //[LINK_MODE_SPEED_10]   
    71+100,
    //[LINK_MODE_SPEED_100]  
    71+100,
    //[LINK_MODE_SPEED_1000] 
    71+16,
    //[LINK_MODE_SPEED_2500] 
    23+8
   };
static u16 egress_latency[] = {
    //[LINK_MODE_SPEED_10]   
    793+100,
    //[LINK_MODE_SPEED_100]  
    793+100,
    //[LINK_MODE_SPEED_1000] 
    153+16,
    //[LINK_MODE_SPEED_2500] 
     67+8
};
 
/* =================================================================
 * E2E - End to end transparent clock
 * =================================================================
 * Initialize PTP in CHIP
 *
 * Return : None
 * =================================================================
 */
void h2_e2etc_init(void)
{
    vtss_iport_no_t port_ext;
    e2etc_conf_t conf;

    conf.e2etc_ena =  TRUE;
    for (port_ext = 1; port_ext <= NO_OF_BOARD_PORTS; port_ext++) {
        if (port_ext > 8 && port_ext <11) {
            /* Using default latencies for 1G ,adopted from vtss_serval_ts.c */
            conf.ingress_latency = 71 + 16;
            conf.egress_latency = 153 + 16;
        } else {
            /* Using default latencies for 2.5G ,adopted from vtss_serval_ts.c */
            conf.ingress_latency = 23 + 8;
            conf.egress_latency = 67 + 8;
        }
        h2_e2etc_conf_set(uport2cport(port_ext), &conf);
    }
    /* Create ACL to match any PTP_ETH and PTP_IP_UDP */
    h2_tcam_e2e_tc_set(TCAM_E2E_TC_PTP_ETH);
    h2_tcam_e2e_tc_set(TCAM_E2E_TC_PTP_IP);
    h2_tcam_e2e_tc_set(TCAM_E2E_TC_PTP_IPV6);
}

void h2_e2etc_conf_set(const uchar chip_port,
        const e2etc_conf_t * const conf) {

    /* Enable or disable PTP */
    H2_WRITE_MASKED(VTSS_DEVCPU_PTP_PTP_CFG_PTP_MISC_CFG,
            VTSS_F_DEVCPU_PTP_PTP_CFG_PTP_MISC_CFG_PTP_ENA(conf->e2etc_ena),
            VTSS_M_DEVCPU_PTP_PTP_CFG_PTP_MISC_CFG_PTP_ENA);
    if (conf->e2etc_ena) {
        /* Configuring defult ingress latency */
        H2_WRITE_MASKED(VTSS_DEV_PORT_MODE_RX_PATH_DELAY(VTSS_TO_DEV(chip_port)),
                VTSS_F_DEV_PORT_MODE_RX_PATH_DELAY_RX_PATH_DELAY(conf->ingress_latency),
                VTSS_M_DEV_PORT_MODE_RX_PATH_DELAY_RX_PATH_DELAY);
        /* Configuring defult egress latency */
        H2_WRITE_MASKED(VTSS_DEV_PORT_MODE_TX_PATH_DELAY(VTSS_TO_DEV(chip_port)),
                VTSS_F_DEV_PORT_MODE_TX_PATH_DELAY_TX_PATH_DELAY(conf->egress_latency),
                VTSS_M_DEV_PORT_MODE_TX_PATH_DELAY_TX_PATH_DELAY);
    }
}

void e2etc_latency_set(const uchar chip_port)
{
    uchar speed_and_fdx = phy_get_speed_and_fdx(chip_port);
    uchar speed = speed_and_fdx & LINK_MODE_SPEED_MASK;

    /* Configuring defult ingress latency */
    H2_WRITE_MASKED(VTSS_DEV_PORT_MODE_RX_PATH_DELAY(VTSS_TO_DEV(chip_port)),
            VTSS_F_DEV_PORT_MODE_RX_PATH_DELAY_RX_PATH_DELAY(ingress_latency[speed]),
            VTSS_M_DEV_PORT_MODE_RX_PATH_DELAY_RX_PATH_DELAY);
    /* Configuring defult egress latency */
    H2_WRITE_MASKED(VTSS_DEV_PORT_MODE_TX_PATH_DELAY(VTSS_TO_DEV(chip_port)),
            VTSS_F_DEV_PORT_MODE_TX_PATH_DELAY_TX_PATH_DELAY(egress_latency[speed]),
            VTSS_M_DEV_PORT_MODE_TX_PATH_DELAY_TX_PATH_DELAY);
}
#endif
