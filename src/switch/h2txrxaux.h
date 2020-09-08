//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#ifndef __TXRX_AUX_H__
#define __TXRX_AUX_H__

#include "h2packet.h"

extern void   h2_rx_frame_get (uchar qno, vtss_rx_frame_t xdata * rx_frame_ptr);
extern bool   h2_tx_frame_port(const uchar port_no,
                               const uchar *const frame,
                               const ushort length,
                               const vtss_vid_t vid);
extern void   h2_discard_frame( vtss_rx_frame_t xdata * rx_frame_ptr);

#endif
