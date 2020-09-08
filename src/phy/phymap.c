//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#include "common.h"     /* Always include common.h at the first place of user-defined herder files */
#include "h2pcs1g.h"    // mac_if_type_t

/*****************************************************************************
 *
 *
 * Defines
 *
 *
 *
 ****************************************************************************/
#if defined(PHYMAP_DEBUG_ENABLE)
#include "print.h"
#endif /* PHYMAP_DEBUG_ENABLE */

/*****************************************************************************
 *
 *
 * Typedefs and enums
 *
 *
 *
 ****************************************************************************/

/*****************************************************************************
 *
 *
 * Prototypes for local functions
 *
 *
 *
 ****************************************************************************/


/*****************************************************************************
 *
 *
 * Local data
 *
 *
 *
 ****************************************************************************/

/*
** Mapping of PHYs on ports. Define for each port, the MIIM (0 or 1) to which
** the PHY is connected, and the PHY number.
*/
static code vtss_cport_no_t cport_to_miim_addr        [NO_OF_CHIP_PORTS] = CPORT_MAPTO_MIIMADDR;
static      vtss_cport_no_t cport_to_miim_bus         [NO_OF_CHIP_PORTS] = CPORT_MAPTO_MIIMBUS;
static code vtss_cport_no_t cport_to_coma_mode_disable[NO_OF_CHIP_PORTS] = CPORT_MAPTO_COMA_MODE_DISABLE;


/* ************************************************************************ */
uchar phy_map_miim_no (vtss_cport_no_t chip_port) small
/* ------------------------------------------------------------------------ --
 * Purpose     : Get MIIM number of the PHY attached to specified port.
 * Remarks     : Returns MIIM number.
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    if (chip_port < NO_OF_CHIP_PORTS) { // Avoid memory leak issue
        return cport_to_miim_bus[chip_port];
    }

#if defined(PHYMAP_DEBUG_ENABLE)
    print_str("%% Error: Wrong parameter when calling phy_map_miim_no(), chip_port=0x");
    print_hex_b(chip_port);
    print_cr_lf();
#endif /* PHYMAP_DEBUG_ENABLE */

    return 0xFF; // Failed case
}

#if 0   // Uncalled function
uchar phy_map_coma_mode_disable (vtss_cport_no_t chip_port) small
/* ------------------------------------------------------------------------ --
 * Purpose     : Get if PHY coma mode shall be disabled. Coma mode shall be
 *               disabled when the last PHY port has been initialized.
 *               For this to work it is assumed that the ports are initialized
 *               begin from the lowest phy port to the highest phy port.
 * Remarks     : Returns if coma mode shall be disabled..
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    if (chip_port < NO_OF_CHIP_PORTS) { // Avoid memory leak issue
        return cport_to_coma_mode_disable[chip_port];
    }

#if defined(PHYMAP_DEBUG_ENABLE)
    print_str("%% Error: Wrong parameter when calling phy_map_coma_mode_disable(), chip_port=0x");
    print_hex_b(chip_port);
    print_cr_lf();
#endif /* PHYMAP_DEBUG_ENABLE */

    return 0xFF; // Failed case
}
#endif  // Uncalled function

/* ************************************************************************ */
uchar phy_map_phy_no (vtss_cport_no_t chip_port) small
/* ------------------------------------------------------------------------ --
 * Purpose     : Get PHY number of the PHY attached to specified port.
 * Remarks     : Returns PHY number.
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    if (chip_port < NO_OF_CHIP_PORTS) { // Avoid memory leak issue
        return cport_to_miim_addr[chip_port];
    }

#if defined(PHYMAP_DEBUG_ENABLE)
    print_str("%% Error: Wrong parameter when calling phy_map_phy_no(), chip_port=0x");
    print_hex_b(chip_port);
    print_cr_lf();
#endif /* PHYMAP_DEBUG_ENABLE */

    return 0xFF; // Failed case
}



/* ************************************************************************ */
bool phy_map (vtss_cport_no_t chip_port) small
/* ------------------------------------------------------------------------ --
 * Purpose     : Check if a PHY is attached to an MIIM for the specified port.
 * Remarks     : Returns TRUE if so, otherwise FALSE
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    if (chip_port < NO_OF_CHIP_PORTS) { // Avoid memory leak issue
        return (cport_to_miim_bus[chip_port] == MAC_IF_INTERNAL ||
                cport_to_miim_bus[chip_port] == MAC_IF_EXTERNAL);
    }

#if defined(PHYMAP_DEBUG_ENABLE)
    print_str("%% Error: Wrong parameter when calling phy_map(), chip_port=0x");
    print_hex_b(chip_port);
    print_cr_lf();
#endif /* PHYMAP_DEBUG_ENABLE */

    return 0xFF; // Failed case
}


#if MAC_TO_MEDIA
/* ************************************************************************ */
uchar phy_map_serdes(vtss_cport_no_t chip_port)	small
/* ------------------------------------------------------------------------ --
 * Purpose     : Check if the PHY attached to specified port is a Serdes port.
 * Remarks     : Returns TRUE if Serdes port, otherwise FALSE
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    if (chip_port < NO_OF_CHIP_PORTS) { // Avoid memory leak issue
        return (cport_to_miim_bus[chip_port] == MAC_IF_SERDES_1G ||
                cport_to_miim_bus[chip_port] == MAC_IF_SERDES_2_5G ||
				cport_to_miim_bus[chip_port] == MAC_IF_100FX ||
				cport_to_miim_bus[chip_port] == MAC_IF_SGMII ||
				cport_to_miim_bus[chip_port] == MAC_IF_SFP_MSA_DETECTED);
    }

#if defined(PHYMAP_DEBUG_ENABLE)
    print_str("%% Error: Wrong parameter when calling phy_map_serdes(), chip_port=0x");
    print_hex_b(chip_port);
    print_cr_lf();
#endif /* PHYMAP_DEBUG_ENABLE */

    return 0xFF; // Failed case
}

#if TRANSIT_SFP_DETECT
void phy_map_serdes_if_update(vtss_cport_no_t chip_port, uchar mac_if) small
/* ------------------------------------------------------------------------ --
 * Purpose     : update if the SPF attached to specified port is a Serdes port.
 * Remarks     :
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    if (chip_port < NO_OF_CHIP_PORTS) { // Avoid memory leak issue
        cport_to_miim_bus[chip_port] = mac_if;
        return;
    }

#if defined(PHYMAP_DEBUG_ENABLE)
    print_str("%% Error: Wrong parameter when calling phy_map_serdes_if_update(), chip_port=0x");
    print_hex_b(chip_port);
    print_cr_lf();
#endif /* PHYMAP_DEBUG_ENABLE */
}
#endif // TRANSIT_SFP_DETECT

void phy_map_serdes_if_restore(vtss_cport_no_t chip_port) small
{
    vtss_cport_no_t cport_to_miim_bus_ori[NO_OF_CHIP_PORTS] = CPORT_MAPTO_MIIMBUS; // Original mapping

    if (chip_port < NO_OF_CHIP_PORTS) { // Avoid memory leak issue
        if (cport_to_miim_bus[chip_port] != cport_to_miim_bus_ori[chip_port]) {
            cport_to_miim_bus[chip_port] = cport_to_miim_bus_ori[chip_port];
        }
        return;
    }

#if defined(PHYMAP_DEBUG_ENABLE)
    print_str("%% Error: Wrong parameter when calling phy_map_serdes_if_restore(), chip_port=0x");
    print_hex_b(chip_port);
    print_cr_lf();
#endif /* PHYMAP_DEBUG_ENABLE */
}
#endif // MAC_TO_MEDIA

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
