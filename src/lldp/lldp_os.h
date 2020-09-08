//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#ifndef LLDP_OS_H
#define LLDP_OS_H

#include "common.h"     /* Always include common.h at the first place of user-defined herder files */

#define LLDP_PORTS (NO_OF_BOARD_PORTS)
#define LLDP_FALSE 0
#define LLDP_TRUE  1

typedef uchar lldp_bool_t;
typedef ushort lldp_counter_t;
typedef ushort lldp_timer_t;

typedef uchar lldp_u8_t;
typedef ushort lldp_u16_t;
typedef ulong lldp_u32_t;

/* port number,  counting from 1..LLDP_PORTS */
typedef uchar lldp_port_t;

typedef enum {
    LLDP_DISABLED,
    LLDP_ENABLED_RX_TX,
    LLDP_ENABLED_TX_ONLY,
    LLDP_ENABLED_RX_ONLY,
} lldp_admin_state_t;

typedef struct {
    /* tlv_optionals_enabled uses bits 0-5 of the word:
    ** Port Description:     bit 0
    ** System Name:          bit 1
    ** System Description:   bit 2
    ** System Capabilities:  bit 3
    ** Management Address:   bit 4
    ** 802.3at Capabilities: bit 5 - bit 7
    ** 802.3az Capabilities: bit 8
    */
    ushort           tlv_optionals_enabled;

    ushort           reInitDelay;
    ushort           msgTxHold;
    ushort           msgTxInterval;
    ushort           txDelay;

    /* interpretation of admin_state is as follows:
    ** (must match lldp_admin_state_t)
    ** 0 = disabled
    ** 1 = rx_tx
    ** 2 = tx
    ** 3 = rx only
    */
    uchar            admin_state[NO_OF_BOARD_PORTS];
} lldp_struc_0_t;

typedef lldp_struc_0_t lldp_conf_t;

void  load_lldp_conf (void);
void  save_lldp_conf (void);

lldp_timer_t lldp_os_get_msg_tx_interval (void);
lldp_timer_t lldp_os_get_msg_tx_hold (void);
lldp_timer_t lldp_os_get_reinit_delay (void);
lldp_timer_t lldp_os_get_tx_delay (void);

void lldp_os_set_msg_tx_interval (lldp_timer_t val);
void lldp_os_set_msg_tx_hold (lldp_timer_t val);
void lldp_os_set_reinit_delay (lldp_timer_t val);
void lldp_os_set_tx_delay (lldp_timer_t val);

lldp_u8_t xdata * lldp_os_get_frame_storage (void);
lldp_admin_state_t lldp_os_get_admin_status (lldp_port_t port);
void lldp_os_set_admin_status (lldp_port_t port, lldp_admin_state_t admin_state);
void lldp_os_tx_frame (lldp_port_t port_no, lldp_u8_t xdata * frame, lldp_u16_t len);
void lldp_os_get_if_descr (lldp_port_t port, lldp_u8_t xdata * dest);
void lldp_os_get_system_name (lldp_u8_t xdata * dest);
void lldp_os_get_system_descr (lldp_u8_t xdata * dest);
void lldp_os_get_ip_address (lldp_u8_t xdata * dest);
lldp_u8_t lldp_os_get_ip_enabled (void);
void lldp_os_set_optional_tlv (lldp_u8_t tlv, lldp_u8_t enabled);
lldp_u8_t lldp_os_get_optional_tlv_enabled (lldp_u8_t tlv);
void lldp_os_print_remotemib (void);
void lldp_os_set_tx_in_progress (lldp_bool_t tx_in_progress);

lldp_u32_t lldp_os_get_sys_up_time (void);

#ifndef LOW_BYTE
#define LOW_BYTE(v) ((lldp_u8_t) (v))
#endif
#ifndef HIGH_BYTE
#define HIGH_BYTE(v) ((lldp_u8_t) (((lldp_u16_t) (v)) >> 8))
#endif

#endif

