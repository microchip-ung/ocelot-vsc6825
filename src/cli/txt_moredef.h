//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#ifndef TXT_MOREDEF_H
#define TXT_MOREDEF_H

/* ************************************************************************ **
 *
 *
 * cmd texts
 *
 *
 *
 * ************************************************************************ */
#if TRANSIT_UNMANAGED_SYS_MAC_CONF
extern const char txt_CMD_TXT_NO_CONFIG [];
extern const char txt_CMD_TXT_NO_MAC [];
extern const char txt_CMD_TXT_NO_SAVE [];
#endif
extern const char txt_CMD_TXT_NO_END [];


typedef enum {
#if TRANSIT_UNMANAGED_SYS_MAC_CONF
    CMD_TXT_NO_CONFIG,
    CMD_TXT_NO_MAC,
    CMD_TXT_NO_SAVE,
#endif
    CMD_TXT_NO_END,
    END_CMD_TXT,
} cmd_txt_t;




extern const struct {
    char *str_ptr;
    uchar min_match;  /* Minimum number of characters that must be present */
} cmd_txt_tab [END_CMD_TXT];



/* ************************************************************************ **
 *
 *
 * std texts
 *
 *
 *
 * ************************************************************************ */
#if TRANSIT_LLDP
extern const char txt_TXT_NO_CHIP_NAME [];
extern const char txt_TXT_NO_SWITCH_DESCRIPTION [];
extern const char txt_TXT_NO_SWITCH_NAME [];

#if UNMANAGED_LLDP_DEBUG_IF
#if 0
extern const char txt_TXT_NO_LLDP_OPTIONAL_TLVS [];
#endif
extern const char txt_TXT_NO_LLDP_REMOTEMIB_HDR [];
extern const char txt_TXT_NO_LLDP_CHASSIS_ID [];
extern const char txt_TXT_NO_LLDP_CHASSIS_TYPE [];
extern const char txt_TXT_NO_LLDP_PORT_TYPE [];
extern const char txt_TXT_NO_LLDP_PORT_ID [];
extern const char txt_TXT_NO_LLDP_SYSTEM_NAME [];
extern const char txt_TXT_NO_LLDP_SYSTEM_DESCR [];
extern const char txt_TXT_NO_LLDP_PORT_DESCR [];
extern const char txt_TXT_NO_LLDP_SYSTEM_CAPA [];
extern const char txt_TXT_NO_LLDP_MGMT_ADDR [];
#if 0
extern const char txt_TXT_NO_LLDP_STATHDR1 [];
extern const char txt_TXT_NO_LLDP_STATHDR2 [];
#endif
#if UNMANAGED_EEE_DEBUG_IF
extern const char txt_TXT_NO_EEE_STAT [];
#endif
extern const char txt_TXT_NO_LLDP_NO_ENTRIES [];
extern const char txt_TXT_NO_LLDP_LOCAL_PORT [];
#endif
#endif /* TRANSIT_LLDP */
#if TRANSIT_VERIPHY
#if UNMANAGED_TRANSIT_VERIPHY_DEBUG_IF
extern const char txt_TXT_NO_VERIPHY_OK [];
extern const char txt_TXT_NO_VERIPHY_OPEN [];
extern const char txt_TXT_NO_VERIPHY_SHORT [];
extern const char txt_TXT_NO_VERIPHY_ABNORMAL [];
extern const char txt_TXT_NO_VERIPHY_XA [];
extern const char txt_TXT_NO_VERIPHY_XB [];
extern const char txt_TXT_NO_VERIPHY_XC [];
extern const char txt_TXT_NO_VERIPHY_XD [];
extern const char txt_TXT_NO_VERIPHY_XCPLA [];
extern const char txt_TXT_NO_VERIPHY_XCPLB [];
extern const char txt_TXT_NO_VERIPHY_XCPLC [];
extern const char txt_TXT_NO_VERIPHY_XCPLD [];
extern const char txt_TXT_NO_VERIPHY_FAULT [];
extern const char txt_TXT_NO_VERIPHY_STAT_HDR [];
extern const char txt_TXT_NO_VERIPHY_PROPER [];
extern const char txt_TXT_NO_VERIPHY_ABNORMAL_WEB [];
extern const char txt_TXT_NO_VERIPHY_XPAIR_WEB [];
extern const char txt_TXT_NO_VERIPHY_XPAIR_CPL_WEB [];
#endif
#endif
extern const char txt_TXT_NO_WRONG_CHIP_ID [];
#if UNMANAGED_PORT_STATISTICS_IF
extern const char txt_TXT_NO_RX_PREFIX [];
extern const char txt_TXT_NO_TX_PREFIX [];
extern const char txt_TXT_NO_PACKETS [];
extern const char txt_TXT_NO_OCTETS [];
extern const char txt_TXT_NO_HI_PACKETS [];
extern const char txt_TXT_NO_LO_PACKETS [];
extern const char txt_TXT_NO_BC_PACKETS [];
extern const char txt_TXT_NO_MC_PACKETS [];
extern const char txt_TXT_NO_64_BYTES [];
extern const char txt_TXT_NO_65_BYTES [];
extern const char txt_TXT_NO_128_BYTES [];
extern const char txt_TXT_NO_256_BYTES [];
extern const char txt_TXT_NO_512_BYTES [];
extern const char txt_TXT_NO_1024_BYTES [];
extern const char txt_TXT_NO_CRC_ALIGN [];
extern const char txt_TXT_NO_COLLISIONS [];
extern const char txt_TXT_NO_UNDERSIZE [];
extern const char txt_TXT_NO_DROPS [];
extern const char txt_TXT_NO_OVERSIZE [];
extern const char txt_TXT_NO_FRAGMENTS [];
extern const char txt_TXT_NO_JABBERS [];
extern const char txt_TXT_NO_CAT_DROPS [];
extern const char txt_TXT_NO_RX_TOTAL [];
extern const char txt_TXT_NO_TX_TOTAL [];
extern const char txt_TXT_NO_RX_SIZE [];
extern const char txt_TXT_NO_TX_SIZE [];
extern const char txt_TXT_NO_RX_ERROR [];
extern const char txt_TXT_NO_TX_ERROR [];
extern const char txt_TXT_NO_BM_PACKETS [];
extern const char txt_TXT_NO_ERR_PACKETS [];
extern const char txt_TXT_NO_OVERFLOW [];
extern const char txt_TXT_NO_AGED [];
extern const char txt_TXT_NO_PAUSE [];
extern const char txt_TXT_NO_MAC_CTRL [];
extern const char txt_TXT_NO_DASH [];

#if UNMANAGED_PORT_STATISTICS_QOS
extern const char txt_TXT_NO_RX_QOS[];
extern const char txt_TXT_NO_TX_QOS[];
extern const char txt_TXT_NO_RED_PRIO_0[];
extern const char txt_TXT_NO_RED_PRIO_1[];
extern const char txt_TXT_NO_RED_PRIO_2[];
extern const char txt_TXT_NO_RED_PRIO_3[];
extern const char txt_TXT_NO_RED_PRIO_4[];
extern const char txt_TXT_NO_RED_PRIO_5[];
extern const char txt_TXT_NO_RED_PRIO_6[];
extern const char txt_TXT_NO_RED_PRIO_7[];
extern const char txt_TXT_NO_YELLOW_PRIO_0[];
extern const char txt_TXT_NO_YELLOW_PRIO_1[];
extern const char txt_TXT_NO_YELLOW_PRIO_2[];
extern const char txt_TXT_NO_YELLOW_PRIO_3[];
extern const char txt_TXT_NO_YELLOW_PRIO_4[];
extern const char txt_TXT_NO_YELLOW_PRIO_5[];
extern const char txt_TXT_NO_YELLOW_PRIO_6[];
extern const char txt_TXT_NO_YELLOW_PRIO_7[];
extern const char txt_TXT_NO_GREEN_PRIO_0[];
extern const char txt_TXT_NO_GREEN_PRIO_1[];
extern const char txt_TXT_NO_GREEN_PRIO_2[];
extern const char txt_TXT_NO_GREEN_PRIO_3[];
extern const char txt_TXT_NO_GREEN_PRIO_4[];
extern const char txt_TXT_NO_GREEN_PRIO_5[];
extern const char txt_TXT_NO_GREEN_PRIO_6[];
extern const char txt_TXT_NO_GREEN_PRIO_7[];
#endif // UNMANAGED_PORT_STATISTICS_QOS
#endif // UNMANAGED_PORT_STATISTICS_IF

extern const char txt_TXT_NO_TEMPERATURE [];
extern const char txt_TXT_NO_COMPILE_DATE [];


typedef enum {
#if TRANSIT_LLDP
    TXT_NO_CHIP_NAME,
#if UNMANAGED_LLDP_DEBUG_IF
#if 0
    TXT_NO_LLDP_OPTIONAL_TLVS,
#endif
    TXT_NO_LLDP_REMOTEMIB_HDR,
    TXT_NO_LLDP_CHASSIS_ID,
    TXT_NO_LLDP_CHASSIS_TYPE,
    TXT_NO_LLDP_PORT_TYPE,
    TXT_NO_LLDP_PORT_ID,
    TXT_NO_LLDP_SYSTEM_NAME,
    TXT_NO_LLDP_SYSTEM_DESCR,
    TXT_NO_LLDP_PORT_DESCR,
    TXT_NO_LLDP_SYSTEM_CAPA,
    TXT_NO_LLDP_MGMT_ADDR,
#if 0
    TXT_NO_LLDP_STATHDR1,
    TXT_NO_LLDP_STATHDR2,
#endif
#if UNMANAGED_EEE_DEBUG_IF
    TXT_NO_EEE_STAT,
#endif
    TXT_NO_LLDP_NO_ENTRIES,
    TXT_NO_LLDP_LOCAL_PORT,
#endif
#endif /* TRANSIT_LLDP */
#if TRANSIT_VERIPHY
#if UNMANAGED_TRANSIT_VERIPHY_DEBUG_IF
    TXT_NO_VERIPHY_OK,
    TXT_NO_VERIPHY_OPEN,
    TXT_NO_VERIPHY_SHORT,
    TXT_NO_VERIPHY_ABNORMAL,
    TXT_NO_VERIPHY_XA,
    TXT_NO_VERIPHY_XB,
    TXT_NO_VERIPHY_XC,
    TXT_NO_VERIPHY_XD,
    TXT_NO_VERIPHY_XCPLA,
    TXT_NO_VERIPHY_XCPLB,
    TXT_NO_VERIPHY_XCPLC,
    TXT_NO_VERIPHY_XCPLD,
    TXT_NO_VERIPHY_FAULT,
    TXT_NO_VERIPHY_STAT_HDR,
    TXT_NO_VERIPHY_PROPER,
    TXT_NO_VERIPHY_ABNORMAL_WEB,
    TXT_NO_VERIPHY_XPAIR_WEB,
    TXT_NO_VERIPHY_XPAIR_CPL_WEB,
#endif
#endif
    TXT_NO_WRONG_CHIP_ID,
#if UNMANAGED_PORT_STATISTICS_IF
    TXT_NO_RX_PREFIX,
    TXT_NO_TX_PREFIX,
    TXT_NO_PACKETS,
    TXT_NO_OCTETS,
    TXT_NO_HI_PACKETS,
    TXT_NO_LO_PACKETS,
    TXT_NO_BC_PACKETS,
    TXT_NO_MC_PACKETS,
    TXT_NO_64_BYTES,
    TXT_NO_65_BYTES,
    TXT_NO_128_BYTES,
    TXT_NO_256_BYTES,
    TXT_NO_512_BYTES,
    TXT_NO_1024_BYTES,
    TXT_NO_CRC_ALIGN,
    TXT_NO_COLLISIONS,
    TXT_NO_UNDERSIZE,
    TXT_NO_DROPS,
    TXT_NO_OVERSIZE,
    TXT_NO_FRAGMENTS,
    TXT_NO_JABBERS,
    TXT_NO_CAT_DROPS,
    TXT_NO_RX_TOTAL,
    TXT_NO_TX_TOTAL,
    TXT_NO_RX_SIZE,
    TXT_NO_TX_SIZE,
    TXT_NO_RX_ERROR,
    TXT_NO_TX_ERROR,
    TXT_NO_BM_PACKETS,
    TXT_NO_ERR_PACKETS,
    TXT_NO_OVERFLOW,
    TXT_NO_AGED,
    TXT_NO_PAUSE,
    TXT_NO_MAC_CTRL,
    TXT_NO_DASH,

#if UNMANAGED_PORT_STATISTICS_QOS
    TXT_NO_RX_QOS,
    TXT_NO_TX_QOS,
    TXT_NO_RED_PRIO_0,
    TXT_NO_RED_PRIO_1,
    TXT_NO_RED_PRIO_2,
    TXT_NO_RED_PRIO_3,
    TXT_NO_RED_PRIO_4,
    TXT_NO_RED_PRIO_5,
    TXT_NO_RED_PRIO_6,
    TXT_NO_RED_PRIO_7,
    TXT_NO_YELLOW_PRIO_0,
    TXT_NO_YELLOW_PRIO_1,
    TXT_NO_YELLOW_PRIO_2,
    TXT_NO_YELLOW_PRIO_3,
    TXT_NO_YELLOW_PRIO_4,
    TXT_NO_YELLOW_PRIO_5,
    TXT_NO_YELLOW_PRIO_6,
    TXT_NO_YELLOW_PRIO_7,
    TXT_NO_GREEN_PRIO_0,
    TXT_NO_GREEN_PRIO_1,
    TXT_NO_GREEN_PRIO_2,
    TXT_NO_GREEN_PRIO_3,
    TXT_NO_GREEN_PRIO_4,
    TXT_NO_GREEN_PRIO_5,
    TXT_NO_GREEN_PRIO_6,
    TXT_NO_GREEN_PRIO_7,
#endif // UNMANAGED_PORT_STATISTICS_QOS
#endif // UNMANAGED_PORT_STATISTICS_IF

    TXT_NO_TEMPERATURE,
    TXT_NO_COMPILE_DATE,

    /* Add new string definition above this line
     *
     * And notice that each field should have the same order sequence in
     * the string arrary td_txt_tab[END_STD_TXT] (txt_moredef.c)
     */
    END_STD_TXT
} std_txt_t;




extern const struct {
    char *str_ptr;
} std_txt_tab [END_STD_TXT];


#endif  // TXT_MOREDEF_H
