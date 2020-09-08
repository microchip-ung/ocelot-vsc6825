//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#ifndef __VERIPHY_H__
#define __VERIPHY_H__

typedef struct {
    uchar       stat [4];     /* status for pairs A-D (0-3), 4-bit unsigned number, see 4.3.8, table 58 */
    uchar       loc  [4];     /* length/fault location for pairs A-D (0-3), 6-bit unsgn */
    uchar       flags;
} veriphy_parms_t;

void veriphy_start              (vtss_port_no_t port_no);
uchar veriphy_run               (vtss_port_no_t port_no, veriphy_parms_t *veriphy_parms_ptr, BOOL *done);
ulong vtss_phy_veriphy_running  (vtss_port_no_t port_no, BOOL set, uchar running);

#endif

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
