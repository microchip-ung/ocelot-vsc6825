//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#ifndef __H2PCS1G_H__
#define __H2PCS1G_H__

#if  MAC_TO_MEDIA
typedef enum {
    MAC_IF_INTERNAL         = 0,    /* MII to internal PHY */
    MAC_IF_EXTERNAL         = 1,    /* MII to external PHY */
    MAC_IF_SGMII            = 2,    /* SGMII directly via MAC */
    MAC_IF_UNCONNECTED      = 7,    /* Unconnected */
    MAC_IF_100FX            = 8,    /* 100FX directly via MAC */
    MAC_IF_SERDES_1G        = 9,    /* SerDes 1G directly via MAC */
    MAC_IF_SFP_MSA_DETECTED = 10,   /* Auto selection via SFP MSA info. */
    MAC_IF_SERDES_2_5G      = 11,   /* SerDes 2.5G directly via MAC */
    MAC_IF_UNKNOWN          = 0xff  /* Unknown type */
} mac_if_type_t;

void  h2_pcs1g_clause_37_control_set(const vtss_cport_no_t chip_port, BOOL flow_ctrl_mode);
uchar h2_pcs1g_clause_37_status_get(const vtss_cport_no_t chip_port);
uchar h2_pcs1g_100fx_status_get(const vtss_cport_no_t chip_port);
void  h2_pcs1g_clock_set(vtss_cport_no_t chip_port, BOOL enable);
void  h2_pcs1g_setup(vtss_cport_no_t chip_port, mac_if_type_t if_type);
#ifdef SGMII_SERDES_FORCE_1G_DEBUG_ENABLE
uchar h2_pcs1g_status_get(const vtss_cport_no_t chip_port);
#endif
/* Get the PCS1G link status */
uchar h2_pcs1g_2_5g_link_status_get(vtss_cport_no_t chip_port);

#endif // MAC_TO_MEDIA

#endif // __H2PCS1G_H__
