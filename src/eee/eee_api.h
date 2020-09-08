//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#ifndef _EEE_API_H_
#define _EEE_API_H_

#include "h2eee.h"
#include "eee_base_api.h"

//************************************************
// Configuration definition
//************************************************

// Switch configuration (configuration that are local for a switch in the stack)
typedef struct {
    eee_port_conf_t port[NO_OF_BOARD_PORTS];
} eee_conf_t;



//************************************************
// Constants
//************************************************

extern const ushort PHY_WAKEUP_VALUE_1G;
extern const ushort PHY_WAKEUP_VALUE_100;

#if TRANSIT_EEE_LLDP
extern eee_sm_t   eee_sm[NO_OF_BOARD_PORTS];
#endif


void eee_mgmt_int (void);
void eee_mgmt (void);

uchar  read_eee_conf_mode (uchar port_ext);
ushort read_eee_conf_tx_tw (uchar port_ext);
ushort read_eee_conf_rx_tw (uchar port_ext);
uchar  read_eee_conf_fast_queues (uchar port_ext);

void write_eee_conf_mode (uchar port_ext, uchar mode);
void write_eee_conf_tx_tw (uchar port_ext, ushort time);
void write_eee_conf_rx_tw (uchar port_ext, ushort time);
void write_conf_fast_queues (uchar port_ext, uchar fast_queues);
void eee_port_mode_setup(uchar iport);
void eee_port_link_change(uchar iport, BOOL up);
void load_eee_conf (void);
void save_eee_conf (void);

#endif // _EEE_API_H_


//***************************************************************************
//  End of file.
//***************************************************************************
