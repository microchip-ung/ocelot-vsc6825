//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#ifndef __PHYMAP_H__
#define __PHYMAP_H__

#if 0   // Uncalled function
uchar phy_map_coma_mode_disable (vtss_port_no_t port_no) small;
#endif  // Uncalled function
uchar phy_map_miim_no           (vtss_port_no_t port_no) small;
uchar phy_map_phy_no            (vtss_port_no_t port_no) small;
bool  phy_map                   (vtss_port_no_t port_no) small;
uchar phy_map_serdes            (vtss_port_no_t port_no) small;

// Update serdes interface mode when sfp_detect(chip_port) != phy_map_miim_no(chip_port)
void  phy_map_serdes_if_update  (vtss_port_no_t port_no, uchar mac_if) small;

// Restore serdes interface mode (when serdes interface mode is configured as Auto selection via SFP info.
// The value is 0xa (See the definition of CPORT_MAPTO_MIIMBUS in src/config/phyconf.h)
void phy_map_serdes_if_restore(vtss_cport_no_t chip_port) small;

#endif

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
