//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#ifndef LLDP_REMOTE_H
#define LLDP_REMOTE_H

#include "lldp_os.h"

/* all lengths must be below 255 (octets)
** If not, change the type of the _length fields to lldp_u16_t
*/
#define MAX_CHASSIS_ID_LENGTH   10
#define MAX_PORT_ID_LENGTH      10
#define MAX_PORT_DESCR_LENGTH   10
#define MAX_SYSTEM_NAME_LENGTH  10
#define MAX_SYSTEM_DESCR_LENGTH 10
#define MAX_MGMT_OID_LENGTH     10

typedef struct {
    /* to begin with, we have a number of fields needed for
    ** SNMP MIB purposes or for other management related
    ** purposes */
    lldp_port_t    receive_port;
    lldp_u16_t     lldp_remote_index;
    lldp_u32_t     time_mark;

    lldp_u8_t      in_use;

    /* The following fields are "data fields" with received data */
    lldp_u8_t      chassis_id_subtype;
    lldp_u8_t      chassis_id_length;
    lldp_u8_t      chassis_id[MAX_CHASSIS_ID_LENGTH + 1];

    lldp_u8_t      port_id_subtype;
    lldp_u8_t      port_id_length;
    lldp_u8_t      port_id[MAX_PORT_ID_LENGTH + 1];

#if !TRANSIT_LLDP_REDUCED
    lldp_u8_t      port_description_length;
    lldp_u8_t      port_description[MAX_PORT_DESCR_LENGTH + 1];

    lldp_u8_t      system_name_length;
    lldp_u8_t      system_name[MAX_SYSTEM_NAME_LENGTH + 1];

    lldp_u8_t      system_description_length;
    lldp_u8_t      system_description[MAX_SYSTEM_DESCR_LENGTH + 1];

    lldp_u8_t      system_capabilities[4];

    lldp_u8_t      mgmt_address_subtype;
    lldp_u8_t      mgmt_address_length;
    lldp_u8_t      mgmt_address[31];
    lldp_u8_t      mgmt_address_if_number_subtype;
    lldp_u8_t      mgmt_address_if_number[4];
    lldp_u8_t      oid_length;
    lldp_u8_t      oid[MAX_MGMT_OID_LENGTH + 1]; /* we don't support OID's up to 128 bytes */
#endif

    lldp_u16_t     rx_info_ttl;
#if TRANSIT_EEE_LLDP
    lldp_u8_t     is_eee;
    lldp_u8_t     mgmt_ieee_subtype;
    lldp_u16_t    xmit_time;
    lldp_u16_t    recv_time;
    lldp_u16_t    fallback_recv_time;
    lldp_u16_t    echo_xmit_time;
    lldp_u16_t    echo_recv_time;
#endif
    lldp_u8_t      something_changed_remote;
} lldp_remote_entry_t;

/* like a remote entry, but with pointers into the received frame instead
** of arrays */
typedef struct {
    lldp_port_t    receive_port;

    /* The following fields are "data fields" with received data */
    lldp_u8_t      chassis_id_subtype;
    lldp_u8_t      chassis_id_length;
    lldp_u8_t xdata * chassis_id;

    lldp_u8_t      port_id_subtype;
    lldp_u8_t      port_id_length;
    lldp_u8_t xdata * port_id;

#if !TRANSIT_LLDP_REDUCED
    lldp_u8_t      port_description_length;
    lldp_u8_t xdata * port_description;

    lldp_u8_t      system_name_length;
    lldp_u8_t xdata * system_name;

    lldp_u8_t      system_description_length;
    lldp_u8_t xdata * system_description;

    lldp_u8_t     system_capabilities[4];

    lldp_u8_t      mgmt_address_subtype;
    lldp_u8_t      mgmt_address_length;
    lldp_u8_t xdata * mgmt_address;
    lldp_u8_t      mgmt_address_if_number_subtype;
    lldp_u8_t xdata * mgmt_address_if_number;
    lldp_u8_t      oid_length;
    lldp_u8_t xdata * oid; /* we don't support OID's up to 128 bytes */
#endif
    lldp_u16_t     ttl;

#if TRANSIT_EEE_LLDP
    lldp_u8_t     mgmt_ieee_subtype;
    lldp_u16_t    xmit_time;
    lldp_u16_t    recv_time;
    lldp_u16_t    fallback_recv_time;
    lldp_u16_t    echo_xmit_time;
    lldp_u16_t    echo_recv_time;
#endif
} lldp_rx_remote_entry_t;

typedef struct {
    lldp_u32_t table_change_time;
    lldp_counter_t table_inserts;
    lldp_counter_t table_deletes;
    lldp_counter_t table_drops;
    lldp_counter_t table_ageouts;
} lldp_mib_stats_t;

void lldp_remote_delete_entries_for_local_port (lldp_port_t port);
lldp_bool_t lldp_remote_handle_msap (lldp_rx_remote_entry_t xdata * rx_entry);
lldp_u8_t lldp_remote_get_max_entries (void);
lldp_remote_entry_t xdata * lldp_get_remote_entry (lldp_u8_t idx);
void lldp_remote_1sec_timer (void);
void lldp_remote_tlv_to_string (lldp_remote_entry_t xdata * entry, lldp_u8_t field, lldp_u8_t xdata * dest);
void lldp_chassis_type_to_string (lldp_remote_entry_t xdata * entry, lldp_u8_t xdata * dest);
void lldp_port_type_to_string (lldp_remote_entry_t xdata * entry, lldp_u8_t xdata * dest);
lldp_mib_stats_t xdata * lldp_get_mib_stats (void);
lldp_remote_entry_t xdata * lldp_remote_get_next(lldp_u32_t time_mark, lldp_port_t port, lldp_u16_t remote_idx);
lldp_remote_entry_t xdata * lldp_remote_get_next_non_zero_addr(lldp_u32_t time_mark, lldp_port_t port, lldp_u16_t remote_idx);
lldp_remote_entry_t xdata * lldp_remote_get(lldp_u32_t time_mark, lldp_port_t port, lldp_u16_t remote_idx);
void lldp_mgmt_address_to_string (lldp_remote_entry_t xdata * entry, lldp_u8_t xdata * dest);
#endif

