//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#include "common.h"     /* Always include common.h at the first place of user-defined herder files */
#include "vtss_common_os.h"
#include "h2eee.h"
#include "vtss_api_base_regs.h"
#include "h2io.h"

#include "eee_base_custom.h"

#if TRANSIT_EEE
#include "eee_api.h"
#include "eee_base_api.h"
#include "lldp_remote.h"
#include "lldp_os.h"
#include "lldp_tlv.h"
#include "spiflash.h"
#include "print.h"
#include "timer.h"
#include "phydrv.h"
#include "phymap.h"

/*****************************************************************************
 *
 *
 * Defines
 *
 *
 *
 ****************************************************************************/

#if TRANSIT_EEE_LLDP
#define L2_EEE_ENABLE_MODE  1  /* Default Enabled */
#else
#define L2_EEE_ENABLE_MODE  0  /* Default Disabled */
#endif

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
 * Public data
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
* ************************************************************************ */
const ushort PHY_WAKEUP_VALUE_1G  = 16;
const ushort PHY_WAKEUP_VALUE_100 = 20;

static eee_conf_t eee_conf;

void eee_mgmt_int (void)
{
#if TRANSIT_EEE_LLDP    
    uchar               port_ext_0 = 0;
    vtss_iport_no_t     iport = 0;
    vtss_cport_no_t     chip_port;
#endif    

    load_eee_conf();

#if TRANSIT_EEE_LLDP
    for(port_ext_0 = 1; port_ext_0 <= NO_OF_BOARD_PORTS; port_ext_0++) {
        iport = uport2iport(port_ext_0);
        chip_port = uport2cport(port_ext_0);
        
        if(phy_map(chip_port))
            eee_sm[iport].eee_ena = read_eee_conf_mode(iport);
        else
            eee_sm[iport].eee_ena = 0; // Disable always
        eee_sm[iport].iport  = iport;
    }
#endif
}


void eee_mgmt(void)
{
#if TRANSIT_EEE_LLDP
    uchar               port_ext_0;
    vtss_iport_no_t     iport = 0;
    vtss_cport_no_t     chip_port;
    
    for(port_ext_0 = 1; port_ext_0 <= NO_OF_BOARD_PORTS; port_ext_0++) {
        iport = uport2iport(port_ext_0);
        chip_port = uport2cport(port_ext_0);
        
        if(phy_map(chip_port))
            sm_step(&eee_sm[iport]);
    }
#endif
}


// Going into LPI will stress the DSP startup as it is not entirely complete when link status becomes good and continues to train afterward.
// This requirement is noted in clause 22.6a.1 (for 100BASE-TX), 35.3a.1 (for 1000BASE-T), and 78.1.2.1.2 (for EEE as a whole).
// For EEE and it is highly desirable to have the 1 second delay  (Done in main.c) from link coming up prior to sending LPI.
// This can be done using PHY registers to block sending LPI when the link is down and only unblock it 1 second after
// the link comes up and then reblock it when the link goes down.
// This can be blocked using extended-page 2, register 17 by setting bits 1 and 3. Then clear bits 1 and 3 a second after the link comes up.
void eee_port_link_change(uchar iport, BOOL up)
{
    //uchar port_ext_1 = cport2uport(iport);
    //uchar port_ext_0 = port_ext_1 - 1;
    
    vtss_cport_no_t     chip_port = iport2cport(iport);
    vtss_uport_no_t     uport  = iport2uport(iport);   
    
    
    BOOL enable = read_eee_conf_mode(iport); // Get if EEE is enabled for this port-

    if (enable) {
#if TRANSIT_EEE_LLDP        
        eee_sm[iport].phy_eee_cap = vtss_os_get_link_pwr_mngr_mode(uport);
        eee_sm[iport].link_up = up;
        //print_str("eee_port_link_change:  ");
        eee_sm[iport].speed = vtss_os_get_linkspeed(uport);
        //print_dec(eee_sm[iport].speed); print_spaces(2); print_dec(iport); print_cr_lf(); 
        eee_set_port_enabled(1, &eee_sm[iport]); // If link changes, update EEE state machine
#endif

        phy_write(chip_port, 31 ,2);
#if TRANSIT_EEE_LLDP        
        if (up && (eee_sm[iport].phy_eee_cap ? 1:0)) {
#else
        if (up && vtss_os_get_link_pwr_mngr_mode(uport)) {
#endif            
            phy_write_masked(chip_port, 17, 0x0, 0x000A);

            // Below is described a problem with getting LPI through the PCS. The work-around is to
            // disable and then re-enable EEE after link up.

            // The PCS1g loses synchronization at port link down/up. After some time the ANEG state machine detects this
            // and issues a change of the XMIT value. This XMIT change is evaluated in the TX-control state machine
            // (similar to a power-on or reset) and state TX_TEST_XMIT is evaluated. There are three possible exits,
            // but no one deals with LPI. Instead the IDLE state is entered and in this state it is waited until
            // tx_en = '0' and tx_er = '0' (whereas the LPI encoding is tx_en='0', tx_er='1').
            // First when a data packet is transferred and normal IDLE afterwards tx_en and tx_er become '0' at the same
            // time and the Low power mode can be established again.
#if defined(VTSS_ARCH_OCELOT)
            H2_WRITE_MASKED(VTSS_QSYS_SYSTEM_EEE_CFG(chip_port), 0x0, 0x20000000);
            H2_WRITE_MASKED(VTSS_QSYS_SYSTEM_EEE_CFG(chip_port), 0x20000000, 0x20000000);
#elif defined(VTSS_ARCH_LUTON26)
            H2_WRITE_MASKED(VTSS_SYS_SYSTEM_EEE_CFG(chip_port), 0x0, 0x20000000);
            H2_WRITE_MASKED(VTSS_SYS_SYSTEM_EEE_CFG(chip_port), 0x20000000, 0x20000000);
#endif // VTSS_ARCH_OCELOT
        } else {
            phy_write_masked(chip_port, 17, 0xA, 0x000A);
        }
        phy_write(chip_port, 31 ,0);
    }
}

// Function for setup EEE for a port
// In : iport -  internal port number
void eee_port_mode_setup(uchar iport)
{
    //uchar port_ext   = cport2uport(iport);
    //uchar port_ext_0 = port_ext - 1;
    
    vtss_cport_no_t     chip_port = iport2cport(iport);
    vtss_uport_no_t     uport  = iport2uport(iport);  
    
    BOOL enable = read_eee_conf_mode(iport); // Get if EEE is enabled for this port-
    eee_port_conf_t eee_port_conf;

    if (phy_map(chip_port)) {
        return;
    }

#if TRANSIT_EEE_LLDP
    if(!lldp_os_get_admin_status(uport) || !lldp_os_get_optional_tlv_enabled(LLDP_TLV_ORG_EEE_TLV)) {
        eee_sm[iport].eee_ena = enable;
    }
#endif


    eee_port_conf.eee_ena = enable;

    // Get the link partner auto negotiation advertisement
    (void) eee_link_partner_advertisements_get(iport, &eee_port_conf.lp_advertisement);

    h2_eee_port_conf_set(iport, &eee_port_conf);
}

void load_eee_conf (void)
{
    uchar               port_ext_0;
    vtss_iport_no_t     iport;

    static uchar eee_conf_init = 0;
    if(!eee_conf_init) {
        for(port_ext_0 = 1; port_ext_0 <= NO_OF_BOARD_PORTS; port_ext_0++) {
            iport = uport2iport(port_ext_0);
#if defined(LUTON26_L16)
            if(port_ext_0 < 12)
                write_eee_conf_mode   (iport, L2_EEE_ENABLE_MODE);
            else
                write_eee_conf_mode   (iport, 0); // force disable
#else
             write_eee_conf_mode   (iport, L2_EEE_ENABLE_MODE);
#endif
            write_eee_conf_tx_tw  (iport, PHY_WAKEUP_VALUE_1G);
            write_eee_conf_rx_tw  (iport, PHY_WAKEUP_VALUE_1G);
            write_conf_fast_queues(iport, 0x0);
        }
        eee_conf_init = 1;
    }
}

uchar read_eee_conf_mode (vtss_iport_no_t iport)
{
    return eee_conf.port[iport].eee_ena;
}

void write_eee_conf_mode (vtss_iport_no_t iport, uchar mode)
{
    //uchar iport = uport2cport(port_ext_0 + 1);
    vtss_cport_no_t     chip_port  = iport2cport(iport);
    //print_str("iport: "); print_dec(iport);
    
    eee_conf.port[iport].eee_ena = mode;

    vtss_phy_eee_ena_private(chip_port, eee_conf.port[iport].eee_ena); // Updated the chip register settings

#if TRANSIT_EEE_LLDP
    eee_sm[iport].conf_changed = 1;
#endif
}

void write_eee_conf_tx_tw (vtss_iport_no_t iport, ushort time)
{
    eee_conf.port[iport].tx_tw = time;
#if TRANSIT_EEE_LLDP
    eee_sm[iport].conf_changed = 1;
#endif
}

void write_eee_conf_rx_tw (vtss_iport_no_t iport, ushort time)
{
    eee_conf.port[iport].rx_tw = time;
#if TRANSIT_EEE_LLDP
    eee_sm[iport].conf_changed = 1;
#endif
}

void write_conf_fast_queues (vtss_iport_no_t iport, uchar fast_queues)
{
    eee_conf.port[iport].eee_fast_queues = fast_queues;
#if TRANSIT_EEE_LLDP
    eee_sm[iport].conf_changed = 1;
#endif
}

#endif

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/