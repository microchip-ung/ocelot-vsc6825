//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#include "common.h"     /* Always include common.h at the first place of user-defined herder files */

#include "lldp_sm.h"
#include "lldp.h"
#include "vtss_common_os.h"
#include "lldp_tlv.h"
#include "lldp_private.h"
#if TRANSIT_EEE_LLDP
#include "eee_api.h"
#include "eee_base_api.h"
#endif

#if TRANSIT_LLDP

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
static lldp_u16_t append_chassis_id (lldp_u8_t xdata * buf);
static void set_tlv_type_and_length (lldp_u8_t xdata * buf, lldp_u8_t tlv_type, lldp_u16_t tlv_info_string_len);
static lldp_u16_t append_port_id (lldp_u8_t xdata * buf, lldp_port_t port);
static lldp_u16_t append_ttl (lldp_u8_t xdata * buf, lldp_port_t port);
static lldp_u16_t append_end_of_pdu (void);
static lldp_u16_t append_port_descr (lldp_u8_t xdata * buf, lldp_port_t port);
static lldp_u16_t append_system_name (lldp_u8_t xdata * buf);
static lldp_u16_t append_system_descr (lldp_u8_t xdata * buf);
static lldp_u16_t append_system_capabilities (lldp_u8_t xdata * buf);
static lldp_u16_t append_mgmt_address (lldp_u8_t xdata * buf, lldp_port_t port);
#if TRANSIT_EEE_LLDP
static lldp_u16_t append_eee_l2_capability (lldp_u8_t xdata * buf, lldp_port_t port);
#endif
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

lldp_u16_t lldp_tlv_add (lldp_u8_t xdata * buf, lldp_u16_t cur_len, lldp_tlv_t tlv, lldp_port_t port)
{
    lldp_u16_t tlv_info_len = 0;

    switch(tlv) {
    case LLDP_TLV_BASIC_MGMT_CHASSIS_ID:
        tlv_info_len = append_chassis_id(buf + 2);
        break;

    case LLDP_TLV_BASIC_MGMT_PORT_ID:
        tlv_info_len = append_port_id(buf + 2, port);
        break;

    case LLDP_TLV_BASIC_MGMT_TTL:
        tlv_info_len = append_ttl(buf + 2, port);
        break;

    case LLDP_TLV_BASIC_MGMT_END_OF_LLDPDU:
        tlv_info_len = append_end_of_pdu();
        break;

#if TRANSIT_EEE_LLDP
    case LLDP_TLV_ORG_EEE_TLV:
        /* we only include the 802.3az information when PHY is supported. */
        tlv_info_len = append_eee_l2_capability(buf + 2 , port);
        /* not included, return current length */
        if(!tlv_info_len)
            return cur_len;
        break;
#endif
    default:
        VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_DEBUG, ("Unhandled TLV Type %u", (unsigned)tlv));
        return cur_len;
        break;
    }

#if TRANSIT_EEE_LLDP
    if(tlv == LLDP_TLV_ORG_EEE_TLV) {
        set_tlv_type_and_length (buf, LLDP_TLV_ORG_TLV, tlv_info_len);
    } else
#endif
        set_tlv_type_and_length (buf, tlv, tlv_info_len);
    /* add additional 2 octets for header */
    return cur_len + 2 + tlv_info_len;
}

lldp_u16_t lldp_tlv_add_zero_ttl (lldp_u8_t xdata * buf, lldp_u16_t cur_len)
{
    buf[2] = 0;
    buf[3] = 0;
    set_tlv_type_and_length (buf, LLDP_TLV_BASIC_MGMT_TTL, 2);
    return cur_len + 4;
}

#if 0
lldp_u32_t lldp_tlv_mgmt_addr_len (void)
{
    return 5;
}
#endif



lldp_u8_t lldp_tlv_get_local_port_id (lldp_port_t port, lldp_u8_t xdata* port_str)
{
    lldp_u8_t len;
    /*
    ** for port id, we append "locally assigned" port id and just use the physical port number
    ** as local identifier
    */
    if(port > 9) {
        port_str[0] = '0' + (port / 10);
        port_str[1] = '0' + (port % 10);
        len = 2;
    } else {
        port_str[0] = '0' + port;
        len = 1;
    }
    return len;
}


static void set_tlv_type_and_length (lldp_u8_t xdata * buf, lldp_u8_t tlv_type, lldp_u16_t tlv_info_string_len)
{
    buf[0] = (0xfe & (tlv_type << 1)) | (tlv_info_string_len >> 7);
    buf[1] = tlv_info_string_len & 0xFF;
}

static lldp_u16_t append_chassis_id (lldp_u8_t xdata * buf)
{
    vtss_common_macaddr_t mac_addr;

    /*
    ** we append MAC address, which gives us length MAC_ADDRESS + Chassis id Subtype, hence
    ** information string length = 7
    */
    buf[0] = lldp_tlv_get_chassis_id_subtype(); /* chassis ID subtype */
    vtss_os_get_systemmac(&mac_addr);
    memcpy(&buf[1], mac_addr.macaddr, VTSS_COMMON_MACADDR_SIZE);
    return 7;
}

static lldp_u16_t append_port_id (lldp_u8_t xdata * buf, lldp_port_t port)
{
    lldp_u8_t len;

    buf[0] = lldp_tlv_get_port_id_subtype(); /* Port ID subtype */
    len = lldp_tlv_get_local_port_id(port, &buf[1]);
    return 1 + len;
}

static lldp_u16_t append_ttl (lldp_u8_t xdata * buf, lldp_port_t port)
{
    lldp_sm_t xdata * sm;
    sm = lldp_get_port_sm(port);

    buf[0] = HIGH_BYTE(sm->tx.txTTL);
    buf[1] = LOW_BYTE(sm->tx.txTTL);

    return 2;
}

static lldp_u16_t append_end_of_pdu (void)
{
    return 0;
}

#if TRANSIT_EEE_LLDP
static lldp_u16_t append_eee_l2_capability (lldp_u8_t xdata * buf, lldp_port_t port)
{
    ushort value;

    vtss_iport_no_t     iport = uport2iport(port);;

    if(!read_eee_conf_mode(iport))
        return 0; // EEE is supported bewteen local or link partner.

    memcpy(buf, ieee_802_3_oui_header, 3);
    buf[3] = 0x05; /* 802.3 Subtype 0x05, Table 79–1, IEEE802.3az D3*/

    /* The following information is based on a port */
    value = eee_transmit_time(iport);
    memcpy(&buf[4], (uchar *) &value, 2);
    value = eee_receive_time(iport);
    memcpy(&buf[6], (uchar *) &value, 2);
    value = eee_fallback_receive_time(iport);
    memcpy(&buf[8], (uchar *) &value, 2);
    value = eee_echo_transmit_time(iport);
    memcpy(&buf[10], (uchar *) &value, 2);
    value = eee_echo_receive_time(iport);
    memcpy(&buf[12], (uchar *) &value, 2);

    return 14;
}
#endif

#endif

