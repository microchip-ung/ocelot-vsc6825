// This file was created by the txt2c.pl utility based on file l26_txtdef.txt
//


#include "common.h"
#include "swconf.h"
#include "l26_txtdef.h"


/* ************************************************************************ **
 *
 *
 * cmd texts
 *
 *
 *
 * ************************************************************************ */
#if TRANSIT_UNMANAGED_SYS_MAC_CONF
const char txt_CMD_TXT_NO_CONFIG [] = {"CONFIG"};

const char txt_CMD_TXT_NO_MAC [] = {"MAC"};

const char txt_CMD_TXT_NO_SAVE [] = {"SAVE"};

#endif
const char txt_CMD_TXT_NO_END [] = {"END"};



const struct {
    char *str_ptr;
    uchar min_match;  /* Minimum number of characters that must be present */
} cmd_txt_tab [END_CMD_TXT] = {
#if TRANSIT_UNMANAGED_SYS_MAC_CONF
    txt_CMD_TXT_NO_CONFIG,                     6,
    txt_CMD_TXT_NO_MAC,                        3,
    txt_CMD_TXT_NO_SAVE,                       4,
#endif
    txt_CMD_TXT_NO_END,                        3,
};



/* ************************************************************************ **
 *
 *
 * std texts
 *
 *
 *
 * ************************************************************************ */
#if TRANSIT_LLDP
const char txt_TXT_NO_CHIP_NAME [] = {"SparX-III"};

#if defined(LUTON26_L25)
const char txt_TXT_NO_SWITCH_DESCRIPTION [] = {" - 25 Port Gigabit Ethernet Switch"};
#elif defined(LUTON26_L16)
const char txt_TXT_NO_SWITCH_DESCRIPTION [] = {" - 16 Port Gigabit Ethernet Switch"};
#elif defined(LUTON26_L10)
const char txt_TXT_NO_SWITCH_DESCRIPTION [] = {" - 10 Port Gigabit Ethernet Switch"};
#elif defined(FERRET_F11)
const char txt_TXT_NO_SWITCH_DESCRIPTION [] = {" - 11 Port Gigabit Ethernet Switch"};
#elif defined(FERRET_F10P)
const char txt_TXT_NO_SWITCH_DESCRIPTION [] = {" - 10 Port Gigabit Ethernet Switch"};
#elif defined(FERRET_F5)
const char txt_TXT_NO_SWITCH_DESCRIPTION [] = {" - 5 Port Gigabit Ethernet Switch"};
#elif defined(FERRET_F4P)
const char txt_TXT_NO_SWITCH_DESCRIPTION [] = {" - 4 Port Gigabit Ethernet Switch"};
#else
const char txt_TXT_NO_SWITCH_DESCRIPTION [] = {" - Unknown switch description"};
#endif

#if defined(LUTON26_L25)
const char txt_TXT_NO_SWITCH_NAME [] = {"VSC7422 25 Port Switch"};
#elif defined(LUTON26_L16)
const char txt_TXT_NO_SWITCH_NAME [] = {"VSC7421 16 Port Switch"};
#elif defined(LUTON26_L10)
const char txt_TXT_NO_SWITCH_NAME [] = {"VSC7420 10 Port Switch"};
#elif defined(FERRET_F11)
const char txt_TXT_NO_SWITCH_NAME [] = {"VSC7412 11 Port Switch"};
#elif defined(FERRET_F10P)
const char txt_TXT_NO_SWITCH_NAME [] = {"VSC7412 10 Port Switch"};
#elif defined(FERRET_F5)
const char txt_TXT_NO_SWITCH_NAME [] = {"VSC7411 5 Port Switch"};
#elif defined(FERRET_F4P)
const char txt_TXT_NO_SWITCH_NAME [] = {"VSC7411 4 Port Switch"};
#else
const char txt_TXT_NO_SWITCH_NAME [] = {"Unknown switch name"};
#endif

#if UNMANAGED_LLDP_DEBUG_IF
#if 0
const char txt_TXT_NO_LLDP_OPTIONAL_TLVS [] = {"Optional TLVs"};

#endif
const char txt_TXT_NO_LLDP_REMOTEMIB_HDR [] = {"Port    Chassis ID                Port ID                System Name\r\n"};

const char txt_TXT_NO_LLDP_CHASSIS_ID [] = {"Chassis ID:"};

const char txt_TXT_NO_LLDP_CHASSIS_TYPE [] = {"Chassis Type:"};

const char txt_TXT_NO_LLDP_PORT_TYPE [] = {"Port Type:"};

const char txt_TXT_NO_LLDP_PORT_ID [] = {"Port ID"};

const char txt_TXT_NO_LLDP_SYSTEM_NAME [] = {"System Name:"};

const char txt_TXT_NO_LLDP_SYSTEM_DESCR [] = {"System Description:"};

const char txt_TXT_NO_LLDP_PORT_DESCR [] = {"Port Description:"};

const char txt_TXT_NO_LLDP_SYSTEM_CAPA [] = {"System Capabilities:"};

const char txt_TXT_NO_LLDP_MGMT_ADDR [] = {"Management Address:"};

#if 0
const char txt_TXT_NO_LLDP_STATHDR1 [] = {"         Tx           Recieved Frames                   TLVs"};

const char txt_TXT_NO_LLDP_STATHDR2 [] = {"Port   Frames   Total   Errors   Discards   Discards   Unrecog.  Org.    Ageouts"};

#endif
#if UNMANAGED_EEE_DEBUG_IF
const char txt_TXT_NO_EEE_STAT [] = {"    Tx_tw   Rx_tw   Fb Rx_tw   Echo Tx_tw   Echo Rx_tw   Resolved Tx   Resolved Rx\r\n"};

#endif
const char txt_TXT_NO_LLDP_NO_ENTRIES [] = {"No entries found"};

const char txt_TXT_NO_LLDP_LOCAL_PORT [] = {"Local port"};

#endif

#if TRANSIT_VERIPHY
#if UNMANAGED_TRANSIT_VERIPHY_DEBUG_IF
const char txt_TXT_NO_VERIPHY_OK [] = {"OK   "};

const char txt_TXT_NO_VERIPHY_OPEN [] = {"Open "};

const char txt_TXT_NO_VERIPHY_SHORT [] = {"Short"};

const char txt_TXT_NO_VERIPHY_ABNORMAL [] = {"Abnrm"};

const char txt_TXT_NO_VERIPHY_XA [] = {"XA   "};

const char txt_TXT_NO_VERIPHY_XB [] = {"XB   "};

const char txt_TXT_NO_VERIPHY_XC [] = {"XC   "};

const char txt_TXT_NO_VERIPHY_XD [] = {"XD   "};

const char txt_TXT_NO_VERIPHY_XCPLA [] = {"XCplA"};

const char txt_TXT_NO_VERIPHY_XCPLB [] = {"XCplB"};

const char txt_TXT_NO_VERIPHY_XCPLC [] = {"XCplC"};

const char txt_TXT_NO_VERIPHY_XCPLD [] = {"XCplD"};

const char txt_TXT_NO_VERIPHY_FAULT [] = {"Fault"};

const char txt_TXT_NO_VERIPHY_STAT_HDR [] = {"Port  Valid  LenA  LenB  LenC  LenD  StatA  StatB  StatC  StatD\r\n"};

const char txt_TXT_NO_VERIPHY_PROPER [] = {"Proper"};

const char txt_TXT_NO_VERIPHY_ABNORMAL_WEB [] = {"Abnormal termination"};

const char txt_TXT_NO_VERIPHY_XPAIR_WEB [] = {"Cross-pair short to pair "};

const char txt_TXT_NO_VERIPHY_XPAIR_CPL_WEB [] = {"Abnormal cross-pair coupling with pair "};

#endif
#endif
const char txt_TXT_NO_WRONG_CHIP_ID [] = {"Wrong Chip ID"};

#if UNMANAGED_PORT_STATISTICS_IF
const char txt_TXT_NO_RX_PREFIX [] = {"Rx "};

const char txt_TXT_NO_TX_PREFIX [] = {"Tx "};

const char txt_TXT_NO_PACKETS [] = {"Packets:"};

const char txt_TXT_NO_OCTETS [] = {"Octets:"};

const char txt_TXT_NO_HI_PACKETS [] = {"High Priority Packets:"};

const char txt_TXT_NO_LO_PACKETS [] = {"Low Priority Packets:"};

const char txt_TXT_NO_BC_PACKETS [] = {"Broadcast:"};

const char txt_TXT_NO_MC_PACKETS [] = {"Multicast:"};

const char txt_TXT_NO_64_BYTES [] = {"64 Bytes:"};

const char txt_TXT_NO_65_BYTES [] = {"65-127 Bytes:"};

const char txt_TXT_NO_128_BYTES [] = {"128-255 Bytes:"};

const char txt_TXT_NO_256_BYTES [] = {"256-511 Bytes:"};

const char txt_TXT_NO_512_BYTES [] = {"512-1023 Bytes:"};

const char txt_TXT_NO_1024_BYTES [] = {"1024- Bytes:"};

const char txt_TXT_NO_CRC_ALIGN [] = {"CRC/Alignment:"};

const char txt_TXT_NO_COLLISIONS [] = {"Collisions:"};

const char txt_TXT_NO_UNDERSIZE [] = {"Undersize:"};

const char txt_TXT_NO_DROPS [] = {"Drops:"};

const char txt_TXT_NO_OVERSIZE [] = {"Oversize:"};

const char txt_TXT_NO_FRAGMENTS [] = {"Fragments:"};

const char txt_TXT_NO_JABBERS [] = {"Jabbers:"};

const char txt_TXT_NO_CAT_DROPS [] = {"Classifier Drops:"};

const char txt_TXT_NO_RX_TOTAL [] = {"Receive Total"};

const char txt_TXT_NO_TX_TOTAL [] = {"Transmit Total"};

const char txt_TXT_NO_RX_SIZE [] = {"Receive Size Counters"};

const char txt_TXT_NO_TX_SIZE [] = {"Transmit Size Counters"};

const char txt_TXT_NO_RX_ERROR [] = {"Receive Error Counters"};

const char txt_TXT_NO_TX_ERROR [] = {"Transmit Error Counters"};

const char txt_TXT_NO_BM_PACKETS [] = {"Broad- and Multicast"};

const char txt_TXT_NO_ERR_PACKETS [] = {"Error Packets"};

const char txt_TXT_NO_OVERFLOW [] = {"Overflow:"};

const char txt_TXT_NO_AGED [] = {"Aged:"};

const char txt_TXT_NO_PAUSE [] = {"Pause:"};

const char txt_TXT_NO_MAC_CTRL [] = {"MAC Ctrl:"};

const char txt_TXT_NO_DASH [] = {"-"};

#endif
const char txt_TXT_NO_TEMPERATURE [] = {"Temperature"};

const char txt_TXT_NO_COMPILE_DATE [] = {"Compile Date: "};



const struct {
    char *str_ptr;
} std_txt_tab [END_STD_TXT] = {
#if TRANSIT_LLDP
    txt_TXT_NO_CHIP_NAME,
#if defined(VTSS_ARCH_LUTON26)
    txt_TXT_NO_SWITCH_DESCRIPTION,
    txt_TXT_NO_SWITCH_NAME,
#endif
#if UNMANAGED_LLDP_DEBUG_IF
#if 0
    txt_TXT_NO_LLDP_OPTIONAL_TLVS,
#endif
    txt_TXT_NO_LLDP_REMOTEMIB_HDR,
    txt_TXT_NO_LLDP_CHASSIS_ID,
    txt_TXT_NO_LLDP_CHASSIS_TYPE,
    txt_TXT_NO_LLDP_PORT_TYPE,
    txt_TXT_NO_LLDP_PORT_ID,
    txt_TXT_NO_LLDP_SYSTEM_NAME,
    txt_TXT_NO_LLDP_SYSTEM_DESCR,
    txt_TXT_NO_LLDP_PORT_DESCR,
    txt_TXT_NO_LLDP_SYSTEM_CAPA,
    txt_TXT_NO_LLDP_MGMT_ADDR,
#if 0
    txt_TXT_NO_LLDP_STATHDR1,
    txt_TXT_NO_LLDP_STATHDR2,
#endif
#if UNMANAGED_EEE_DEBUG_IF
    txt_TXT_NO_EEE_STAT,
#endif
    txt_TXT_NO_LLDP_NO_ENTRIES,
    txt_TXT_NO_LLDP_LOCAL_PORT,
#endif
#endif /* TRANSIT_LLDP */

#if TRANSIT_VERIPHY
#if UNMANAGED_TRANSIT_VERIPHY_DEBUG_IF
    txt_TXT_NO_VERIPHY_OK,
    txt_TXT_NO_VERIPHY_OPEN,
    txt_TXT_NO_VERIPHY_SHORT,
    txt_TXT_NO_VERIPHY_ABNORMAL,
    txt_TXT_NO_VERIPHY_XA,
    txt_TXT_NO_VERIPHY_XB,
    txt_TXT_NO_VERIPHY_XC,
    txt_TXT_NO_VERIPHY_XD,
    txt_TXT_NO_VERIPHY_XCPLA,
    txt_TXT_NO_VERIPHY_XCPLB,
    txt_TXT_NO_VERIPHY_XCPLC,
    txt_TXT_NO_VERIPHY_XCPLD,
    txt_TXT_NO_VERIPHY_FAULT,
    txt_TXT_NO_VERIPHY_STAT_HDR,
    txt_TXT_NO_VERIPHY_PROPER,
    txt_TXT_NO_VERIPHY_ABNORMAL_WEB,
    txt_TXT_NO_VERIPHY_XPAIR_WEB,
    txt_TXT_NO_VERIPHY_XPAIR_CPL_WEB,
#endif
#endif
    txt_TXT_NO_WRONG_CHIP_ID,
#if UNMANAGED_PORT_STATISTICS_IF
    txt_TXT_NO_RX_PREFIX,
    txt_TXT_NO_TX_PREFIX,
    txt_TXT_NO_PACKETS,
    txt_TXT_NO_OCTETS,
    txt_TXT_NO_HI_PACKETS,
    txt_TXT_NO_LO_PACKETS,
    txt_TXT_NO_BC_PACKETS,
    txt_TXT_NO_MC_PACKETS,
    txt_TXT_NO_64_BYTES,
    txt_TXT_NO_65_BYTES,
    txt_TXT_NO_128_BYTES,
    txt_TXT_NO_256_BYTES,
    txt_TXT_NO_512_BYTES,
    txt_TXT_NO_1024_BYTES,
    txt_TXT_NO_CRC_ALIGN,
    txt_TXT_NO_COLLISIONS,
    txt_TXT_NO_UNDERSIZE,
    txt_TXT_NO_DROPS,
    txt_TXT_NO_OVERSIZE,
    txt_TXT_NO_FRAGMENTS,
    txt_TXT_NO_JABBERS,
    txt_TXT_NO_CAT_DROPS,
    txt_TXT_NO_RX_TOTAL,
    txt_TXT_NO_TX_TOTAL,
    txt_TXT_NO_RX_SIZE,
    txt_TXT_NO_TX_SIZE,
    txt_TXT_NO_RX_ERROR,
    txt_TXT_NO_TX_ERROR,
    txt_TXT_NO_BM_PACKETS,
    txt_TXT_NO_ERR_PACKETS,
    txt_TXT_NO_OVERFLOW,
    txt_TXT_NO_AGED,
    txt_TXT_NO_PAUSE,
    txt_TXT_NO_MAC_CTRL,
    txt_TXT_NO_DASH,
#endif
    txt_TXT_NO_TEMPERATURE,
    txt_TXT_NO_COMPILE_DATE,
};
