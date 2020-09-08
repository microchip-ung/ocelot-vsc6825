//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#ifndef _EEE_BASE_API_H_
#define _EEE_BASE_API_H_


 
typedef enum {
    tx_no_eee = 0,
    tx_initial,
    tx_running,
    tx_remote_change,
    tx_local_change,
    tx_update,
    tx_system_reallocation,
    tx_mirror_update,
} eee_sm_tx_type_t;

typedef enum {
    rx_no_eee = 0,
    rx_initial,
    rx_running,
    rx_change,
    rx_system_reallocation,
    rx_update,
    rx_mirror_update,
} eee_sm_rx_type_t; 


typedef struct {
    // Figure 78-4
    ushort LOCAL_INITIAL_TX_VALUE;
    ushort LocTxSystemValue;
    ushort RemTxSystemValueEcho;
    ushort RemRxSystemValue;
    ushort LocRxSystemValueEcho;
    ushort LocResolvedTxSystemValue;
    ushort TempRxVar; // IEEE 802.3az section 78.4.2.3 and figure 78-4
    ushort New_TX_VALUE;
    
    // 78-5
    ushort LOCAL_INITIAL_RX_VALUE;
    ushort LocRxSystemValue;
    ushort RemRxSystemValueEcho;
    ushort RemTxSystemValue;
    ushort LocTxSystemValueEcho;
    ushort LocResolvedRxSystemValue;
    ushort TempTxVar; // IEEE 802.3az section 78.4.2.3 and figure 78-5        
    ushort New_RX_VALUE;
} eee_variables_t;
 
typedef struct {
    uchar  link_up;
    uchar  phy_eee_cap;
    ushort speed;
    vtss_port_no_t  iport;   
    uchar  conf_changed;
    
    eee_sm_tx_type_t  tx_state_type;
    eee_sm_rx_type_t  rx_state_type;
    eee_variables_t   tm;
    BOOL eee_ena;
//    vtss_eee_port_conf_t *port_conf;
} eee_sm_t;



//************************************************
// Configuration definition 
//************************************************

// Switch configuration (configuration that are local for a switch in the stack)
// typedef struct {
//    vtss_eee_port_conf_t port[VTSS_PORTS]; 
//} eee_switch_conf_t;



//************************************************
// Constants
//************************************************

// Fast queues
#define EEE_FAST_QUEUES_MIN 1
#define EEE_FAST_QUEUES_MAX 8
#define EEE_FAST_QUEUES_CNT 8

// Wakeup time
#define EEE_WAKEUP_TIME_MAX 1000
#define EEE_WAKEUP_TIME_MIN 0

// TW Values. Set to the minimum values defines in IEEE 802.3az/D3.2 table 78-4.
#define PHY_WAKEUP_VALUE(speed) eee_default_lpi_timing_get(speed) 
#define VTSS_TX_TW_VALUE_1G 17
#define VTSS_TX_TW_VALUE_100M 30
#define VTSS_RX_TW_VALUE_1G 17
#define VTSS_RX_TW_VALUE_100M 30
#define LOCAL_INITIAL_TX_VALUE 30
#define LOCAL_INITIAL_RX_VALUE 30


void eee_set_port_enabled (uchar enabled, eee_sm_t *sm);

void sm_step(eee_sm_t xdata * sm);


ushort eee_transmit_time (uchar iport);
ushort eee_receive_time (uchar iport);
ushort eee_fallback_receive_time (uchar iport);
ushort eee_echo_transmit_time (uchar iport);
ushort eee_echo_receive_time (uchar iport);
// void  eee_mgmt_get_switch_conf(eee_switch_conf_t *switch_conf);
// void  eee_mgmt_set_switch_conf(eee_switch_conf_t *new_switch_conf);

#endif // _EEE_API_H_


//***************************************************************************
//  End of file.
//***************************************************************************
