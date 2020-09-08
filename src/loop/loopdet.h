//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#ifndef __LOOP_DET_H__
#define __LOOP_DET_H__

void                ldettsk             (void);
void                ldet_aging_100ms    (void);
void                ldet_aging_1s       (void);
void                ldet_add_cpu_found  (vtss_port_no_t i_port_no);
port_bit_mask_t     ldet_blocked_ports  (void);

#endif /* __LOOP_DET_H__ */
