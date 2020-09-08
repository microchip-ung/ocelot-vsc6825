//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#ifndef LLDP_H
#define LLDP_H

#include "lldp_os.h"

#define LLDP_ETHTYPE 0x88CC

/* IEEE 802.3 OUI */
extern const uchar ieee_802_3_oui_header[3];

#if TRANSIT_LLDP
extern const mac_addr_t xdata mac_addr_lldp;
#endif /* TRANSIT_LLDP */

void lldp_init (void);
void lldp_set_port_enabled (lldp_port_t port, lldp_u8_t enabled);
void lldp_1sec_timer_tick (void);
void lldp_frame_received (lldp_port_t port_no, lldp_u8_t xdata * frame, lldp_u16_t len);
void lldp_pre_port_disabled (lldp_port_t port_no);
void lldp_set_timing_changed (void);

#if TRANSIT_UNMANAGED_SYS_MAC_CONF || defined(TRANSIT_WEB)
void lldp_something_changed_local (void);
#endif /* TRANSIT_UNMANAGED_SYS_MAC_CONF || defined(TRANSIT_WEB) */

lldp_counter_t lldp_get_tx_frames (lldp_port_t port);
lldp_counter_t lldp_get_rx_total_frames (lldp_port_t port);
lldp_counter_t lldp_get_rx_error_frames (lldp_port_t port);
lldp_counter_t lldp_get_rx_discarded_frames (lldp_port_t port);
lldp_counter_t lldp_get_TLVs_discarded (lldp_port_t port);
lldp_counter_t lldp_get_TLVs_unrecognized (lldp_port_t port);
lldp_counter_t lldp_get_TLVs_org_discarded (lldp_port_t port);
lldp_counter_t lldp_get_ageouts (lldp_port_t port);
#endif


