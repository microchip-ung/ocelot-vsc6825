//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#ifndef __H2TXRX_H__
#define __H2TXRX_H__

#include "common.h"
#include "h2packet.h"

void h2_rx_init (void);
void rx_packet_tsk (void);
extern uchar xdata rx_packet[];
extern vtss_rx_frame_t vtss_rx_frame;
extern uchar xdata rx_packet[];

void   h2_bpdu_t_registration (uchar type, uchar enable);
void   h2_rx_conf_set(void);
void   h2_rx_flush (void) small;
void   h2_send_frame (uchar port_no, uchar xdata *frame_ptr, ushort frame_len);

#endif








