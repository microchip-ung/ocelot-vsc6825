//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#include "common.h"     /* Always include common.h at the first place of user-defined herder files */

#include "taskdef.h"
#include "hwport.h"
#include "vtss_common_os.h"

#if TRANSIT_LLDP
#include "lldp.h"
#endif /* TRANSIT_LLDP */

#if FRONT_LED_PRESENT
#include "ledtsk.h"
#endif

#if TRANSIT_EEE
#include "eee_api.h"
#endif

#if TRANSIT_SNMP
#include "traps.h"
#endif

#if TRANSIT_LACP
#include "vtss_lacp_os.h"
#include "vtss_lacp.h"
#endif /* TRANSIT_LACP */

#if TRANSIT_E2ETC
#include "h2e2etc.h"
#endif

/*****************************************************************************
 *
 *
 * Defines
 *
 *
 *
 ****************************************************************************/

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

#if TRANSIT_EEE
static BOOL eee_port_link_up[NO_OF_BOARD_PORTS];
#endif /* TRANSIT_EEE */

/****************************************************************************
 *
 * Local Functions
 *
 ****************************************************************************/


/****************************************************************************
 *
 * Public Functions
 *
 ****************************************************************************/


/**
 * Notify applications when link goes up on a port.
 *
 * @note Called by phytsk.c
 */
void callback_link_up (vtss_cport_no_t chip_port)
{
    chip_port = chip_port;  /* make compiler happy */

#if FRONT_LED_PRESENT
    led_refresh();
#endif

#if TRANSIT_LLDP
    TASK(SUB_TASK_ID_LLDP_LINK,
         lldp_set_port_enabled(cport2uport(chip_port), LLDP_TRUE));
#endif /* TRANSIT_LLDP */

#if TRANSIT_LACP
    TASK(SUB_TASK_ID_LACP_LINK, vtss_lacp_linkstate_changed(OSINT2LACP(chip_port), VTSS_COMMON_LINKSTATE_UP));
#endif /* TRANSIT_LACP */

#if TRANSIT_EEE
    eee_port_link_up[cport2iport(chip_port)] = TRUE;
#endif /* TRANSIT_EEE */

#if TRANSIT_E2ETC
    e2etc_latency_set(chip_port);
#endif /* TRANSIT_E2ETC */
}

/**
 * Notify applications when link goes down on a port.
 *
 * @note Called by phytsk.c.
 */
void callback_link_down (vtss_cport_no_t chip_port)
{
    chip_port = chip_port;  /* make compiler happy */

#if FRONT_LED_PRESENT
    led_refresh();
#endif

#if TRANSIT_LLDP
    TASK(SUB_TASK_ID_LLDP_LINK,
         lldp_set_port_enabled(cport2uport(chip_port), LLDP_FALSE));
#endif /* TRANSIT_LLDP */

#if TRANSIT_LACP
    TASK(SUB_TASK_ID_LACP_LINK, vtss_lacp_linkstate_changed(OSINT2LACP(chip_port), VTSS_COMMON_LINKSTATE_DOWN));
#endif /* TRANSIT_LACP */

#if TRANSIT_EEE
    eee_port_link_change(cport2iport(chip_port), FALSE);
#endif /* TRANSIT_EEE */
}


#if TRANSIT_EEE
/**
 * Going into LPI will stress the DSP startup as it is not entirely
 * complete when link status becomes good and continues to train afterward.
 *
 * This requirement is noted in clause 22.6a.1 (for 100BASE-TX), 35.3a.1
 * (for 1000BASE-T), and 78.1.2.1.2 (for EEE as a whole).
 *
 * For EEE and it is highly desirable to have the 1 second delay from link
 * coming up prior to sending LPI.
 */
void callback_delayed_eee_lpi(void)
{
    vtss_iport_no_t iport_idx;

    for (iport_idx = 0; iport_idx < NO_OF_BOARD_PORTS; iport_idx++) {
        if (eee_port_link_up[iport_idx]) {
            eee_port_link_up[iport_idx] = FALSE;
            eee_port_link_change(iport_idx, TRUE);
        }
    }
}
#endif /* TRANSIT_EEE */



/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
