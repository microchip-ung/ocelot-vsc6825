//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#ifndef __HWPORT_H__
#define __HWPORT_H__

#include "common.h"     /* Always include common.h at the first place of user-defined herder files */

/**
 * Convert Chip/Switch port (zero-based) --> User/Front port (one-based).
 * port number.
 * We start to use the new API cport2uport() from Ferret project
 * and keep the original API port2ext() for backward compatibly
 */
vtss_uport_no_t cport2uport(vtss_cport_no_t chip_port);
#define port2ext(chip_port)     (cport2uport(chip_port))

/**
 * User/Front port (one-based) --> Chip/Switch port (zero-based)
 * We start to use the new API uport2cport() from Ferret project
 * and keep the original API port2int() for backward compatibly
 */
vtss_cport_no_t uport2cport(vtss_uport_no_t uport);
#define port2int(uport)         (uport2cport(uport))

/**
 * Convert internal (zero-based) port number to user/front port number (one-based).
 */
#define iport2uport(iport_no)   ((iport_no) + 1)

/**
 * Convert user/front (one-based) port number to internal port number (zero-based).
 */
#define uport2iport(uport_no)   ((uport_no) - 1)

/**
 * Convert internal (zero-based) port number to chip/switch port number (one-based).
 */
#define iport2cport(iport_no)   (uport2cport(iport2uport(iport_no)))

/**
 * Convert chip/switch port number (one-based) to internal (zero-based) port number.
 */
#define cport2iport(cport_no)   (uport2iport(cport2uport(cport_no)))

/**
 * Read hardware version from hardware.
 *
 * @TODO    This is a dummy. Customers should customize this if needed.
 */
char get_hw_version(void);

/**
 * Read configured MAC address and adjust it according to specified port.
 */
void  get_mac_addr (uchar port_no, uchar *mac_addr);

/**
 * Remove hardware reset of the PHY chips.
 */
void phy_hw_init (void);

/* GPIO/SGPIO initialization and SFP_TXDISABLE setup */
void gpio_init(void);

#endif /* __HWPORT_H__ */

/*****************************************************************************
 *                                                                           *
 *  End of file.                                                             *
 *                                                                           *
 *****************************************************************************/
