//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#include "common.h"     /* Always include common.h at the first place of user-defined header files */
#include "h2tcam.h"
#include "h2io.h"
#include "vtss_api_base_regs.h"
#include "print.h"
#include "phydrv.h"
#include "hwport.h"
#include <string.h>

#if TRANSIT_TCAM_IS2

/*****************************************************************************
 *
 *
 * Defines
 *
 *
 *
 ****************************************************************************/
#define MAX_ACE_LIMIT     0xffff 
#define VCAP_CMD_WRITE    0 //000: Write from cache to VCAP
#define VCAP_CMD_READ     1 //001: Read from VCAP to cache
#define VCAP_CMD_MOV_UP   2 //010: Move entry and/or action up (decreasing addresses)
#define VCAP_CMD_MOV_DOWN 3 //011: Move entry and/or action down (increasing addresses)
#define VCAP_CMD_INIT     4 //100: Initialize VCAP with the cache-value

#define S2_TYPE_MAC_ETYPE    0
#define S2_TYPE_IPV4_TCP_UDP 4
#define S2_TYPE_IPV6_TCP_UDP 0 
#define MAC_ETYPE_PTP        0x88f7
#define PTP_UDP_PORT         319

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
 * Public data
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
typedef enum {
    IS2_HALF_TYPE=0,
    IS2_HALF_FIRST_LOOKUP,
    IS2_HALF_INGR_PORT_MASK,
    IS2_HALF_ETHERNET_PROTO,
    IS2_HALF_ETH_PTP_MSG_TYPE,
    IS2_HALF_ETH_TWO_STEP,
    IS2_HALF_IPV4_IS_IPV4,
    IS2_HALF_IPV4_UDP_TYPE,
    IS2_HALF_IPV4_DPORT,
    IS2_HALF_IPV4_PTP_MSG_TYPE,
    IS2_HALF_IPV4_TWO_STEP,
    IS2_FULL_TYPE,
    IS2_FULL_FIRST_LOOKUP,
    IS2_FULL_INGR_PORT_MASK,
    IS2_FULL_IPV6_UDP_TYPE,
    IS2_FULL_IPV6_DPORT,
    IS2_FULL_IPV6_PTP_MSG_TYPE,
    IS2_FULL_IPV6_TWO_STEP,
} is2_conf_param_t;

typedef struct {
    u8 half_type:4;
    u8 is_first_lookup:1;
    u16 ing_port_mask:12;
} h2_is2_half_common_conf_t;

typedef struct {
    u8 full_type:2;
    u8 is_first_lookup:1;
    u16 ing_port_mask:12;
} h2_is2_full_common_conf_t;

typedef struct {
    u16 etype;
    struct {
        union {
            u16 l2_payload0;
            u16 l2_payload1:8;
            u16 l2_payload2:3;
        } eth_payload;
        union {
            u8 ptp_msg_type:4;
            u8 ptp_two_step:1;
        } ptp_payload;
    } payload;
} h2_is2_half_eth_conf_t;

typedef struct {
    u8 ipv4:1;
    u8 tcp:1;
    u16 dport;
    struct {
        union {
            u8 ptp_msg_type:4;
            u8 ptp_two_step:1;
        }ptp_payload;
    } payload;
} h2_is2_half_ipv4_conf_t;

typedef struct {
    u8 tcp:1;
    u16 dport;
    struct {
        union {
            u8 ptp_msg_type:4;
            u8 ptp_two_step:1;
        }ptp_payload;
    } payload;
} h2_is2_full_ipv6_tcp_udp_conf_t;

typedef struct {
    union {
        h2_is2_half_common_conf_t common_half;
        h2_is2_full_common_conf_t common_full;
    } common;
    union {
        h2_is2_half_eth_conf_t eth;
        h2_is2_half_ipv4_conf_t ipv4;
        h2_is2_full_ipv6_tcp_udp_conf_t ipv6;
    } pkt_info;
} h2ace_is2_t;
static u16 is2_entry_id;
static void h2tcam_add_is2_ptp_ethernet(u16 entry_id,tcam_e2e_tc_t ace_type); 
static void h2tcam_add_is2_ptp_ipv4(u16 entry_id,tcam_e2e_tc_t ace_type);
static void h2tcam_entry_cache_to_tcam(u16 entry_id,const tcam_data_conf_t *conf); 
static void h2tcam_entry_tcam_to_cache(u16 entry_id,tcam_data_conf_t *conf);
static void h2tcam_action_cache_to_tcam(u16 entry_id,const tcam_data_conf_t *conf);
static void h2tcam_action_tcam_to_cache(u16 entry_id,tcam_data_conf_t *conf);
static void h2tcam_bit_feild_32bit_set(u16 offset,u16 length,u32 *value,u32 *conf); 
static void h2tcam_is2_half_conf(u32 *conf,is2_conf_param_t conf_type,u32 *value);
static u8 is_in_1_word(u8 bf_lsb, u8 length);
static void h2tcam_is2_get_def_conf(tcam_e2e_tc_t ace_type,h2ace_is2_t *ace_conf);
static void h2tcam_is2_set_common_conf(h2ace_is2_t *ace_conf,tcam_data_conf_t *tcam_conf,tcam_e2e_tc_t ace_type);
static void h2tcam_is2_set_ipv4_conf(h2ace_is2_t *ace_conf,tcam_data_conf_t *tcam_conf);
static void h2tcam_add_is2_ptp_ipv6(u16 entry_id,tcam_e2e_tc_t ace_type);
static void h2tcam_is2_set_ipv6_conf(h2ace_is2_t *ace_conf,tcam_data_conf_t *tcam_conf);

/* =================================================================
 *  TCAM - Ternary content addressable memory 
 * =================================================================
 * Initialize TCAM 
 *
 * Return : None
 * =================================================================
 */
void h2_tcam_init(void)
{
    u8 port_ext;

    /* TCAM BIST to be verified ,TBD */

    /* Initialize IS2 */
    H2_WRITE_MASKED(VTSS_VCAP_CORE_VCAP_CORE_CFG_VCAP_MV_CFG(VTSS_TO_VCAP_IS2),
            VTSS_F_VCAP_CORE_VCAP_CORE_CFG_VCAP_MV_CFG_MV_SIZE(0xffff),
            VTSS_M_VCAP_CORE_VCAP_CORE_CFG_VCAP_MV_CFG_MV_SIZE);
    H2_WRITE_MASKED(VTSS_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL(VTSS_TO_VCAP_IS2),
            VTSS_F_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL_UPDATE_CMD(VCAP_CMD_INIT) |
            VTSS_F_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL_UPDATE_SHOT(1),
            VTSS_M_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL_UPDATE_CMD |
            VTSS_M_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL_UPDATE_SHOT );

    // Enable IS2
    for (port_ext = 1; port_ext <= NO_OF_BOARD_PORTS; port_ext++) {
        H2_WRITE_MASKED(VTSS_ANA_PORT_VCAP_S2_CFG(uport2cport(port_ext)),
                VTSS_F_ANA_PORT_VCAP_S2_CFG_S2_ENA(1) |
                VTSS_F_ANA_PORT_VCAP_S2_CFG_S2_IP6_CFG(0),
                VTSS_M_ANA_PORT_VCAP_S2_CFG_S2_ENA|
                VTSS_M_ANA_PORT_VCAP_S2_CFG_S2_IP6_CFG);
    }

    /* Initialize ES0 */
    H2_WRITE_MASKED(VTSS_VCAP_CORE_VCAP_CORE_CFG_VCAP_MV_CFG(VTSS_TO_VCAP_ES0),
            VTSS_F_VCAP_CORE_VCAP_CORE_CFG_VCAP_MV_CFG_MV_SIZE(0xffff),
            VTSS_M_VCAP_CORE_VCAP_CORE_CFG_VCAP_MV_CFG_MV_SIZE);
    H2_WRITE_MASKED(VTSS_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL(VTSS_TO_VCAP_ES0),
            VTSS_F_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL_UPDATE_CMD(VCAP_CMD_INIT) |
            VTSS_F_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL_UPDATE_SHOT(1),
            VTSS_M_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL_UPDATE_CMD |
            VTSS_M_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL_UPDATE_SHOT );

    /* Initialize IS1 */
    H2_WRITE_MASKED(VTSS_VCAP_CORE_VCAP_CORE_CFG_VCAP_MV_CFG(VTSS_TO_VCAP_IS1),
            VTSS_F_VCAP_CORE_VCAP_CORE_CFG_VCAP_MV_CFG_MV_SIZE(0xffff),
            VTSS_M_VCAP_CORE_VCAP_CORE_CFG_VCAP_MV_CFG_MV_SIZE);
    H2_WRITE_MASKED(VTSS_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL(VTSS_TO_VCAP_IS1),
            VTSS_F_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL_UPDATE_CMD(VCAP_CMD_INIT) |
            VTSS_F_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL_UPDATE_SHOT(1),
            VTSS_M_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL_UPDATE_CMD |
            VTSS_M_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL_UPDATE_SHOT );

}
#if UNMANAGED_TCAM_DEBUG_IF

static void h2tcam_entry_tcam_to_cache(u16 entry_id,tcam_data_conf_t *conf) 
{
    u8 cntr;
    H2_WRITE_MASKED(VTSS_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL(VTSS_TO_VCAP_IS2),
            VTSS_F_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL_UPDATE_CMD(VCAP_CMD_READ) |
            VTSS_F_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL_UPDATE_ENTRY_DIS(0) |
            VTSS_F_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL_UPDATE_ADDR(entry_id) |
            VTSS_F_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL_UPDATE_SHOT(1),
            VTSS_M_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL_UPDATE_CMD |
            VTSS_M_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL_UPDATE_ENTRY_DIS |
            VTSS_M_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL_UPDATE_ADDR |
            VTSS_M_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL_UPDATE_SHOT );

    // Type group 
    H2_READ(VTSS_VCAP_CORE_VCAP_CORE_CACHE_VCAP_TG_DAT(VTSS_TO_VCAP_IS2),
            conf->tg);

    //Cache entry and mask
    for (cntr=0;cntr<12;cntr++) {
        H2_READ(VTSS_VCAP_CORE_VCAP_CORE_CACHE_VCAP_ENTRY_DAT(VTSS_TO_VCAP_IS2,cntr),conf->tcam_entry[cntr]);
        H2_READ(VTSS_VCAP_CORE_VCAP_CORE_CACHE_VCAP_MASK_DAT(VTSS_TO_VCAP_IS2,cntr),conf->tcam_mask[cntr]);
    }

    H2_READ(VTSS_VCAP_CORE_VCAP_CORE_CACHE_VCAP_CNT_DAT(VTSS_TO_VCAP_IS2,0),conf->tcam_count);
}

static void h2tcam_action_tcam_to_cache(u16 entry_id,tcam_data_conf_t *conf) 
{
    H2_WRITE_MASKED(VTSS_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL(VTSS_TO_VCAP_IS2),
            VTSS_F_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL_UPDATE_CMD(VCAP_CMD_READ) |
            VTSS_F_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL_UPDATE_ENTRY_DIS(0) |
            VTSS_F_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL_UPDATE_ADDR(entry_id) |
            VTSS_F_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL_UPDATE_SHOT(1),
            VTSS_M_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL_UPDATE_CMD |
            VTSS_M_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL_UPDATE_ENTRY_DIS |
            VTSS_M_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL_UPDATE_ADDR |
            VTSS_M_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL_UPDATE_SHOT );
    //ACTION entries
    H2_READ(VTSS_VCAP_CORE_VCAP_CORE_CACHE_VCAP_ACTION_DAT(VTSS_TO_VCAP_IS2,0),conf->tcam_action[0]);
    H2_READ(VTSS_VCAP_CORE_VCAP_CORE_CACHE_VCAP_ACTION_DAT(VTSS_TO_VCAP_IS2,1),conf->tcam_action[1]);
}

u16 h2_tcam_count_get(const tcam_target_t trgt)
{
    u16 r_value;
    switch(trgt){
        case TCAM_TARGET_IS2:
            r_value = is2_entry_id;
            break;
        case TCAM_TARGET_ES0:
        case TCAM_TARGET_IS1:
        default:
            r_value = 0;
            break;
    }
    return r_value;
}

void h2_tcam_e2e_tc_get(u16 entry_id,tcam_data_conf_t *conf) 
{
    h2tcam_entry_tcam_to_cache(entry_id,conf);
    h2tcam_action_tcam_to_cache(entry_id,conf);
}
#endif //#if UNMANAGED_TCAM_DEBUG_IF

void h2_tcam_e2e_tc_set(tcam_e2e_tc_t ace_conf) 
{
    if ( is2_entry_id <= MAX_ACE_LIMIT ) { 
        if (ace_conf == TCAM_E2E_TC_PTP_ETH) {
            h2tcam_add_is2_ptp_ethernet(is2_entry_id++,ace_conf);
        } else if (ace_conf == TCAM_E2E_TC_PTP_IP) {
            h2tcam_add_is2_ptp_ipv4(is2_entry_id++,ace_conf);
        } else if (ace_conf == TCAM_E2E_TC_PTP_IPV6) {
            h2tcam_add_is2_ptp_ipv6(is2_entry_id++,ace_conf);
        }
    } else {
        print_hex_dw(MAX_ACE_LIMIT);
        println_str(" MAX ACE limit reached");
    }
}
static void h2tcam_add_is2_ptp_ipv6(u16 entry_id,tcam_e2e_tc_t ace_type) 
{
    tcam_data_conf_t tcam_conf;
    h2ace_is2_t ace_conf;
    u32 value[2];

    memset(&tcam_conf,0,sizeof(tcam_data_conf_t));

    h2tcam_is2_get_def_conf(ace_type,&ace_conf);
    h2tcam_is2_set_common_conf(&ace_conf,&tcam_conf,ace_type);
    h2tcam_is2_set_ipv6_conf(&ace_conf,&tcam_conf);

    //ACTION entries
    value[0] = 0x2;
    h2tcam_bit_feild_32bit_set(32,9,value,tcam_conf.tcam_action);

    // Type group MAC_ETYPE TG = 2= 1010 = 0xa
    tcam_conf.tg = 0x55;
    tcam_conf.tcam_count = 0;
    tcam_conf.entry_length = 11;
    tcam_conf.action_length = 1;
    h2tcam_action_cache_to_tcam(entry_id,&tcam_conf);
    h2tcam_entry_cache_to_tcam(entry_id,&tcam_conf);
}

static void h2tcam_add_is2_ptp_ipv4(u16 entry_id,tcam_e2e_tc_t ace_type) 
{
    tcam_data_conf_t tcam_conf;
    h2ace_is2_t ace_conf;
    u32 value[2];

    memset(&tcam_conf,0,sizeof(tcam_data_conf_t));

    h2tcam_is2_get_def_conf(ace_type,&ace_conf);
    h2tcam_is2_set_common_conf(&ace_conf,&tcam_conf,ace_type);
    h2tcam_is2_set_ipv4_conf(&ace_conf,&tcam_conf);
    
    ////ACTION entries
    value[0] = 0x2;
    h2tcam_bit_feild_32bit_set(32,9,value,tcam_conf.tcam_action);

    // Type group MAC_ETYPE TG = 2= 1010 = 0xa
    tcam_conf.tg = 0xa;
    tcam_conf.tcam_count = 0;
    tcam_conf.entry_length = 6;
    tcam_conf.action_length = 1;
    h2tcam_action_cache_to_tcam(entry_id,&tcam_conf);
    h2tcam_entry_cache_to_tcam(entry_id,&tcam_conf);
}

static void get_offset_len_par(is2_conf_param_t type,u16 *offset)
{
    switch(type) {
        case IS2_HALF_TYPE :
            offset[0] = 0;
            offset[1] = 4;
            break;
        case IS2_HALF_FIRST_LOOKUP :
            offset[0] = 4;
            offset[1] = 1;
            break;
        case IS2_HALF_INGR_PORT_MASK :
            offset[0] = 13;
            offset[1] = 12;
            break;
        case IS2_HALF_ETHERNET_PROTO :
            offset[0] = 142;
            offset[1] = 16;
            break;
        case IS2_HALF_ETH_PTP_MSG_TYPE:
            offset[0] = 167;
            offset[1] = 4;
            break;
        case IS2_HALF_ETH_TWO_STEP:
            offset[0] = 182;
            offset[1] = 1;
            break;
        case IS2_HALF_IPV4_IS_IPV4:
            offset[0] = 46;
            offset[1] = 1;
            break;
        case IS2_HALF_IPV4_UDP_TYPE:
            offset[0] = 124;
            offset[1] = 1;
            break;
        case IS2_HALF_IPV4_DPORT:
            offset[0] = 141;
            offset[1] = 16;
            break;
        case IS2_HALF_IPV4_PTP_MSG_TYPE:
            offset[0] = 166;
            offset[1] = 4;
            break;
        case IS2_HALF_IPV4_TWO_STEP:
            offset[0] = 170;
            offset[1] = 1;
            break;
        case IS2_FULL_TYPE:
            offset[0] = 0;
            offset[1] = 2;
            break;
        case IS2_FULL_FIRST_LOOKUP:
            offset[0] = 2;
            offset[1] = 1;
            break;
        case IS2_FULL_INGR_PORT_MASK:
            offset[0] = 11;
            offset[1] = 12;
            break;
        case IS2_FULL_IPV6_UDP_TYPE:
            offset[0] = 310;
            offset[1] = 1;
            break;
        case IS2_FULL_IPV6_DPORT:
            offset[0] = 311;
            offset[1] = 16;
            break;
        case IS2_FULL_IPV6_PTP_MSG_TYPE:
            offset[0] = 352;
            offset[1] = 4;
            break;
        case IS2_FULL_IPV6_TWO_STEP:
            offset[0] = 356;
            offset[1] = 1;
            break;
        default:
            break;
    }
}

static void h2tcam_is2_get_def_conf(tcam_e2e_tc_t ace_type,h2ace_is2_t *ace_conf)
{
    switch(ace_type) {
        case TCAM_E2E_TC_PTP_ETH:
            ace_conf->common.common_half.half_type = S2_TYPE_MAC_ETYPE;
            ace_conf->common.common_half.is_first_lookup = 1;
            ace_conf->pkt_info.eth.etype = MAC_ETYPE_PTP;
            /* Match sync,delay request */
            ace_conf->pkt_info.eth.payload.ptp_payload.ptp_msg_type = 0;
            /* Match only one-step */
            ace_conf->pkt_info.eth.payload.ptp_payload.ptp_two_step = 0;
            break;
        case TCAM_E2E_TC_PTP_IP:
            ace_conf->common.common_half.half_type = S2_TYPE_IPV4_TCP_UDP;
            ace_conf->common.common_half.is_first_lookup = 1;
            ace_conf->pkt_info.ipv4.ipv4 = 1;
            ace_conf->pkt_info.ipv4.tcp = 0;
            ace_conf->pkt_info.ipv4.dport = PTP_UDP_PORT;
            /* Match sync,delay request */
            ace_conf->pkt_info.ipv4.payload.ptp_payload.ptp_msg_type = 0;
            /* Match only one-step */
            ace_conf->pkt_info.ipv4.payload.ptp_payload.ptp_two_step = 0;
            break;
        case TCAM_E2E_TC_PTP_IPV6:
            ace_conf->common.common_full.full_type = S2_TYPE_IPV6_TCP_UDP;
            ace_conf->common.common_full.is_first_lookup = 1;
            ace_conf->pkt_info.ipv6.tcp = 0;
            ace_conf->pkt_info.ipv6.dport = PTP_UDP_PORT;
            /* Match sync,delay request */
            ace_conf->pkt_info.ipv6.payload.ptp_payload.ptp_msg_type = 0;
            /* Match only one-step */
            ace_conf->pkt_info.ipv6.payload.ptp_payload.ptp_two_step = 0;
            break;
    }
}
static void h2tcam_is2_set_common_conf(h2ace_is2_t *ace_conf,tcam_data_conf_t *tcam_conf,tcam_e2e_tc_t ace_type)
{
    u32 value[2]={0,0};
    u16 offset_info[2]={0,0};

    if (ace_type == TCAM_E2E_TC_PTP_ETH | ace_type == TCAM_E2E_TC_PTP_IP) {
        get_offset_len_par(IS2_HALF_TYPE,offset_info);

        value[0]=(u32)(ace_conf->common.common_half.half_type);
        h2tcam_bit_feild_32bit_set(offset_info[0],offset_info[1],value,tcam_conf->tcam_entry);
        value[0] = (u32)VTSS_BITMASK(offset_info[1]);
        h2tcam_bit_feild_32bit_set(offset_info[0],offset_info[1],value,tcam_conf->tcam_mask);

        get_offset_len_par(IS2_HALF_FIRST_LOOKUP,offset_info);
        value[0]=(u32)(ace_conf->common.common_half.is_first_lookup);
        h2tcam_bit_feild_32bit_set(offset_info[0],offset_info[1],value,tcam_conf->tcam_entry);
        value[0] = 1;
        h2tcam_bit_feild_32bit_set(offset_info[0],offset_info[1],value,tcam_conf->tcam_mask);
    } else if (ace_type == TCAM_E2E_TC_PTP_IPV6) {
        get_offset_len_par(IS2_FULL_TYPE,offset_info);

        value[0]=(u32)(ace_conf->common.common_full.full_type);
        h2tcam_bit_feild_32bit_set(offset_info[0],offset_info[1],value,tcam_conf->tcam_entry);
        value[0] = (u32)VTSS_BITMASK(offset_info[1]);
        //value[0] = 0;
        h2tcam_bit_feild_32bit_set(offset_info[0],offset_info[1],value,tcam_conf->tcam_mask);

        get_offset_len_par(IS2_FULL_FIRST_LOOKUP,offset_info);
        value[0]=(u32)(ace_conf->common.common_full.is_first_lookup);
        h2tcam_bit_feild_32bit_set(offset_info[0],offset_info[1],value,tcam_conf->tcam_entry);
        value[0] = 1;
        h2tcam_bit_feild_32bit_set(offset_info[0],offset_info[1],value,tcam_conf->tcam_mask);
    }
}
static void h2tcam_is2_set_ipv6_conf(h2ace_is2_t *ace_conf,tcam_data_conf_t *tcam_conf)
{
    u32 value[2]={0,0};
    u16 offset_info[2]={0,0};

    get_offset_len_par(IS2_FULL_IPV6_UDP_TYPE,offset_info);
    value[0]=(u32)(ace_conf->pkt_info.ipv6.tcp);
    h2tcam_bit_feild_32bit_set(offset_info[0],offset_info[1],value,&tcam_conf->tcam_entry);
    value[0] = 1;
    h2tcam_bit_feild_32bit_set(offset_info[0],offset_info[1],value,&tcam_conf->tcam_mask);

    get_offset_len_par(IS2_FULL_IPV6_DPORT,offset_info);
    value[0]=(u32)(ace_conf->pkt_info.ipv6.dport);
    h2tcam_bit_feild_32bit_set(offset_info[0],offset_info[1],value,&tcam_conf->tcam_entry);
    value[0] = VTSS_BITMASK(offset_info[1]);
    h2tcam_bit_feild_32bit_set(offset_info[0],offset_info[1],value,&tcam_conf->tcam_mask);

    get_offset_len_par(IS2_FULL_IPV6_PTP_MSG_TYPE,offset_info);
    value[0]=(u32)(ace_conf->pkt_info.ipv6.payload.ptp_payload.ptp_msg_type);
    h2tcam_bit_feild_32bit_set(offset_info[0],offset_info[1],value,&tcam_conf->tcam_entry);
    value[0] = 2;
    h2tcam_bit_feild_32bit_set(offset_info[0],offset_info[1],value,&tcam_conf->tcam_mask);

    get_offset_len_par(IS2_FULL_IPV6_TWO_STEP,offset_info);
    value[0]=(u32)(ace_conf->pkt_info.ipv6.payload.ptp_payload.ptp_two_step);
    h2tcam_bit_feild_32bit_set(offset_info[0],offset_info[1],value,&tcam_conf->tcam_entry);
    value[0] = 1;
    h2tcam_bit_feild_32bit_set(offset_info[0],offset_info[1],value,&tcam_conf->tcam_mask);
}
static void h2tcam_is2_set_ipv4_conf(h2ace_is2_t *ace_conf,tcam_data_conf_t *tcam_conf)
{
    u32 value[2];
    u16 offset_info[2];

    get_offset_len_par(IS2_HALF_IPV4_IS_IPV4,offset_info);
    value[0]=(u32)(ace_conf->pkt_info.ipv4.ipv4);
    h2tcam_bit_feild_32bit_set(offset_info[0],offset_info[1],value,tcam_conf->tcam_entry);
    value[0] = 1;
    h2tcam_bit_feild_32bit_set(offset_info[0],offset_info[1],value,tcam_conf->tcam_mask);

    get_offset_len_par(IS2_HALF_IPV4_UDP_TYPE,offset_info);
    value[0]=(u32)(ace_conf->pkt_info.ipv4.tcp);
    h2tcam_bit_feild_32bit_set(offset_info[0],offset_info[1],value,tcam_conf->tcam_entry);
    value[0] = 1;
    h2tcam_bit_feild_32bit_set(offset_info[0],offset_info[1],value,tcam_conf->tcam_mask);

    get_offset_len_par(IS2_HALF_IPV4_DPORT,offset_info);
    value[0]=(u32)(ace_conf->pkt_info.ipv4.dport);
    h2tcam_bit_feild_32bit_set(offset_info[0],offset_info[1],value,tcam_conf->tcam_entry);
    value[0] = VTSS_BITMASK(offset_info[1]);
    h2tcam_bit_feild_32bit_set(offset_info[0],offset_info[1],value,tcam_conf->tcam_mask);

    get_offset_len_par(IS2_HALF_IPV4_PTP_MSG_TYPE,offset_info);
    value[0]=(u32)(ace_conf->pkt_info.ipv4.payload.ptp_payload.ptp_msg_type);
    h2tcam_bit_feild_32bit_set(offset_info[0],offset_info[1],value,tcam_conf->tcam_entry);
    value[0] = 2;
    h2tcam_bit_feild_32bit_set(offset_info[0],offset_info[1],value,tcam_conf->tcam_mask);

    get_offset_len_par(IS2_HALF_IPV4_TWO_STEP,offset_info);
    value[0]=(u32)(ace_conf->pkt_info.ipv4.payload.ptp_payload.ptp_two_step);
    h2tcam_bit_feild_32bit_set(offset_info[0],offset_info[1],value,tcam_conf->tcam_entry);
    value[0] = 1;
    h2tcam_bit_feild_32bit_set(offset_info[0],offset_info[1],value,tcam_conf->tcam_mask);
}
static void h2tcam_is2_set_eth_conf(h2ace_is2_t *ace_conf,tcam_data_conf_t *tcam_conf)
{
    u32 value[2]={0,0};
    u16 offset_info[2]={0,0};

    get_offset_len_par(IS2_HALF_ETHERNET_PROTO,offset_info);

    value[0]=(u32)(ace_conf->pkt_info.eth.etype);
    h2tcam_bit_feild_32bit_set(offset_info[0],offset_info[1],value,tcam_conf->tcam_entry);
    value[0] = VTSS_BITMASK(offset_info[1]);
    h2tcam_bit_feild_32bit_set(offset_info[0],offset_info[1],value,tcam_conf->tcam_mask);
    
    get_offset_len_par(IS2_HALF_ETH_PTP_MSG_TYPE,offset_info);
    value[0]=(u32)(ace_conf->pkt_info.eth.payload.ptp_payload.ptp_msg_type);
    h2tcam_bit_feild_32bit_set(offset_info[0],offset_info[1],value,tcam_conf->tcam_entry);
    value[0] = 2;
    h2tcam_bit_feild_32bit_set(offset_info[0],offset_info[1],value,tcam_conf->tcam_mask);
    
    get_offset_len_par(IS2_HALF_ETH_TWO_STEP,offset_info);
    value[0]=(u32)(ace_conf->pkt_info.eth.payload.ptp_payload.ptp_two_step);
    h2tcam_bit_feild_32bit_set(offset_info[0],offset_info[1],value,tcam_conf->tcam_entry);
    value[0] = 0x1;
    h2tcam_bit_feild_32bit_set(offset_info[0],offset_info[1],value,tcam_conf->tcam_mask);
}
static void h2tcam_add_is2_ptp_ethernet(u16 entry_id,tcam_e2e_tc_t ace_type) 
{
    tcam_data_conf_t tcam_conf;
    h2ace_is2_t ace_conf;
    u32 value[2];

    memset(&tcam_conf,0,sizeof(tcam_data_conf_t));

    h2tcam_is2_get_def_conf(ace_type,&ace_conf);

    h2tcam_is2_set_common_conf(&ace_conf,&tcam_conf,ace_type);
    h2tcam_is2_set_eth_conf(&ace_conf,&tcam_conf);
    
    ////ACTION entries
    value[0] = 0x2;
    h2tcam_bit_feild_32bit_set(32,9,value,tcam_conf.tcam_action);

    // Type group MAC_ETYPE TG = 2= 1010 = 0xa
    tcam_conf.tg = 0xa;
    tcam_conf.tcam_count = 0;
    tcam_conf.entry_length = 5;
    tcam_conf.action_length = 1;
    h2tcam_action_cache_to_tcam(entry_id,&tcam_conf);
    h2tcam_entry_cache_to_tcam(entry_id,&tcam_conf);
}

static void h2tcam_entry_cache_to_tcam(u16 entry_id,const tcam_data_conf_t *conf) 
{
    u8 cntr;

    // Type group 
    H2_WRITE_MASKED(VTSS_VCAP_CORE_VCAP_CORE_CACHE_VCAP_TG_DAT(VTSS_TO_VCAP_IS2),
            conf->tg,
            0xff);
    //Cache entry and mask
    for (cntr=0;cntr<=conf->entry_length;cntr++) {
        H2_WRITE_MASKED(VTSS_VCAP_CORE_VCAP_CORE_CACHE_VCAP_ENTRY_DAT(VTSS_TO_VCAP_IS2,cntr),
                VTSS_F_VCAP_CORE_VCAP_CORE_CACHE_VCAP_ENTRY_DAT_ENTRY_DAT(conf->tcam_entry[cntr]),
                VTSS_M_VCAP_CORE_VCAP_CORE_CACHE_VCAP_ENTRY_DAT_ENTRY_DAT);
        H2_WRITE_MASKED(VTSS_VCAP_CORE_VCAP_CORE_CACHE_VCAP_MASK_DAT(VTSS_TO_VCAP_IS2,cntr),
                VTSS_F_VCAP_CORE_VCAP_CORE_CACHE_VCAP_MASK_DAT_MASK_DAT(~(conf->tcam_mask[cntr])),
                VTSS_M_VCAP_CORE_VCAP_CORE_CACHE_VCAP_MASK_DAT_MASK_DAT);
    }
    H2_WRITE_MASKED(VTSS_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL(VTSS_TO_VCAP_IS2),
            VTSS_F_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL_UPDATE_CMD(VCAP_CMD_WRITE) |
            VTSS_F_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL_UPDATE_ENTRY_DIS(0) |
            VTSS_F_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL_UPDATE_ADDR(entry_id) |
            VTSS_F_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL_UPDATE_SHOT(1),
            VTSS_M_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL_UPDATE_CMD |
            VTSS_M_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL_UPDATE_ENTRY_DIS |
            VTSS_M_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL_UPDATE_ADDR |
            VTSS_M_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL_UPDATE_SHOT );

    H2_WRITE_MASKED(VTSS_VCAP_CORE_VCAP_CORE_CACHE_VCAP_CNT_DAT(VTSS_TO_VCAP_IS2,0),
            VTSS_F_VCAP_CORE_VCAP_CORE_CACHE_VCAP_CNT_DAT_CNT_DAT(conf->tcam_count),
            VTSS_M_VCAP_CORE_VCAP_CORE_CACHE_VCAP_CNT_DAT_CNT_DAT);;
}

static void h2tcam_action_cache_to_tcam(u16 entry_id,const tcam_data_conf_t *conf) 
{
    u8 cntr;
    H2_WRITE_MASKED(VTSS_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL(VTSS_TO_VCAP_IS2),
            VTSS_F_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL_UPDATE_CMD(VCAP_CMD_WRITE) |
            VTSS_F_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL_UPDATE_ENTRY_DIS(0) |
            VTSS_F_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL_UPDATE_ADDR(entry_id) |
            VTSS_F_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL_UPDATE_SHOT(1),
            VTSS_M_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL_UPDATE_CMD |
            VTSS_M_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL_UPDATE_ENTRY_DIS |
            VTSS_M_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL_UPDATE_ADDR |
            VTSS_M_VCAP_CORE_VCAP_CORE_CFG_VCAP_UPDATE_CTRL_UPDATE_SHOT );
    //ACTION entries
    for (cntr=0;cntr<=conf->action_length;cntr++) {
        H2_WRITE_MASKED(VTSS_VCAP_CORE_VCAP_CORE_CACHE_VCAP_ACTION_DAT(VTSS_TO_VCAP_IS2,cntr),
                VTSS_F_VCAP_CORE_VCAP_CORE_CACHE_VCAP_ACTION_DAT_ACTION_DAT(conf->tcam_action[cntr]),
                VTSS_M_VCAP_CORE_VCAP_CORE_CACHE_VCAP_ACTION_DAT_ACTION_DAT);
    }

}


static void h2tcam_bit_feild_32bit_set(u16 offset,u16 length,u32 *value,u32 *conf) 
{
    u16 arr_sub=0,actual_off=0,lbits=0,rbits=0;

    arr_sub = offset/32;
    actual_off = offset%32;
    // lbits is least significant bits that are in curret word 
    lbits = (arr_sub + 1) * 32 - offset;
    if (length <=32 ) {
        /* Case where Bitfeild is less than or equal to 4 bytes */
        if(is_in_1_word(offset,length)) {
            /* Case where bit feild is in single entry */
            conf[arr_sub] |= (value[0] << (offset - (arr_sub * 32)));
        } else {
            /* Case where bit feild is spreaded over two entries */
            conf[arr_sub] |= ((value[0] & VTSS_BITMASK(lbits)) << (offset - (arr_sub * 32)));

            // rbits is most significant bits excluding lbits
            rbits = length - lbits;
            value[0] >>= lbits;
            h2tcam_bit_feild_32bit_set((offset+lbits),rbits,value,conf);
        }
    } else {
        /* TBD for more than 32-bit sized bit feilds */

    }
}
static u8 is_in_1_word(u8 bf_lsb, u8 length)
{
    u8 msb,lsb,bf_msb,cntr=0;

    bf_msb = bf_lsb + length - 1;
    for (cntr = 1;cntr<=12;cntr++) {
        msb = cntr * 32 - 1;
        lsb = cntr * 32 - 32;
        if ((bf_msb <= msb) && (bf_lsb >= lsb)) {
            return TRUE;
        }
    }
    return FALSE;
}
#endif
