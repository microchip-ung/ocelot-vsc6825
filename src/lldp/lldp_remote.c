//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#include "common.h"     /* Always include common.h at the first place of user-defined herder files */

#include "lldp_sm.h"
#include "lldp.h"
#include "vtss_common_os.h"
#include "lldp_remote.h"
#include "lldp_tlv.h"
#include "uartdrv.h"
#include "print.h"
#include "lldp_private.h"

#if TRANSIT_LLDP

#include "eee_api.h"
#include "misc1.h"
/*****************************************************************************
 *
 *
 * Defines
 *
 *
 *
 ****************************************************************************/

#define LLDP_REMOTE_ENTRIES LLDP_PORTS
#define MSAP_ID_IDX_UNKNOWN 0xFF


#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif /* MIN */

#ifndef MAX
#define MAX(a,b) ((a)<(b)?(b):(a))
#endif /* MAX */

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
static lldp_u8_t msap_id_idx (lldp_rx_remote_entry_t xdata * rx_entry);
static lldp_u8_t compare_msap_ids (lldp_rx_remote_entry_t xdata * rx_entry, lldp_remote_entry_t xdata * remote_entry);
static void update_entry (lldp_rx_remote_entry_t xdata * rx_entry, lldp_remote_entry_t xdata * entry);
static lldp_bool_t insert_new_entry (lldp_rx_remote_entry_t xdata * rx_entry);
static lldp_bool_t update_neccessary (lldp_rx_remote_entry_t xdata * rx_entry, lldp_remote_entry_t xdata * entry);

static void print_characters (lldp_u8_t xdata * p, lldp_u8_t len);
static void print_hex_value (lldp_u8_t xdata * p, lldp_u8_t len);
static lldp_u8_t string_is_printable (lldp_u8_t xdata * p, lldp_u8_t len);
static void remote_port_id_to_string (lldp_remote_entry_t xdata * entry);
static void remote_chassis_id_to_string (lldp_remote_entry_t xdata * entry);
static void delete_entry (lldp_rx_remote_entry_t xdata * rx_entry);
static void too_many_neighbors_discard_current_lldpdu (lldp_rx_remote_entry_t xdata * rx_entry);
static void print_mgmt_addr_type (lldp_remote_entry_t xdata * entry);
static void mib_stats_table_changed_now (void);
static void mib_stats_inc_inserts (void);
static void mib_stats_inc_deletes (void);
static void mib_stats_inc_drops (void);
static void mib_stats_inc_ageouts (void);
static void update_entry_mib_info (lldp_remote_entry_t xdata * entry, lldp_bool_t update_remote_idx);
static lldp_u8_t get_next(lldp_u32_t time_mark, lldp_port_t port, lldp_u16_t remote_idx);
static lldp_u8_t compare_values (lldp_u32_t time_mark, lldp_port_t port, lldp_u16_t remote_idx, lldp_remote_entry_t xdata * entry);

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
 ****************************************************************************/

static lldp_u8_t too_many_neighbors = LLDP_FALSE;
static lldp_timer_t too_many_neighbors_timer = 0;
static lldp_remote_entry_t remote_entries[LLDP_REMOTE_ENTRIES];
static lldp_mib_stats_t lldp_mib_stats = {0};
static lldp_u32_t last_remote_index = 0;


void lldp_remote_delete_entries_for_local_port (lldp_port_t port)
{
    lldp_u8_t i;
    for(i = 0; i < LLDP_REMOTE_ENTRIES; i++) {
        if(remote_entries[i].in_use && (remote_entries[i].receive_port == port)) {
            remote_entries[i].in_use = 0;
#if TRANSIT_EEE_LLDP
            remote_entries[i].is_eee = 0;
#endif
            mib_stats_table_changed_now();
            mib_stats_inc_deletes();
        }
    }
}


void lldp_remote_1sec_timer (void)
{
    lldp_u8_t i;
    lldp_sm_t xdata * sm;

    for(i = 0; i < LLDP_REMOTE_ENTRIES; i++) {
        if(remote_entries[i].in_use) {
            if(remote_entries[i].rx_info_ttl > 0) {
                if(--remote_entries[i].rx_info_ttl == 0) {
                    /* timer expired, we shall remove this entry */
                    remote_entries[i].in_use = 0;
#if TRANSIT_EEE_LLDP
                    remote_entries[i].is_eee = 0;
#endif
                    mib_stats_table_changed_now();
                    mib_stats_inc_ageouts();
                    mib_stats_inc_deletes();
                    VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_DEBUG, ("Ageing performed for remote entry on port %u",
                                      (unsigned)remote_entries[i].receive_port));
                    sm = lldp_get_port_sm(remote_entries[i].receive_port);
                    sm->rx.rxInfoAge = LLDP_TRUE;
                    sm->stats.statsAgeoutsTotal++;
                    lldp_sm_step(sm);

                    /* note that we do not clear tooManyNeighbors here but wait until its timer runs out */
                }
            }
        }
    }

    if(too_many_neighbors) {
        if(too_many_neighbors_timer > 0) {
            if(--too_many_neighbors_timer == 0) {
                too_many_neighbors = LLDP_FALSE;
            }
        }
    }
}

lldp_bool_t lldp_remote_handle_msap (lldp_rx_remote_entry_t xdata * rx_entry)
{
    lldp_u8_t msap_idx;


    /*
    ** Check if we need to delete this entry (if TTL = 0)
    */
    if(rx_entry->ttl == 0) {
        VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_DEBUG, ("TTL is 0. Will delete remote entry"));
        delete_entry(rx_entry);
        return LLDP_FALSE; /* doesn't really matter what we return here */
    }

    /*
    ** check to see if this is an already know MSAP Identifier in our
    ** remote entries table
    */
    msap_idx = msap_id_idx(rx_entry);
    if(msap_idx == MSAP_ID_IDX_UNKNOWN) {
        VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_DEBUG, ("Inserting new remote entry"));
        return insert_new_entry(rx_entry);
    } else {
        if(update_neccessary(rx_entry, &remote_entries[msap_idx])) {
            VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_DEBUG, ("Updating remote entry"));
            update_entry(rx_entry, &remote_entries[msap_idx]);
            update_entry_mib_info(&remote_entries[msap_idx], LLDP_FALSE);
            return LLDP_TRUE;
        } else {
            VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_DEBUG, ("No update neccessary - new rxTTL=%u", (unsigned)rx_entry->ttl));
            remote_entries[msap_idx].rx_info_ttl = rx_entry->ttl;
        }
    }

    return LLDP_FALSE;
}

#if TRANSIT_EEE_LLDP || UNMANAGED_LLDP_DEBUG_IF
lldp_u8_t lldp_remote_get_max_entries (void)
{
    return LLDP_REMOTE_ENTRIES;
}

lldp_remote_entry_t xdata * lldp_get_remote_entry (lldp_u8_t idx)
{
    return &remote_entries[idx];
}
#endif

#if UNMANAGED_LLDP_DEBUG_IF
static void remote_chassis_id_to_string (lldp_remote_entry_t xdata * entry)
{
    switch(entry->chassis_id_subtype) {
    case 2: /* interface alias */
    case 3: /* port component */
    case 6: /* interface name */
    case 7: /* locally assigned */
        if(string_is_printable(entry->chassis_id, entry->chassis_id_length)) {
            print_characters(entry->chassis_id, entry->chassis_id_length);
        } else {
            print_hex_value(entry->chassis_id, entry->chassis_id_length);
        }
        break;

    case 4: /* MAC address */
        print_mac_addr(entry->chassis_id);
        break;

    case 5: /* network address */
        if(entry->chassis_id[0] == 1) { /* IANA Address Family = IPv4 */
            print_ip_addr(&entry->chassis_id[1]);
        } else {
            print_hex_value(entry->chassis_id, entry->chassis_id_length);
        }
        break;

        /* we just show the binary (hex) data for the following */
    case 0: /* reserved */
    case 1: /* chassis component */
    default: /* reserved */
        print_hex_value(entry->chassis_id, entry->chassis_id_length);
        break;
    }
}

static void remote_port_id_to_string (lldp_remote_entry_t xdata * entry)
{
    switch(entry->port_id_subtype) {
    case 1: /* interface Alias */
    case 2: /* port component */
    case 5: /* interface name */
    case 7: /* locally assigned */
        if(string_is_printable(entry->port_id, entry->port_id_length)) {
            print_characters(entry->port_id, entry->port_id_length);
        } else {
            print_hex_value(entry->port_id, entry->port_id_length);
        }
        break;

    case 3: /* MAC address */
        print_mac_addr(entry->port_id);
        break;

    case 4: /* network address */
        if(entry->port_id[0] == 1) { /* IANA Address Family = IPv4 */
            print_ip_addr(&entry->port_id[1]);
        } else {
            print_hex_value(entry->port_id, entry->port_id_length);
        }
        break;

        /* we just show the binary (hex) data for the following */
    case 0: /* reserved */
    case 6: /* agent circuit ID */
    default: /* reserved */
        print_hex_value(entry->port_id, entry->port_id_length);
        break;
    }
}

void lldp_remote_tlv_to_string (lldp_remote_entry_t xdata * entry, lldp_u8_t field, lldp_u8_t xdata * dest)
{
    lldp_u8_t xdata * p = 0;
    lldp_u8_t len = 0;
#if !TRANSIT_LLDP_REDUCED
    lldp_u16_t capa;
    lldp_u16_t capa_ena;
    lldp_u8_t bit_no;
#endif

    const char * sys_capa[] = {"Other", "Repeater", "Bridge", "WLAN Access Point", "Router", "Telephone", "DOCSIS cable device", "Station Only"};
    lldp_bool_t print_comma = LLDP_FALSE;

#if !TRANSIT_LLDP_REDUCED
    uart_redirect(dest);
#endif /* !TRANSIT_LLDP_REDUCED */
    switch(field) {
    case LLDP_TLV_BASIC_MGMT_CHASSIS_ID:
        remote_chassis_id_to_string(entry);
        break;

    case LLDP_TLV_BASIC_MGMT_PORT_ID:
        remote_port_id_to_string(entry);
        break;
#if !TRANSIT_LLDP_REDUCED
    case LLDP_TLV_BASIC_MGMT_PORT_DESCR:
        len = MIN(entry->port_description_length, MAX_PORT_DESCR_LENGTH);
        p = entry->port_description;
        break;

    case LLDP_TLV_BASIC_MGMT_SYSTEM_NAME:
        len = MIN(entry->system_name_length, MAX_SYSTEM_NAME_LENGTH);
        p = entry->system_name;
        break;

    case LLDP_TLV_BASIC_MGMT_SYSTEM_DESCR:
        len = MIN(entry->system_description_length, MAX_SYSTEM_DESCR_LENGTH);
        p = entry->system_description;
        break;

    case LLDP_TLV_BASIC_MGMT_SYSTEM_CAPA:
        capa     = ((lldp_u16_t)entry->system_capabilities[0]) << 8 | entry->system_capabilities[1];
        capa_ena = ((lldp_u16_t)entry->system_capabilities[2]) << 8 | entry->system_capabilities[3];

        for(bit_no = 0; bit_no <= 7; bit_no++) {
            if(capa & (1<<bit_no)) {
                if(print_comma) {
                    print_ch(',');
                    print_spaces(1);
                }
                print_comma = LLDP_TRUE;
                print_str(sys_capa[bit_no]);
                /* print enabled/disabled indicaion */
                print_ch('(');
                if(capa_ena & (1<<bit_no)) {
                    print_ch('+');
                } else {
                    print_ch('-');
                }
                print_ch(')');
            }
        }
        break;

    case LLDP_TLV_BASIC_MGMT_MGMT_ADDR:
        switch(entry->mgmt_address_subtype) {
        case 1: /* ipv4 */
            print_ip_addr(entry->mgmt_address);
            break;

        default:
            if(string_is_printable(entry->mgmt_address, entry->mgmt_address_length)) {
                print_characters(entry->mgmt_address, entry->mgmt_address_length);
            } else {
                print_hex_value(entry->mgmt_address, entry->mgmt_address_length);
            }
            break;
        }

        if(entry->mgmt_address_length > 0) {
            print_spaces(2);
            print_mgmt_addr_type(entry);
        }
        break;
#endif

#if UNMANAGED_EEE_DEBUG_IF
    case LLDP_TLV_ORG_EEE_TLV:
        if(entry->is_eee) {
            print_dec_nright(entry->xmit_time, 8);
            print_dec_nright(entry->recv_time, 8);
            print_dec_nright(entry->fallback_recv_time, 11);
            print_dec_nright(entry->echo_xmit_time, 13);
            print_dec_nright(entry->echo_recv_time, 13);
            print_dec_nright(eee_sm[uport2iport(entry->receive_port)].tm.LocResolvedTxSystemValue, 14);
            print_dec_nright(eee_sm[uport2iport(entry->receive_port)].tm.LocResolvedRxSystemValue, 14);
        }
        break;
#endif
    default:
        break;
    }

    if(len > 0) {
        if(string_is_printable(p, len)) {
            print_characters(p, len);
        } else {
            print_hex_value(p, len);
        }
    }
#if !TRANSIT_LLDP_REDUCED
    uart_redirect(0);
#endif /* !TRANSIT_LLDP_REDUCED */
}

void lldp_chassis_type_to_string (lldp_remote_entry_t xdata * entry, lldp_u8_t xdata * dest)
{
#if !TRANSIT_LLDP_REDUCED
    uart_redirect(dest);
#endif /* !TRANSIT_LLDP_REDUCED */
    switch(entry->chassis_id_subtype) {
    case 1: /* entPhyClass = port(10) | backplane(4) */
    case 3: /* entPhyClass = chassis(3) */
        print_str("EntPhysicalAlias");
        break;
    case 2:
        print_str("ifAlias");
        break;
    case 4:
        print_str("MAC-address");
        break;
    case 5:
        print_str("Network Address");
        break;
    case 6:
        print_str("ifName");
        break;
    case 7:
        print_str("local");
        break;
    }
#if !TRANSIT_LLDP_REDUCED
    uart_redirect(0);
#endif /* !TRANSIT_LLDP_REDUCED */
}

void lldp_port_type_to_string (lldp_remote_entry_t xdata * entry, lldp_u8_t xdata * dest)
{
#if !TRANSIT_LLDP_REDUCED
    uart_redirect(dest);
#endif /* !TRANSIT_LLDP_REDUCED */
    switch(entry->port_id_subtype) {
    case 1:
        print_str("ifAlias");
        break;
    case 2:
        print_str("entPhysicalAlias");
        break;
    case 3:
        print_str("MAC-address");
        break;
    case 4:
        print_str("Network Address");
        break;
    case 5:
        print_str("ifName");
        break;
    case 6:
        print_str("Agent circuit ID");
        break;
    case 7:
        print_str("local");
        break;
    }
#if !TRANSIT_LLDP_REDUCED
    uart_redirect(0);
#endif /* !TRANSIT_LLDP_REDUCED */
}

#if 0
lldp_mib_stats_t xdata * lldp_get_mib_stats (void)
{
    return &lldp_mib_stats;
}


lldp_remote_entry_t xdata * lldp_remote_get(lldp_u32_t time_mark, lldp_port_t port, lldp_u16_t remote_idx)
{
    lldp_u8_t i;

    /* run through all entries */
    for(i = 0; i < LLDP_REMOTE_ENTRIES; i++) {
        if(remote_entries[i].in_use) {
            /* if entry is equal to  what user supplied */
            if(compare_values(time_mark, port, remote_idx, &remote_entries[i]) == 2) {
                return &remote_entries[i];
            }
        }
    }
    return XNULL;
}

lldp_remote_entry_t xdata * lldp_remote_get_next(lldp_u32_t time_mark, lldp_port_t port, lldp_u16_t remote_idx)
{
    lldp_u8_t idx;

    idx = get_next(time_mark, port, remote_idx);
    if(idx == 0xFF) {
        /* no more entries larger than current */
        return XNULL;
    } else {
        return &remote_entries[idx];
    }
}

lldp_remote_entry_t xdata * lldp_remote_get_next_non_zero_addr (lldp_u32_t time_mark, lldp_port_t port, lldp_u16_t remote_idx)
{
    lldp_u8_t idx;

    for(;;) {
        idx = get_next(time_mark, port, remote_idx);
        /* if no more entries, get out */
        if(idx == 0xFF) {
            break;
        }

        /* if we found something with a management address */
        if(remote_entries[idx].mgmt_address_length != 0) {
            break;
        }

        /* otherwise, try to proceed, starting with this entry without management address  */
        time_mark = remote_entries[idx].time_mark;
        port = remote_entries[idx].receive_port;
        remote_idx = remote_entries[idx].lldp_remote_index;
    };

    if(idx == 0xFF) {
        /* no more entries larger than current */
        return XNULL;
    } else {
        return &remote_entries[idx];
    }
}
#endif

#if !TRANSIT_LLDP_REDUCED
static void print_mgmt_addr_type (lldp_remote_entry_t xdata * entry)
{
    /* from http://www.iana.org/assignments/address-family-numbers
       Number    Description                                          Reference
       ------    ---------------------------------------------------- ---------
       0    Reserved
       1    IP (IP version 4)
       2    IP6 (IP version 6)
       3    NSAP
       4    HDLC (8-bit multidrop)
       5    BBN 1822
       6    802 (includes all 802 media plus Ethernet "canonical format")
       7    E.163
       8    E.164 (SMDS, Frame Relay, ATM)
       9    F.69 (Telex)
       10    X.121 (X.25, Frame Relay)
       11    IPX
       12    Appletalk
       13    Decnet IV
       14    Banyan Vines
       15    E.164 with NSAP format subaddress           [UNI-3.1] [Malis]
       16    DNS (Domain Name System)
       17    Distinguished Name                                    [Lynn]
       18    AS Number                                             [Lynn]
       19    XTP over IP version 4                                 [Saul]
       20    XTP over IP version 6                                 [Saul]
       21    XTP native mode XTP                                   [Saul]
       22    Fibre Channel World-Wide Port Name                   [Bakke]
       23    Fibre Channel World-Wide Node Name                   [Bakke]
       24    GWID                                                 [Hegde]
       65535    Reserved
    */
    /* we support the following subset of these... */
    print_ch('(');
    switch(entry->mgmt_address_subtype) {
    case 1:
        print_str("IPv4");
        break;
    case 2:
        print_str("IPv6");
        break;
    case 3:
        print_str("NSAP");
        break;
    case 6:
        print_str("802");
        break;
    case 11:
        print_str("IPX");
        break;
    case 12:
        print_str("Appletalk");
        break;
    case 16:
        print_str("DNS");
        break;
    case 17:
        print_str("Distinguished Name");
        break;
    default:
        print_str("Other");
        break;
    }
    print_ch(')');
}
#endif

static lldp_u8_t string_is_printable (lldp_u8_t xdata * p, lldp_u8_t len)
{
    while(len--) {
        if((*p > 31) && (*p < 127)) {
            p++;
        } else {
            return LLDP_FALSE;
        }
    }
    return LLDP_TRUE;
}

static void print_characters (lldp_u8_t xdata * p, lldp_u8_t len)
{
    while(len--) {
        print_ch(*p++);
    }
}

static void print_hex_value (lldp_u8_t xdata * p, lldp_u8_t len)
{
    while(len--) {
        print_hex_b(*p);
        print_ch(' ');
        p++;
    }
}
#endif

/* returns 0 if msap identifiers are equal, 1 otherwise */
static lldp_u8_t compare_msap_ids (lldp_rx_remote_entry_t xdata * rx_entry, lldp_remote_entry_t xdata * remote_entry)
{
    /* perform simple tests prior to doing memory compare */
    if(rx_entry->chassis_id_subtype != remote_entry->chassis_id_subtype) {
        return 1;
    }

    if(rx_entry->chassis_id_length != remote_entry->chassis_id_length) {
        return 1;
    }

    if(rx_entry->port_id_subtype != remote_entry->port_id_subtype) {
        return 1;
    }

    if(rx_entry->port_id_length != remote_entry->port_id_length) {
        return 1;
    }

    if(xmemcmp(rx_entry->chassis_id, remote_entry->chassis_id, rx_entry->chassis_id_length) != 0) {
        return 1;
    }

    if(xmemcmp(rx_entry->port_id, remote_entry->port_id, rx_entry->port_id_length) != 0) {
        return 1;
    }

    return 0;
}

static lldp_u8_t msap_id_idx (lldp_rx_remote_entry_t xdata * rx_entry)
{
    lldp_u8_t i;
    for(i = 0; i < LLDP_REMOTE_ENTRIES; i++) {
        if(remote_entries[i].in_use) {
            if(compare_msap_ids(rx_entry, &remote_entries[i]) == 0) {
                VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_DEBUG, ("Found MSAP identifier in index %u",
                                  (unsigned)i));
                return i;
            }
        }
    }

    VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_DEBUG, ("MSAP identifier unknown"));
    return MSAP_ID_IDX_UNKNOWN;
}


static lldp_bool_t insert_new_entry (lldp_rx_remote_entry_t xdata * rx_entry)
{
    lldp_u8_t i;
    for(i = 0; i < LLDP_REMOTE_ENTRIES; i++) {
        if(!remote_entries[i].in_use) {
            update_entry(rx_entry, &remote_entries[i]);
            update_entry_mib_info(&remote_entries[i], LLDP_TRUE);
            return LLDP_TRUE;
        }
    }

    /* no room */

    /* for now we just discard the new LLDPDU */
    too_many_neighbors_discard_current_lldpdu(rx_entry);
    mib_stats_inc_drops();

    return LLDP_FALSE;
}

static void update_entry_mib_info (lldp_remote_entry_t xdata * entry, lldp_bool_t update_remote_idx)
{
    if(update_remote_idx) {
        entry->lldp_remote_index = ++last_remote_index;
    }
    entry->time_mark = lldp_os_get_sys_up_time();
}

static void too_many_neighbors_discard_current_lldpdu (lldp_rx_remote_entry_t xdata * rx_entry)
{
    lldp_sm_t xdata * sm;

    sm = lldp_get_port_sm(rx_entry->receive_port);
    too_many_neighbors = LLDP_TRUE;
    too_many_neighbors_timer = MAX(too_many_neighbors_timer, rx_entry->ttl);
    sm->stats.statsFramesDiscardedTotal++;
}

static void update_entry (lldp_rx_remote_entry_t xdata * rx_entry, lldp_remote_entry_t xdata * entry)
{
    entry->receive_port = rx_entry->receive_port;

    entry->chassis_id_subtype = rx_entry->chassis_id_subtype;
    entry->chassis_id_length = rx_entry->chassis_id_length;
    memcpy(entry->chassis_id, rx_entry->chassis_id, MIN(rx_entry->chassis_id_length, MAX_CHASSIS_ID_LENGTH));
    entry->chassis_id[MAX_CHASSIS_ID_LENGTH] = '\0';

    entry->port_id_subtype = rx_entry->port_id_subtype;
    entry->port_id_length = rx_entry->port_id_length;
    memcpy(entry->port_id, rx_entry->port_id, MIN(rx_entry->port_id_length, MAX_PORT_ID_LENGTH));
    entry->port_id[MAX_PORT_ID_LENGTH] = '\0';

#if !TRANSIT_LLDP_REDUCED
    entry->port_description_length = rx_entry->port_description_length;
    memcpy(entry->port_description, rx_entry->port_description, MIN(rx_entry->port_description_length, MAX_PORT_DESCR_LENGTH));
    entry->port_description[MAX_PORT_DESCR_LENGTH] = '\0';

    entry->system_name_length = rx_entry->system_name_length;
    memcpy(entry->system_name, rx_entry->system_name, MIN(rx_entry->system_name_length, MAX_SYSTEM_NAME_LENGTH));
    entry->system_name[MAX_SYSTEM_NAME_LENGTH] = '\0';

    entry->system_description_length = rx_entry->system_description_length;
    memcpy(entry->system_description, rx_entry->system_description, MIN(rx_entry->system_description_length, MAX_SYSTEM_DESCR_LENGTH));
    entry->system_description[MAX_SYSTEM_DESCR_LENGTH] = '\0';

    memcpy(entry->system_capabilities, rx_entry->system_capabilities, sizeof(entry->system_capabilities));

    entry->mgmt_address_subtype = rx_entry->mgmt_address_subtype;
    entry->mgmt_address_length = rx_entry->mgmt_address_length;
    memcpy(entry->mgmt_address, rx_entry->mgmt_address, rx_entry->mgmt_address_length);
    entry->mgmt_address_if_number_subtype = rx_entry->mgmt_address_if_number_subtype;

    memcpy(entry->mgmt_address_if_number, rx_entry->mgmt_address_if_number, sizeof(entry->mgmt_address_if_number));

    entry->oid_length = rx_entry->oid_length;
    memcpy(entry->oid, rx_entry->oid, MIN(rx_entry->oid_length, sizeof(entry->oid)));
    rx_entry->oid[MAX_MGMT_OID_LENGTH] = '\0';
#endif

#if TRANSIT_EEE_LLDP
    if(entry->mgmt_ieee_subtype != rx_entry->mgmt_ieee_subtype) {
        entry->mgmt_ieee_subtype = rx_entry->mgmt_ieee_subtype;
    }

    if(entry->xmit_time != rx_entry->xmit_time) {
        entry->xmit_time = rx_entry->xmit_time;
    }
    if(entry->recv_time != rx_entry->recv_time) {
        entry->recv_time = rx_entry->recv_time;
    }
    if(entry->fallback_recv_time != rx_entry->fallback_recv_time) {
        entry->fallback_recv_time = rx_entry->fallback_recv_time;
    }
    if(entry->echo_xmit_time != rx_entry->echo_xmit_time) {
        entry->echo_xmit_time = rx_entry->echo_xmit_time;
    }
    if(entry->echo_recv_time != rx_entry->echo_recv_time) {
        entry->echo_recv_time = rx_entry->echo_recv_time;
    }
    entry->is_eee = 1;
#endif

    entry->in_use = 1;
    entry->rx_info_ttl = rx_entry->ttl;
    entry->something_changed_remote = 1;
    mib_stats_table_changed_now();
    /* fixme: determine if an update shall count as an insert */
    mib_stats_inc_inserts();
}

static lldp_bool_t update_neccessary (lldp_rx_remote_entry_t xdata * rx_entry, lldp_remote_entry_t xdata * entry)
{
    /* we don't need to check the MSAP id here, we already know these are identical */

    /* we check simple variables first */
    if(rx_entry->receive_port != entry->receive_port) {
        VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_DEBUG, ("- Receive port"));
        return LLDP_TRUE;
    }

#if !TRANSIT_LLDP_REDUCED
    if(rx_entry->port_description_length != entry->port_description_length) {
        VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_DEBUG, ("- Port descr. len"));
        return LLDP_TRUE;
    }

    if(rx_entry->system_name_length != entry->system_name_length) {
        VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_DEBUG, ("- Sys name len"));
        return LLDP_TRUE;
    }

    if(rx_entry->system_description_length != entry->system_description_length) {
        VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_DEBUG, ("- Sys descr len"));
        return LLDP_TRUE;
    }


    if(rx_entry->mgmt_address_subtype != entry->mgmt_address_subtype) {
        VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_DEBUG, ("- mgmt addr subtype"));
        return LLDP_TRUE;
    }

    if(rx_entry->mgmt_address_length != entry->mgmt_address_length) {
        VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_DEBUG, ("- mgmt addr len"));
        return LLDP_TRUE;
    }

    if(rx_entry->mgmt_address_if_number_subtype != entry->mgmt_address_if_number_subtype) {
        VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_DEBUG, ("- mgmt addr ifnnumber subtype"));
        return LLDP_TRUE;
    }

    if(rx_entry->oid_length != entry->oid_length) {
        VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_DEBUG, ("- oid len"));
        return LLDP_TRUE;
    }


    /* now check with memcmps */
    if(memcmp(rx_entry->port_description, entry->port_description, rx_entry->port_description_length) != 0) {
        VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_DEBUG, ("- port descr"));
        return LLDP_TRUE;
    }

    if(memcmp(rx_entry->system_name, entry->system_name, rx_entry->system_name_length) != 0) {
        VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_DEBUG, ("- sys name"));
        return LLDP_TRUE;
    }

    if(memcmp(rx_entry->system_description, entry->system_description, rx_entry->system_description_length) != 0) {
        VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_DEBUG, ("- sys descr"));
        return LLDP_TRUE;
    }

    if(memcmp(rx_entry->system_capabilities, entry->system_capabilities, sizeof(entry->system_capabilities)) != 0) {
        VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_DEBUG, ("- sys capa"));
        return LLDP_TRUE;
    }

    if(memcmp(rx_entry->mgmt_address, entry->mgmt_address, rx_entry->mgmt_address_length) != 0) {
        VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_DEBUG, ("- mgmt addr"));
        return LLDP_TRUE;
    }

    if(memcmp(rx_entry->mgmt_address_if_number, entry->mgmt_address_if_number, sizeof(entry->mgmt_address_if_number)) != 0) {
        VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_DEBUG, ("- mgmt addr ifnum"));
        return LLDP_TRUE;
    }

    if(memcmp(rx_entry->oid, entry->oid, rx_entry->oid_length) != 0) {
        VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_DEBUG, ("- oid"));
        return LLDP_TRUE;
    }
#endif

#if TRANSIT_EEE_LLDP
    if(entry->mgmt_ieee_subtype != rx_entry->mgmt_ieee_subtype) {
        VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_DEBUG, ("- mgmt IEEE 802.3 subtype"));
        return LLDP_TRUE;
    }
    if(entry->xmit_time != rx_entry->xmit_time) {
        VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_DEBUG, ("- Transmit Tw"));
        return LLDP_TRUE;
    }
    if(entry->recv_time != rx_entry->recv_time) {
        VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_DEBUG, ("- Receive Tw"));
        return LLDP_TRUE;
    }
    if(entry->fallback_recv_time != rx_entry->fallback_recv_time) {
        VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_DEBUG, ("- Fallback Tw"));
        return LLDP_TRUE;
    }
    if(entry->echo_xmit_time != rx_entry->echo_xmit_time) {
        VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_DEBUG, ("- Echo Transmit Tw"));
        return LLDP_TRUE;
    }
    if(entry->echo_recv_time != rx_entry->echo_recv_time) {
        VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_DEBUG, ("- Echo Receive Tw"));
        return LLDP_TRUE;
    }
#endif

    /* everything the same */
    VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_DEBUG, ("... Current entry in remote table is up to date"));
    return LLDP_FALSE;
}

static void delete_entry (lldp_rx_remote_entry_t xdata * rx_entry)
{
    lldp_u8_t idx;
    /* try to find the existing index */
    idx = msap_id_idx(rx_entry);

    if(idx != MSAP_ID_IDX_UNKNOWN) {
        /* delete it */
        remote_entries[idx].in_use = 0;
#if TRANSIT_EEE_LLDP
        remote_entries[idx].is_eee = 0;
#endif
        mib_stats_table_changed_now();
        mib_stats_inc_deletes();
    }
}

static void mib_stats_table_changed_now (void)
{
    lldp_mib_stats.table_change_time = lldp_os_get_sys_up_time();
}

static void mib_stats_inc_inserts (void)
{
    lldp_mib_stats.table_inserts++;
}

static void mib_stats_inc_deletes (void)
{
    lldp_mib_stats.table_deletes++;
}

static void mib_stats_inc_drops (void)
{
    lldp_mib_stats.table_drops++;
}

static void mib_stats_inc_ageouts (void)
{
    lldp_mib_stats.table_ageouts++;
}

#if UNMANAGED_LLDP_DEBUG_IF
#if 0
static lldp_u8_t compare_values (lldp_u32_t time_mark, lldp_port_t port, lldp_u16_t remote_idx, lldp_remote_entry_t xdata * entry)
{
    if(entry->time_mark < time_mark) {
        return 1;
    } else if(entry->time_mark == time_mark) {
        if(entry->receive_port < port) {
            return 1;
        } else if(entry->receive_port == port) {
            if(entry->lldp_remote_index < remote_idx) {
                return 1;
            }
            if(entry->lldp_remote_index == remote_idx) {
                return 2;
            }
        }
    }

    return 0;
}

static lldp_u8_t get_next (lldp_u32_t time_mark, lldp_port_t port, lldp_u16_t remote_idx)
{
    lldp_u8_t i;
    lldp_u8_t idx_low = 0xFF;
    lldp_u32_t time_mark_low = ~0;
    lldp_port_t port_low = ~0;
    lldp_u16_t remote_idx_low = ~0;

    /* run through all entries */
    for(i = 0; i < LLDP_REMOTE_ENTRIES; i++) {
        if(remote_entries[i].in_use) {
            /* if entry is larger than what user supplied */
            if(compare_values(time_mark, port, remote_idx, &remote_entries[i]) == 0) {
                /* and lower than the current low */
                if(compare_values(time_mark_low, port_low, remote_idx_low, &remote_entries[i]) == 1) {
                    /* remember this index */
                    idx_low = i;
                    /* and update criteria */
                    time_mark_low  = remote_entries[i].time_mark;
                    port_low       = remote_entries[i].receive_port;
                    remote_idx_low = remote_entries[i].lldp_remote_index;
                }
            }
        }
    }

    return idx_low;
}
#endif
#endif

#endif

