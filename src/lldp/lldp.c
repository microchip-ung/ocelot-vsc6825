//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#include "common.h"     /* Always include common.h at the first place of user-defined herder files */

#include "lldp_sm.h"
#include "lldp.h"
#include "vtss_common_os.h"
#include "lldp_tlv.h"
#include "lldp_remote.h"
#include "lldp_private.h"

#include "h2txrx.h"
#include "misc1.h"

#include "print.h"

#ifndef NDEBUG
#include "print.h"
#endif

#if TRANSIT_LLDP

#if TRANSIT_EEE
#include "eee_api.h"
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
static lldp_bool_t validate_lldpdu (lldp_sm_t xdata * sm, lldp_rx_remote_entry_t xdata * rx_entry);
static void bad_lldpdu (lldp_sm_t xdata * sm);
static lldp_u16_t get_tlv_info_len (lldp_u8_t xdata * tlv);
static lldp_u8_t get_tlv_type (lldp_u8_t xdata * tlv);
static lldp_bool_t validate_tlv (lldp_u8_t xdata * tlv);
#ifndef NDEBUG
static void debug_dump_lldpdu (lldp_rx_remote_entry_t xdata * rx_entry);
#else
#define debug_dump_lldpdu(e)
#endif /* NDEBUG */

/*****************************************************************************
 *
 *
 * Public data
 *
 *
 *
 ****************************************************************************/

/* IEEE 802.3 OUI */
const uchar ieee_802_3_oui_header[3] = {0x00, 0x12, 0x0f};
const uchar ieee_802_3_az_subtype    = 0x05; /* D3.2 */
const mac_addr_t xdata mac_addr_lldp = {0x01, 0x80, 0xC2, 0x00, 0x00, 0x0E};

/*****************************************************************************
 *
 *
 * Local data
 *
 *
 *
 ****************************************************************************/
/* the state machines, one per switch port */
static lldp_sm_t xdata lldp_sm[LLDP_PORTS];

/* frame_len is file-global as it is shared between 2 functions */
static lldp_u16_t frame_len;

static lldp_u8_t xdata * lldp_rx_frame;
static lldp_u16_t lldp_rx_frame_len;


lldp_sm_t xdata * lldp_get_port_sm (lldp_port_t port)
{
    return &lldp_sm[port-1];
}


void lldp_tx_initialize_lldp (lldp_sm_t xdata * sm)
{
    sm->tx.somethingChangedLocal = LLDP_FALSE;
    sm->adminStatus = lldp_os_get_admin_status(sm->port_no);

    sm->tx.re_evaluate_timers = LLDP_FALSE;
}

void lldp_set_port_enabled (lldp_port_t port, lldp_u8_t enabled)
{
    lldp_sm_t xdata * sm;

    sm = lldp_get_port_sm(port);
    sm->portEnabled = enabled;

    VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_DEBUG, ("Port %u new state: %u", (unsigned)sm->port_no, (unsigned)enabled));
}

#if TRANSIT_UNMANAGED_SYS_MAC_CONF || defined(TRANSIT_WEB)
/*
 * Call this function on change to any of the following:
 *      ip address
 *      system name
 */
void lldp_something_changed_local (void)
{
    lldp_port_t port;
    for(port = 0; port < LLDP_PORTS; port++) {
        lldp_sm[port].tx.somethingChangedLocal = LLDP_TRUE;
    }
}
#endif /* TRANSIT_UNMANAGED_SYS_MAC_CONF || defined(TRANSIT_WEB) */

void lldp_init (void)
{
    lldp_port_t port;

    mac_addr_t  mac_addr;

#if TRANSIT_BPDU_PASS_THROUGH
    h2_bpdu_t_registration(0x0e, TRUE);
#endif // TRANSIT_BPDU_PASS_THROUGH

    load_lldp_conf();
    flash_read_mac_addr(mac_addr);
    if(mac_cmp(mac_addr, mac_addr_0) == 0 ||
            mac_addr[0] & 0x01)
        for(port = 1; port <= LLDP_PORTS; port++) {
            /* force lldp mode disable */
            lldp_os_set_admin_status(port, LLDP_DISABLED);
        }

    for(port = 0; port < LLDP_PORTS; port++) {
        lldp_sm_init(&lldp_sm[port], 1+port);
    }
}

void lldp_1sec_timer_tick (void)
{
    lldp_port_t port;

    for(port = 0; port < LLDP_PORTS; port++) {
            lldp_port_timers_tick(&lldp_sm[port]);
    }

    lldp_remote_1sec_timer();
}


/* The transmission module has been split up into two logical functions
** to be consistent with 802.1AB
*/

void lldp_construct_info_lldpdu (lldp_port_t port_no)
{
    lldp_u8_t xdata * buf;
    lldp_u8_t code optional_tlvs[] = { LLDP_TLV_BASIC_MGMT_PORT_DESCR,
                                       LLDP_TLV_BASIC_MGMT_SYSTEM_NAME,
                                       LLDP_TLV_BASIC_MGMT_SYSTEM_DESCR,
                                       LLDP_TLV_BASIC_MGMT_SYSTEM_CAPA,
                                       LLDP_TLV_BASIC_MGMT_MGMT_ADDR
#if TRANSIT_EEE_LLDP
                                       , LLDP_TLV_ORG_EEE_TLV
#endif
                                     };
    lldp_u8_t tlv;

    buf = lldp_os_get_frame_storage();

    /* reserve room for DA, SA and EthType */
    frame_len = 14;

    /* Append Mandatory TLVs */
    frame_len = lldp_tlv_add(&buf[frame_len], frame_len, LLDP_TLV_BASIC_MGMT_CHASSIS_ID, port_no);
    frame_len = lldp_tlv_add(&buf[frame_len], frame_len, LLDP_TLV_BASIC_MGMT_PORT_ID, port_no);
    frame_len = lldp_tlv_add(&buf[frame_len], frame_len, LLDP_TLV_BASIC_MGMT_TTL, port_no);

    /* Append enabled optional TLVs */
    for(tlv = 0; tlv < sizeof(optional_tlvs) / sizeof(optional_tlvs[0]); tlv++) {
        if(lldp_os_get_optional_tlv_enabled(optional_tlvs[tlv])) {
            frame_len = lldp_tlv_add(&buf[frame_len], frame_len, optional_tlvs[tlv], port_no);
        }
    }

    /* End of LLDPPDU is also mandatory */
    frame_len = lldp_tlv_add(&buf[frame_len], frame_len, LLDP_TLV_BASIC_MGMT_END_OF_LLDPDU, port_no);

    VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_DEBUG, ("Port %u Construct Info PDU, length: %u", (unsigned)port_no, (unsigned)frame_len));
}

#if TRANSIT_THERMAL
void lldp_pre_port_disabled (lldp_port_t port_no)
{
    lldp_port_t iport = uport2iport(port_no);
    if(((lldp_sm[iport].adminStatus == LLDP_ENABLED_RX_TX) ||
            (lldp_sm[iport].adminStatus == LLDP_ENABLED_TX_ONLY)) &&
            lldp_sm[iport].portEnabled) {
        /* make sure we don't use shared buffer */
        lldp_os_set_tx_in_progress(LLDP_TRUE);

        /* construct & transmit shutdown LLDPDU */
        lldp_construct_shutdown_lldpdu(port_no);
        lldp_tx_frame(port_no);

        /* Now we can use shared buffer again */
        lldp_os_set_tx_in_progress(LLDP_FALSE);
    }
}
#endif

void lldp_construct_shutdown_lldpdu (lldp_port_t port_no)
{
    lldp_u8_t xdata * buf;

    buf = lldp_os_get_frame_storage();

    /* reserve room for DA, SA and EthType */
    frame_len = 14;

    /* Append Mandatory TLVs, now with zeroed TTL */
    frame_len = lldp_tlv_add(&buf[frame_len], frame_len, LLDP_TLV_BASIC_MGMT_CHASSIS_ID, port_no);
    frame_len = lldp_tlv_add(&buf[frame_len], frame_len, LLDP_TLV_BASIC_MGMT_PORT_ID, port_no);
    frame_len = lldp_tlv_add_zero_ttl(&buf[frame_len], frame_len);

    /* End of LLDPPDU is also mandatory */
    frame_len = lldp_tlv_add(&buf[frame_len], frame_len, LLDP_TLV_BASIC_MGMT_END_OF_LLDPDU, port_no);

    VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_DEBUG, ("Port %u Construct Shutdown PDU, length: %u", (unsigned)port_no, (unsigned)frame_len));
}

void lldp_tx_frame (lldp_port_t port_no)
{
    lldp_u8_t xdata * buf;
    vtss_common_macaddr_t mac_addr;
    lldp_sm_t xdata * sm;

    sm = lldp_get_port_sm(port_no);
    sm->stats.statsFramesOutTotal++;

    //vtss_os_get_systemmac(&mac_addr);
    vtss_os_get_portmac(port_no, &mac_addr);

    buf = lldp_os_get_frame_storage();

    /* fill in SA, DA + eth Type */
    buf[0] = 0x01;
    buf[1] = 0x80;
    buf[2] = 0xC2;
    buf[3] = 0x00;
    buf[4] = 0x00;
    buf[5] = 0x0E;

    VTSS_COMMON_MACADDR_ASSIGN(&buf[6], mac_addr.macaddr);

    buf[12] = 0x88;
    buf[13] = 0xCC;

    VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_DEBUG, ("Port %u Tx Frame", (unsigned)port_no));
    lldp_os_tx_frame(port_no, buf, frame_len);
}


void lldp_frame_received (lldp_port_t port_no, lldp_u8_t xdata * frame, lldp_u16_t len)
{
    lldp_sm_t xdata * sm;
    vtss_iport_no_t     iport = uport2iport(port_no);

    VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_DEBUG, ("Port %u Rx LLDP Frame, length=%u",
                      (unsigned)port_no,
                      (unsigned)len));
                      
		//print_str("port: "); print_dec(port_no);  print_str("len: ");  print_dec(len); print_cr_lf();                   
    /*
    ** it has already been verified that both the ethtype and the DSTMAC is as expected
    ** Now we can kick the state machine
    */

    lldp_rx_frame = frame;
    lldp_rx_frame_len = len;

    sm = lldp_get_port_sm(iport);
    sm->rx.rcvFrame = LLDP_TRUE;
    lldp_sm_step(sm);
}

void lldp_rx_initialize_lldp (lldp_port_t port)
{
    /* Delete all information in remote systems mib with this local port */
    lldp_remote_delete_entries_for_local_port(port);
}


#if 0 //uncall functions
lldp_counter_t lldp_get_tx_frames (lldp_port_t port)
{
    return lldp_sm[uport2iport(port)].stats.statsFramesOutTotal;
}

lldp_counter_t lldp_get_rx_total_frames (lldp_port_t port)
{
    return lldp_sm[uport2iport(port)].stats.statsFramesInTotal;
}

lldp_counter_t lldp_get_rx_error_frames (lldp_port_t port)
{
    return lldp_sm[uport2iport(port)].stats.statsFramesInErrorsTotal;
}

lldp_counter_t lldp_get_rx_discarded_frames (lldp_port_t port)
{
    return lldp_sm[uport2iport(port)].stats.statsFramesDiscardedTotal;
}

lldp_counter_t lldp_get_TLVs_discarded (lldp_port_t port)
{
    return lldp_sm[uport2iport(port)].stats.statsTLVsDiscardedTotal;
}

lldp_counter_t lldp_get_TLVs_unrecognized (lldp_port_t port)
{
    return lldp_sm[uport2iport(port)].stats.statsTLVsUnrecognizedTotal;
}

lldp_counter_t lldp_get_TLVs_org_discarded (lldp_port_t port)
{
    return lldp_sm[uport2iport(port)].stats.statsOrgTVLsDiscarded;
}

lldp_counter_t lldp_get_ageouts (lldp_port_t port)
{
    return lldp_sm[uport2iport(port)].stats.statsAgeoutsTotal;

}
#endif

void lldp_rx_process_frame (lldp_sm_t xdata * sm)
{
    lldp_rx_remote_entry_t xdata received_entry = {0};
    sm->stats.statsFramesInTotal++;

    received_entry.receive_port = sm->port_no;

    if(!validate_lldpdu(sm, &received_entry)) {
        /* discard LLDPDU. badFrame has already been set true by validation function */
        return;
    }

#ifndef NDEBUG
    debug_dump_lldpdu(&received_entry);
#endif

    /* pass to remote entries handler */
    if(lldp_remote_handle_msap(&received_entry)) {
        sm->rx.rxChanges = LLDP_TRUE;
    } else {
        sm->rx.rxChanges = LLDP_FALSE;
    }
}

#ifndef NDEBUG
static lldp_u8_t is_printable (lldp_u8_t xdata * p, lldp_u8_t len)
{
    while(len--) {
        if(*p > 31 && *p < 127) {
            p++;
        } else {
            return LLDP_FALSE;
        }
    }
    return LLDP_TRUE;
}

static void debug_dump_utf8 (char * prefix, lldp_u8_t * p, lldp_u8_t len)
{
    lldp_u8_t i;

    print_str(prefix);
    print_str(": ");

    if(is_printable(p, len)) {
        for(i = 0; i < len; i++) {
            print_ch(p[i]);
        }
    } else {
        for(i = 0; i < len; i++) {
            print_hex_prefix();
            print_hex_b(p[i]);
            print_ch(' ');
        }
    }
    print_cr_lf();
}

static void debug_dump_lldpdu (lldp_rx_remote_entry_t xdata * rx_entry)
{
    VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_DEBUG, ("Validated TLV Follows:"));
    VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_DEBUG, ("Receive port: %u", (unsigned)rx_entry->receive_port));
    VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_DEBUG, ("Chassis ID Subtype: %u", (unsigned)rx_entry->chassis_id_subtype));
    debug_dump_utf8("Chassis ID", rx_entry->chassis_id, rx_entry->chassis_id_length);
    VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_DEBUG, ("Port ID Subtype: %u", (unsigned)rx_entry->port_id_subtype));
    VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_DEBUG, ("TTL: %u", (unsigned)rx_entry->ttl));
    debug_dump_utf8("Port ID", rx_entry->port_id, rx_entry->port_id_length);
    debug_dump_utf8("Port Description", rx_entry->port_description, rx_entry->port_description_length);
    debug_dump_utf8("System Name", rx_entry->system_name, rx_entry->system_name_length);
    debug_dump_utf8("System Description", rx_entry->system_description, rx_entry->system_description_length);
    debug_dump_utf8("System Capabilities", rx_entry->system_capabilities, 4);

    VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_DEBUG, ("Mgmt address subtype: %u", (unsigned)rx_entry->mgmt_address_subtype));
    if(rx_entry->mgmt_address_subtype == 1) { /* IPv4 */
        print_str("Mgmt address: ");
        print_ip_addr(rx_entry->mgmt_address);
        print_cr_lf();
    } else {
        debug_dump_utf8("Mgmt address", rx_entry->mgmt_address, rx_entry->mgmt_address_length);
    }

    VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_DEBUG, ("interface numbering subtype: %u", (unsigned)rx_entry->mgmt_address_if_number_subtype));
    debug_dump_utf8("interface number", rx_entry->mgmt_address_if_number, 4);
    debug_dump_utf8("OID", rx_entry->oid, rx_entry->oid_length);

#if TRANSIT_EEE
    VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_DEBUG, ("IEEE 802.3az subtype: %u", (unsigned)rx_entry->mgmt_ieee_subtype));
#endif
}
#endif /* NDEBUG */

static void bad_lldpdu (lldp_sm_t xdata * sm)
{
    sm->stats.statsFramesDiscardedTotal++;
    sm->stats.statsFramesInErrorsTotal++;
    sm->rx.badFrame = LLDP_TRUE;
}

static lldp_u8_t get_tlv_type (lldp_u8_t xdata * tlv)
{
    return ((*tlv) >> 1);
}

static lldp_u16_t get_tlv_info_len (lldp_u8_t xdata * tlv)
{
    lldp_u16_t info_len = *tlv & 1;
    info_len <<= 8;
    info_len |= tlv[1];
    return info_len;
}


static lldp_bool_t validate_tlv (lldp_u8_t xdata * tlv)
{
    lldp_u8_t tlv_type;
    tlv_type = get_tlv_type(tlv);

    if((tlv_type == LLDP_TLV_BASIC_MGMT_CHASSIS_ID) ||
            (tlv_type == LLDP_TLV_BASIC_MGMT_PORT_ID)    ||
            (tlv_type == LLDP_TLV_BASIC_MGMT_TTL)) {
        return LLDP_FALSE;
    }

    return LLDP_TRUE;
}

static lldp_bool_t validate_lldpdu (lldp_sm_t xdata * sm, lldp_rx_remote_entry_t xdata * rx_entry)
{
    lldp_u8_t xdata * tlv;
    lldp_u16_t len;
    lldp_u8_t tlv_no;
    lldp_u8_t tlv_type;

    /* do some basic validation that we have a somewhat reasonable LLDPDU to work with */
    if(lldp_rx_frame_len < 26) {
        bad_lldpdu(sm);
        return LLDP_FALSE;
    }

    /* the numbers below refers to sections in IEEE802.1AB */

    /* 10.3.2 - check three mandatory TLVs at beginning of LLDPDU */

    /* 10.3.2.a check CHASSIS ID TLV */
    tlv = &lldp_rx_frame[14];

    if(get_tlv_type(tlv) != LLDP_TLV_BASIC_MGMT_CHASSIS_ID) {
        VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_DEBUG, ("TLV 1 not CHASSIS ID but %u", (unsigned)get_tlv_type(tlv)));
        bad_lldpdu(sm);
        return LLDP_FALSE;
    }

    len = get_tlv_info_len(tlv);
    if((256 < len) || (len < 2)) {
        VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_DEBUG, ("TLV 1 length %u error", (unsigned)len));
        bad_lldpdu(sm);
        return LLDP_FALSE;
    }

    rx_entry->chassis_id_length = len - 1;
    rx_entry->chassis_id_subtype = tlv[2];
    rx_entry->chassis_id = &tlv[3];

    /* advance to next tlv */
    tlv += (2+len);

    /* 10.3.2.b check PORT ID TLV */
    if(get_tlv_type(tlv) != LLDP_TLV_BASIC_MGMT_PORT_ID) {
        VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_DEBUG, ("TLV 2 not PORT ID"));
        bad_lldpdu(sm);
        return LLDP_FALSE;
    }

    len = get_tlv_info_len(tlv);
    if((256 < len) || (len < 2)) {
        VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_DEBUG, ("TLV 2 length %u error", (unsigned)len));
        bad_lldpdu(sm);
        return LLDP_FALSE;
    }

    rx_entry->port_id_length = len - 1;
    rx_entry->port_id_subtype = tlv[2];
    rx_entry->port_id = &tlv[3];

    /* advance to next tlv */
    tlv += (2+len);

    /* 10.3.2.c check TIME TO LIVE TLV */
    if(get_tlv_type(tlv) != LLDP_TLV_BASIC_MGMT_TTL) {
        VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_DEBUG, ("TLV 3 not TTL"));
        bad_lldpdu(sm);
        return LLDP_FALSE;
    }

    len = get_tlv_info_len(tlv);
    if(len < 2) {
        VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_DEBUG, ("TLV 3 length %u error", (unsigned)len));
        bad_lldpdu(sm);
        return LLDP_FALSE;
    }

    rx_entry->ttl = (((lldp_u16_t)tlv[2]) << 8) | (tlv[3]);
    sm->rx.rxTTL = rx_entry->ttl;

    /* advance to next tlv */
    tlv += (2+len);

    /* we have reached tlv # 4 */
    tlv_no = 4;

    /* iterate all other TLVs and save those we support */
    while(tlv < (lldp_rx_frame + lldp_rx_frame_len)) {
        if(!validate_tlv(tlv)) {
            VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_DEBUG, ("TLV %u error", (unsigned)tlv_no));
            bad_lldpdu(sm);
            return LLDP_FALSE;
        }

        len = get_tlv_info_len(tlv);
        tlv_type = get_tlv_type(tlv);

        if (((tlv_type == LLDP_TLV_BASIC_MGMT_PORT_DESCR) ||
                (tlv_type == LLDP_TLV_BASIC_MGMT_SYSTEM_NAME) ||
                (tlv_type == LLDP_TLV_BASIC_MGMT_SYSTEM_DESCR)) &&
                (256 < len)) {
            VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_DEBUG, ("TLV %u length %u error", (unsigned)tlv_type, (unsigned)len));
            bad_lldpdu(sm);
            return LLDP_FALSE;
        }

        /*
        ** we know for sure that this TLV is not:
        ** chassis id, port id or ttl
        */
        switch(tlv_type) {
        case LLDP_TLV_BASIC_MGMT_END_OF_LLDPDU:
            /* end validation thing immediately */
            if (sm->rx.badFrame == LLDP_TRUE) { //During TLVs parsing process, having TLV not supported, then return FALSE for state machine transition
                return LLDP_FALSE;
            }

            return LLDP_TRUE;

        case LLDP_TLV_ORG_TLV:
#if TRANSIT_EEE
            if(len == 14 && xmemcmp((tlv + 2), ieee_802_3_oui_header, 3) == 0) {
                rx_entry->mgmt_ieee_subtype = tlv[5];
                if(rx_entry->mgmt_ieee_subtype == ieee_802_3_az_subtype) { // ???
                		println_str("TRANSIT_EEE");
                    memcpy(&rx_entry->xmit_time, &tlv[6], 2);
                    memcpy(&rx_entry->recv_time, &tlv[8], 2);
                    memcpy(&rx_entry->fallback_recv_time, &tlv[10], 2);
                    memcpy(&rx_entry->echo_xmit_time, &tlv[12], 2);
                    memcpy(&rx_entry->echo_recv_time, &tlv[14], 2);
                } else {
                    VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_DEBUG, ("Ignoring Org. TLV no %u - type %u length %u",
                                      (unsigned)tlv_no,
                                      (unsigned)tlv_type,
                                      (unsigned)len));
                    sm->stats.statsOrgTVLsDiscarded++;
                }
            } else
#endif
            {
                    VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_DEBUG, ("Ignoring Org. TLV no %u - type %u length %u",
                                      (unsigned)tlv_no,
                                      (unsigned)tlv_type,
                                      (unsigned)len));
                    sm->stats.statsOrgTVLsDiscarded++;
                    sm->rx.badFrame = LLDP_TRUE;
                }
            break;

        default:
            if((9 <= tlv_type) && (tlv_type <= 126)) {
                sm->stats.statsTLVsUnrecognizedTotal++;
            }

            VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_DEBUG, ("Ignoring TLV no %u - type %u length %u",
                              (unsigned)tlv_no,
                              (unsigned)tlv_type,
                              (unsigned)len));
            break;
        }

        /* advance */
        tlv += (2+len);

        tlv_no++;
    }

    /* we didn't see any END OF LLDPDU, so we return false */
    return LLDP_FALSE;
}

#endif


