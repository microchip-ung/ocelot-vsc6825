//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



// In order to share this EEE base between multiple projects and platforms
// some platform define must be defined below.

#include "eee_base_custom.h"

#if EEE_BASE_ENABLE
#include "vtss_common_os.h"
#include "lldp_remote.h"
#include "eee_base_api.h"

// State machine
eee_sm_t                 eee_sm[EEE_NO_OF_PORTS];
BOOL                     tx_tw_changed[EEE_NO_OF_PORTS];

#if 0 // Uncalled function
// For debugging purpose we like to be able to set the tx_tw value. This is purpose with this function.
// If we shouldn't use this debug tx_tw value, then tx_tw is indicated by setting tx_tw to 0.
//
// In : iport - The port in question.
//      set - Set to TRUE when setting the tx_tw value else FALSE
//      new_debug_tx_tw - Updates the tx_tw value. Only valid when "set" is TRUE.
//
// Return: The current value of tx_tw for the corresponding port.
ushort eee_debug_tx_tw(eee_vtss_port_no_t iport, BOOL set, uint new_debug_tx_tw)
{
    static BOOL init_done = FALSE;
    static uint debug_tx_tw[EEE_NO_OF_PORTS]; // Array with tx_tw value for each port
    eee_vtss_port_no_t port;

    // Initialize the debug_tx_tw array the first time this function is reached
    if (init_done == FALSE) {
        init_done = TRUE;
        for (port = 0; port < EEE_NO_OF_PORTS; port++) {
            debug_tx_tw[port] = 0;
        }
    }


    // Update the tx_tw value
    if (set) {
        debug_tx_tw[iport] = new_debug_tx_tw;
        tx_tw_changed[iport] = TRUE; // Signaling to state machine that we have changed "something"
    }
#if 0
    T_I_PORT(iport, "debug_tx_tw = %d, set = %d, new_debug_tx_tw =%d",
             debug_tx_tw[iport], set, new_debug_tx_tw);
#endif

    return debug_tx_tw[iport];
}
#endif



// Function for getting tx_tw time for this switch.
ushort eee_transmit_time(eee_vtss_port_no_t iport)
{
#if defined(VTSS_ARCH_OCELOT)
    eee_sm_t *sm = &eee_sm[iport]; 	
    
    return (sm->speed == EEE_SPEED_1G) ? VTSS_TX_TW_VALUE_1G : VTSS_TX_TW_VALUE_100M;  		  
#else
	uint debug_tx_rw = eee_debug_tx_tw(iport, FALSE, 0);
    eee_sm_t *sm = &eee_sm[iport];

    T_I_PORT(iport, "Speed = %d", sm->speed);

    if (debug_tx_rw == 0) {
        return (sm->speed == EEE_SPEED_1G) ? VTSS_TX_TW_VALUE_1G : VTSS_TX_TW_VALUE_100M;
    } else {
        return debug_tx_rw;
    }
#endif    
}

// Function for getting rx_tw time for this switch.
ushort eee_receive_time (eee_vtss_port_no_t iport)
{
    eee_sm_t *sm = &eee_sm[iport];

    //print_str("eee_receive_iport: "); print_dec(iport); print_cr_lf();
    //Remote Request Recevie Timer
#if 0
    T_D_PORT(iport, "Getting LocRxSystemValue");
#endif
    return (sm->speed == EEE_SPEED_1G) ? VTSS_RX_TW_VALUE_1G : VTSS_RX_TW_VALUE_100M;
}

// Function for getting fallbacktime time for this switch.
ushort eee_fallback_receive_time (eee_vtss_port_no_t iport)
{
    //Fallback Tw. Not supported - Set to Receive Tw, IEEE 802.3az section 79.3.0.2
    return eee_receive_time(iport);
}

// Function for getting LocTxSystemValueEcho
ushort eee_echo_transmit_time (eee_vtss_port_no_t iport)
{
    eee_sm_t *sm = &eee_sm[iport];
    
    //print_str("eee_echo_transmit_time: "); print_dec(iport); print_cr_lf();

    if (sm->eee_ena) {
#if 0
        T_D_PORT(iport, "Getting transmit time");
#endif
        return sm->tm.LocTxSystemValueEcho;
    }
    return 0;
}

// Function for getting LocRxSystemValueEcho
ushort eee_echo_receive_time (eee_vtss_port_no_t iport)
{
    eee_sm_t *sm = &eee_sm[iport];

#if 0
    T_D_PORT(iport, "Getting echo_recv_time");
#endif
    return sm->tm.LocRxSystemValueEcho;
}



// Function for enabling EEE (starting the state machine) for a specific port
// In: sm - Current state machine
void eee_set_port_enabled (uchar enabled, eee_sm_t *sm)
{
    if (sm->link_up && enabled) {
#if 0
        T_D_PORT(sm->iport, "link up");
#endif
        // Start state machine
        sm->tx_state_type = tx_initial;
        sm->rx_state_type = rx_initial;
    } else {
#if 0
        T_D_PORT(sm->iport, "link down");
#endif
        // Keep state machine in "idle"
        sm->tx_state_type = tx_no_eee;
        sm->tx_state_type = tx_no_eee;
    }
}

// Function for finding the required tx_tw
ushort examine_tx_change(eee_sm_t *sm)
{
    // Find the tx_tw for the local switch
    ushort loc_tx_tw_max = eee_transmit_time(sm->iport);
    // The remote rx_tw
    ushort rem_rx_tw_max = sm->tm.RemRxSystemValue;


    // Return the maximum tw (local or remote)
    return MAX(loc_tx_tw_max, rem_rx_tw_max);
}



// Function for finding the required rx_tw
ushort examine_rx_change(eee_sm_t *sm)
{
    // Find the tx_tw for the local switch
    ushort loc_rx_tw_max = (sm->speed == EEE_SPEED_1G) ? VTSS_RX_TW_VALUE_1G : VTSS_RX_TW_VALUE_100M;

    // The remote rx_tw
    ushort rem_tx_tw_max = sm->tm.RemTxSystemValue;


    // Return the largest tw (local or remote)
    return MAX(loc_rx_tw_max, rem_tx_tw_max);
}

// Function returning the default LPI timing setting depending upon link speed.
//
ushort eee_default_lpi_timing_get(ushort speed)
{
    return speed == EEE_SPEED_1G ? VTSS_RX_TW_VALUE_1G : VTSS_RX_TW_VALUE_100M;
}


// IEEE 802.3az/D3.2 Figure 78-6 - EEE DLL Receiver State Diagram
static void eee_rx_state_diagram(eee_sm_t xdata *sm)
{

    uchar max;
    int i;
    lldp_remote_entry_t *entry;
    BOOL local_system_change = FALSE;


    // RX state diagram - Figure 78-5 in IEEE 802.3az
    switch (sm->rx_state_type) {
    case rx_initial:
#if 0
        T_D_PORT(sm->iport, "rx_initial, speed");
#endif
        sm->tm.LocRxSystemValue         = PHY_WAKEUP_VALUE(sm->speed);
        sm->tm.LocRxSystemValueEcho     = PHY_WAKEUP_VALUE(sm->speed);
        sm->tm.RemTxSystemValue         = PHY_WAKEUP_VALUE(sm->speed);
        sm->tm.RemTxSystemValueEcho     = PHY_WAKEUP_VALUE(sm->speed);
        sm->tm.LocResolvedRxSystemValue = PHY_WAKEUP_VALUE(sm->speed);
        sm->tm.TempTxVar                = PHY_WAKEUP_VALUE(sm->speed);
        local_system_change = TRUE;

    case rx_running:
        sm->tm.RemTxSystemValue = PHY_WAKEUP_VALUE(sm->speed);
        max = lldp_remote_get_max_entries();
        for (i = 0; i < max; i++) {
            entry = lldp_get_remote_entry(i);
            if ((entry->in_use) && (entry->receive_port == eee_iport2uport(sm->iport)) && entry->is_eee) {
                sm->tm.RemTxSystemValue     = entry->xmit_time;
                sm->tm.RemRxSystemValueEcho = entry->echo_recv_time;
#if 0
                T_D_PORT(sm->iport, "Setting  sm.RemTxSystemValue");
#endif
                break;
            }
        }

        if (sm->tm.RemTxSystemValue != sm->tm.TempTxVar || local_system_change) {
            sm->rx_state_type = rx_change;
        } else {
            sm->rx_state_type = rx_running;
        }
#if 0
        T_D_PORT(sm->iport, "rx_running: RemTxSystemValue");
#endif
        break;

    case rx_change:
        sm->tm.New_RX_VALUE = examine_rx_change(sm);
        sm->tm.TempTxVar = sm->tm.RemTxSystemValue;
        if (sm->tm.New_RX_VALUE <= sm->tm.LocResolvedRxSystemValue || sm->tm.New_RX_VALUE <= sm->tm.TempTxVar || !sm->tm.LocResolvedRxSystemValue) {
            sm->rx_state_type = rx_system_reallocation;
        } else {
            sm->rx_state_type = rx_update;
        }
#if 0
        T_D_PORT(sm->iport, "rx_change");
#endif
        break;

    case rx_system_reallocation:
        /* Rx System Reallocation */
        /* Just update value */
        sm->tm.LocResolvedRxSystemValue = sm->tm.New_RX_VALUE;

#if 0
        T_D_PORT(sm->iport, "rx_system_reallocation");
#endif
#if 0
        T_I_PORT(sm->iport, "rx_system_reallocation: sm->tm.LocResolvedRxSystemValue = %d, sm->tm.New_RX_VALUE = %d",
                 sm->tm.LocResolvedRxSystemValue, sm->tm.New_RX_VALUE);
#endif
        /* UCT to Rx Update */

    case rx_update:
        sm->tm.LocRxSystemValue = sm->tm.New_RX_VALUE;
        /* UCT to Mirror update */

    case rx_mirror_update:
        sm->tm.LocTxSystemValueEcho = sm->tm.TempTxVar;
        sm->rx_state_type = rx_running;
        break;

    default:
        sm->rx_state_type = rx_initial;
        break;
    }

}


// IEEE 802.3az/D3.2 Figure 78-5 - EEE DLL Transmitter State Diagram
static void eee_tx_state_diagram(eee_sm_t xdata *sm)
{
    uchar  i, max;
    lldp_remote_entry_t *entry;
    vtss_eee_port_conf_t eee_port_conf;

    eee_port_conf.eee_ena = sm->eee_ena;
    switch (sm->tx_state_type) {

    case tx_initial:
        /* Assign default value */
#if 0
        T_D_PORT(sm->iport, "tx_initial speed");
#endif
        sm->tm.LocTxSystemValue         = PHY_WAKEUP_VALUE(sm->speed);
        sm->tm.LocTxSystemValueEcho     = PHY_WAKEUP_VALUE(sm->speed);
        sm->tm.RemRxSystemValue         = PHY_WAKEUP_VALUE(sm->speed);
        sm->tm.RemRxSystemValueEcho     = PHY_WAKEUP_VALUE(sm->speed);
        sm->tm.LocResolvedTxSystemValue = PHY_WAKEUP_VALUE(sm->speed);
        sm->tm.TempRxVar                = PHY_WAKEUP_VALUE(sm->speed);


    case tx_running:
        /*
        ** During running state
        ** 1. Read LLDP TLV information
        **    - Update RemXXXX time parameters.
        ** 2. Check Local configuration if changed.
        **    - If changed, move to Local Change state.
        ** 3. Check if RemRxSystemValue and RemTxSystemValueEcho
        **    - If changed, mode to Remote Change
        ** ps: Add a temp variable new_tx_value. */

        /* Read LLDP TLV */
        max = lldp_remote_get_max_entries();
        sm->tm.RemRxSystemValue = PHY_WAKEUP_VALUE(sm->speed);
        for (i = 0; i < max; i++) {
            entry = lldp_get_remote_entry(i);
            if ((entry->in_use) && (entry->receive_port == eee_iport2uport(sm->iport)) && entry->is_eee) {
                sm->tm.RemRxSystemValue = entry->recv_time;
                sm->tm.RemTxSystemValueEcho = entry->echo_xmit_time;
#if 0
                T_D_PORT(sm->iport, "Setting  sm.RemRxSystemValue");
#endif
                break;
            }
        }


#if 0
        T_I_PORT(sm->iport, "tx_running, RemRxSystemValue = %d, TempRxVar =%d, LocTxSystemValue =%d, RemTxSystemValueEcho =%d, conf_changed =%d, tx_tw_changed = %d",
                 sm->tm.RemRxSystemValue, sm->tm.TempRxVar, sm->tm.LocTxSystemValue, sm->tm.RemTxSystemValueEcho, sm->conf_changed, tx_tw_changed[sm->iport]);
#endif


        /* Check configuration */
        if (sm->conf_changed || tx_tw_changed[sm->iport]) {
            tx_tw_changed[sm->iport] = FALSE;
            sm->tx_state_type = tx_local_change;
            sm->conf_changed = FALSE;
#if 0
            T_D_PORT(sm->iport, "sm->tx_state_type = tx_local_change");
#endif

            // !!! FJ-TBD !!! I can not understand the reason for "sm->tm.LocTxSystemValue == sm->tm.RemTxSystemValueEch" in state diagram so that are skipped for now
        } else  if ((sm->tm.RemRxSystemValue != sm->tm.TempRxVar) || (sm->tm.LocTxSystemValue != sm->tm.RemTxSystemValueEcho)) {
#if 0
            T_D_PORT(sm->iport, "sm->tx_state_type = tx_remote_change");
#endif
            sm->tx_state_type = tx_remote_change;
        }
        break;


    case tx_local_change:

        sm->tm.TempRxVar = sm->tm.RemRxSystemValue;
        sm->tm.New_TX_VALUE = examine_tx_change(sm);  // Find the maximum value of local tx_tw and remote rx_tw
        if (sm->tm.LocTxSystemValue == sm->tm.RemTxSystemValueEcho || sm->tm.New_TX_VALUE < sm->tm.LocTxSystemValue) {
            sm->tx_state_type = tx_update;
        } else {
            sm->tx_state_type = tx_mirror_update; // !!! FJ-TBD !!!  IEEE 802.3az D3.2 Says go to running state.
            // But if the change doesn't result in a remote change then
            // the LocRxSystemValueEcho gets reset, and never gets set again, so I goes to mirror state.
        }
#if 0
        T_I_PORT(sm->iport, "tx_local_change");
#endif
        break;


    case tx_remote_change:
        /* Remote Change */
        sm->tm.TempRxVar = sm->tm.RemRxSystemValue;
        sm->tm.New_TX_VALUE = examine_tx_change(sm); // Find the maximum value of local tx_tw and remote rx_tw
#if 0
        T_I_PORT(sm->iport, "tx_remote_change");
#endif
        /* UTC to Tx Update */

    case tx_update:
        sm->tm.LocTxSystemValue = sm->tm.New_TX_VALUE;
        /* Check transfer condition */
        /* if new value larger than both local system value and remote rx value, then update */
        if (((sm->tm.New_TX_VALUE >= sm->tm.LocResolvedTxSystemValue) || (sm->tm.New_TX_VALUE >= sm->tm.TempRxVar))
            || !sm->tm.LocResolvedTxSystemValue) {
            sm->tx_state_type = tx_system_reallocation;
        } else {
            sm->tx_state_type = tx_mirror_update;
        }
#if 0
        T_D_PORT(sm->iport, "tx_update");
#endif
        break;

    case tx_system_reallocation:
        sm->tm.LocResolvedTxSystemValue = sm->tm.New_TX_VALUE;

        // Update new wakeup timer into system
        eee_port_conf.tx_tw   = sm->tm.LocResolvedTxSystemValue;

        // Get the link partner auto negotiation advertisement
        (void) eee_link_partner_advertisements_get(sm->iport, &eee_port_conf.lp_advertisement);

        /* Assign value to chip */
        eee_api_conf_set(sm->iport, &eee_port_conf);

#if 0
        T_D_PORT(sm->iport, "tx_system_reallocation");
#endif
#if 0
        T_I_PORT(sm->iport, "sm->tm.LocResolvedTxSystemValue = %d, sm->tm.New_TX_VALUE = %d",
                 sm->tm.LocResolvedTxSystemValue, sm->tm.New_TX_VALUE);
#endif

        /* UCT to Mirror Update */

    case tx_mirror_update:
        /* Mirror update */
        sm->tm.LocRxSystemValueEcho = sm->tm.TempRxVar;
        sm->tx_state_type = tx_running;
#if 0
        T_D_PORT(sm->iport, "tx_mirror_update");
#endif
        break;

    default:
        break;
        sm->tx_state_type = tx_initial;
        break;
    }

}

// State Machine for TX and RX
void sm_step(eee_sm_t xdata *sm)
{
    vtss_eee_port_conf_t eee_port_conf;


    // Force state machine to "IDLE" if the link is down etc.
    if (!sm->eee_ena || sm->tx_state_type == tx_no_eee || sm->rx_state_type == rx_no_eee || !sm->link_up) {
        eee_port_conf.eee_ena = FALSE;

        sm->tm.LocTxSystemValue         = LOCAL_INITIAL_TX_VALUE;
        sm->tm.LocResolvedTxSystemValue = LOCAL_INITIAL_TX_VALUE;
        sm->tm.LocRxSystemValue         = LOCAL_INITIAL_RX_VALUE;
        sm->tm.LocResolvedRxSystemValue = LOCAL_INITIAL_RX_VALUE;


        // Reset to default. Needed to make sure that XX_tw doesn't get set to zero, which gives frames drops.
        eee_port_conf.tx_tw = LOCAL_INITIAL_TX_VALUE;

#if 0
        T_D_PORT(sm->iport, "sm->eee_ena =%d, sm->tx_state_type =%d, sm->rx_state_type %d, sm->link_up = %d",
                 sm->eee_ena, sm->tx_state_type, sm->rx_state_type, sm->link_up );
#endif


        // Get the link partner auto negotiation advertisement
        (void) eee_link_partner_advertisements_get(sm->iport, &eee_port_conf.lp_advertisement);

        // Update API
        eee_api_conf_set(sm->iport, &eee_port_conf);
        return;
    }

    // Call the state diagrams
    eee_tx_state_diagram(sm);
    eee_rx_state_diagram(sm);

    eee_sm[sm->iport] = *sm;
}

#endif

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
