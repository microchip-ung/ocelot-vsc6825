//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#ifndef LLDP_PRIVATE_H
#define LLDP_PRIVATE_H

#include "lldp_os.h"

lldp_sm_t xdata * lldp_get_port_sm (lldp_port_t port);
void lldp_tx_initialize_lldp (lldp_sm_t xdata * sm);
void lldp_construct_info_lldpdu(lldp_port_t port_no);
void lldp_construct_shutdown_lldpdu(lldp_port_t port_no);
void lldp_tx_frame(lldp_port_t port_no);
void lldp_rx_initialize_lldp (lldp_port_t port);
void lldp_rx_process_frame (lldp_sm_t xdata * sm);
#endif



