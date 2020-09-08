//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT

#ifndef __H2AGGR_H__
#define __H2AGGR_H__

#define MAX_KEY   16

void   h2_aggr_init (void);
void   h2_aggr_set_mode (uchar mode);
uchar h2_aggr_add (port_bit_mask_t i_port_mask);
uchar  h2_aggr_delete (port_bit_mask_t port_mask);
void   h2_aggr_update (void);
void   h2_aggr_setup (uchar port_no);
uchar  h2_aggr_find_group (port_bit_mask_t port_mask);
#if TRANSIT_RSTP_TRUNK_COWORK && TRANSIT_RSTP
uchar chk_staggr_member (uchar pno);
#endif

#define ERROR_AGGR_1 1
#define ERROR_AGGR_2 2
#define ERROR_AGGR_3 3
#define ERROR_AGGR_4 4

#endif
