//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#ifndef __PHY_BASE_H__
#define __PHY_BASE_H__

#include "common.h"     /* Always include common.h at the first place of user-defined herder files */
#include "phy_family.h"

/****************************************************************************
 *
 *
 * Defines
 *
 *
 ****************************************************************************/

#if VTSS_ATOM12 || VTSS_TESLA
#define FIRMWARE_START_ADDR 0x4000
#endif

/****************************************************************************
 *
 *
 * Typedefs and enums
 *
 *
 ****************************************************************************/


/****************************************************************************
 *
 *
 * Structures
 *
 *
 ****************************************************************************/


/****************************************************************************
 *
 *
 * Functions
 *
 *
 ****************************************************************************/

/**
 * Download micro patch to internal PHY micro-controller.
 *
 * @param   port_no     The lowest phy port for the chip in question.
 * @param   code_array  Pointer to array of op-codes.
 * @param   code_size   The size of the code in bytes.
 */
vtss_rc atom_download_8051_code
(
    vtss_port_no_t  port_no,
    uchar const     *code_array,
    uint            code_size
);

/**
 * Suspend the execution of micro patch in ATOM. This is needed when some
 * tasks like Veriphy is going to run.
 */
void atom_patch_suspend
(
    vtss_port_no_t  port_no,
    BOOL            suspend
);

/**
 * Wait for the micro code to complete.
 *
 * @param   port_no     port number staring from 0.
 */
vtss_rc vtss_phy_wait_for_micro_complete
(
    vtss_port_no_t  port_no
);

/**
 * Check the micro patch is downloaded correctly to the PHY micro controller.
 *
 * @param   port_no         The lowest phy port for the chip in question.
 * @param   code_length     The length of the micro patch code.
 * @param   expected_crc    The expected CRC for the micro patch code.
 */
vtss_rc vtss_phy_is_8051_crc_ok_private
(
    vtss_port_no_t  port_no,
    u16             start_addr,
    u16             code_length,
    u16             expected_crc
);

/**
 * Called any time the micro reset is being asserted.
 *
 * This function specifically avoids bugzilla #5731.
 *
 * @param   port_no     Any port within the chip.
 */
vtss_rc vtss_phy_micro_assert_reset
(
    const vtss_port_no_t    port_no
);


#if TRANSIT_FAN_CONTROL || TRANSIT_THERMAL
ushort vtss_phy_read_temp(vtss_port_no_t port_no);
#endif // TRANSIT_FAN_CONTROL || TRANSIT_THERMAL

void   phy_read_id
(
    vtss_port_no_t  port_no,
    phy_id_t        *phy_id_p
);

#define SERDES_MAC_RX     (0x1 << 6)
#define SERDES_MAC_TX     (0x1 << 5)
#define SERDES_MAC_RX_TX     (SERDES_MAC_RX | SERDES_MAC_TX)
void phy_serdes_polarity_invert 
(
    vtss_port_no_t port_no,
    ushort         mac_direction

);

#if VTSS_ELISE || VTSS_TESLA
/**
 * Soft resetting a single PHY port.
 *
 * @param   port_no     The PHY port number to be soft resetted.
 */
vtss_rc vtss_phy_soft_reset_port
(
    vtss_port_no_t  port_no
);
#endif /* VTSS_ELISE || VTSS_TESLA */

#if VTSS_QUATTRO || VTSS_SPYDER || VTSS_COBRA
void vtss_phy_enab_smrt_premphasis
(
    vtss_port_no_t  port_no
);
#endif /* VTSS_QUATTRO || VTSS_SPYDER */

#endif /* __PHY_BASE_H__ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
