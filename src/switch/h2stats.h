//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#if !defined(__H2STATS_H__)
#define __H2STATS_H__

/****************************************************************************
 * Defines
 *
 *
 ************************************************************************** */

/****************************************************************************
 * Typedefs and enums
 *
 *
 ************************************************************************** */
#if defined(VTSS_ARCH_LUTON26)
typedef enum _port_statistics_t {
    /* TABLE Lookup counters */

    CNT_RX_OCTETS           =  0x0,
    CNT_RX_PKTS             =  0x1,
    CNT_RX_MCAST_PKTS       =  0x2,
    CNT_RX_BCAST_PKTS       =  0x3,
    CNT_RX_UNDERSIZE_PKTS   =  0x4,
    CNT_RX_FRAGMENTS        =  0x5,
    CNT_RX_JABBERS          =  0x6,
    CNT_RX_CRC_ALIGN_ERRS   =  0x7,
    CNT_RX_64               =  0x8,
    CNT_RX_65_TO_127        =  0x9,
    CNT_RX_128_TO_255       =  0xa,
    CNT_RX_256_TO_511       =  0xb,
    CNT_RX_512_TO_1023      =  0xc,
    CNT_RX_1024_TO_1526     =  0xd,
    CNT_RX_OVERSIZE_PKTS    =  0xe,
    CNT_RX_PAUSE            =  0xf,
    CNT_RX_CTRL             = 0x10,
    CNT_RX_LONG             = 0x11,
    CNT_RX_CAT_DROP         = 0x12,

    CNT_TX_OCTETS           = 0x800,
    CNT_TX_PKTS             = 0x801,
    CNT_TX_MCAST_PKTS       = 0x802,
    CNT_TX_BCAST_PKTS       = 0x803,
    CNT_TX_COLLISIONS       = 0x804,
    CNT_TX_DROP             = 0x805,
    CNT_TX_PAUSE            = 0x806,
    CNT_TX_64               = 0x807,
    CNT_TX_65_TO_127        = 0x808,
    CNT_TX_128_TO_255       = 0x809,
    CNT_TX_256_TO_511       = 0x80a,
    CNT_TX_512_TO_1023      = 0x80b,
    CNT_TX_1024_TO_1526     = 0x80c,
    CNT_TX_OVERSIZE_PKTS    = 0x80d,
    CNT_TX_AGED             = 0x81e,

    CNT_DROP_LOCAL          = 0xc00,
    CNT_DROP_TAIL           = 0xc01,

    /* Combined Counters */
    CNT_RX_UCAST_PKTS       = 0x7000,
    CNT_RX_NON_UCAST_PKTS   = 0x7001,
    CNT_RX_DISCARD_PKTS     = 0x7002,
    CNT_RX_ERR_PKTS         = 0x7003,
    CNT_RX_UNKNOWN_PROTOS   = 0x7004,
    CNT_RX_DROPEVENTS       = 0x7005,

    CNT_TX_UCAST_PKTS       = 0x7100,
    CNT_TX_NON_UCAST_PKTS   = 0x7101,
    CNT_TX_DISCARDS         = 0x7102,
    CNT_TX_ERR_PKTS         = 0x7103,


    COUNTER_NONE            = 0xffff,
} port_statistics_t;

#elif defined(VTSS_ARCH_OCELOT)
/* Refer to chip specification: Receive Counters in the Statistics Block */
typedef enum _port_statistics_t {
    /* TABLE Lookup counters */

    CNT_RX_OCTETS           = 0x0,
    CNT_RX_PKTS             = 0x1,
    CNT_RX_MCAST_PKTS       = 0x2,
    CNT_RX_BCAST_PKTS       = 0x3,
    CNT_RX_UNDERSIZE_PKTS   = 0x4,
    CNT_RX_FRAGMENTS        = 0x5,
    CNT_RX_JABBERS          = 0x6,
    CNT_RX_CRC_ALIGN_ERRS   = 0x7,
    CNT_RX_64               = 0x9,
    CNT_RX_65_TO_127        = 0xa,
    CNT_RX_128_TO_255       = 0xb,
    CNT_RX_256_TO_511       = 0xc,
    CNT_RX_512_TO_1023      = 0xd,
    CNT_RX_1024_TO_1526     = 0xe,
    CNT_RX_OVERSIZE_PKTS    = 0xf,
    CNT_RX_PAUSE            = 0x10,
    CNT_RX_CTRL             = 0x11,
    CNT_RX_LONG             = 0x12,
    CNT_RX_CAT_DROP         = 0x13,

#if UNMANAGED_PORT_STATISTICS_QOS
    CNT_RX_RED_PRIO_0       = 0x14,
    CNT_RX_RED_PRIO_1       = 0x15,
    CNT_RX_RED_PRIO_2       = 0x16,
    CNT_RX_RED_PRIO_3       = 0x17,
    CNT_RX_RED_PRIO_4       = 0x18,
    CNT_RX_RED_PRIO_5       = 0x19,
    CNT_RX_RED_PRIO_6       = 0x1a,
    CNT_RX_RED_PRIO_7       = 0x1b,

    CNT_RX_YELLOW_PRIO_0    = 0x1c,
    CNT_RX_YELLOW_PRIO_1    = 0x1d,
    CNT_RX_YELLOW_PRIO_2    = 0x1e,
    CNT_RX_YELLOW_PRIO_3    = 0x1f,
    CNT_RX_YELLOW_PRIO_4    = 0x20,
    CNT_RX_YELLOW_PRIO_5    = 0x21,
    CNT_RX_YELLOW_PRIO_6    = 0x22,
    CNT_RX_YELLOW_PRIO_7    = 0x23,

    CNT_RX_GREEN_PRIO_0     = 0x24,
    CNT_RX_GREEN_PRIO_1     = 0x25,
    CNT_RX_GREEN_PRIO_2     = 0x26,
    CNT_RX_GREEN_PRIO_3     = 0x27,
    CNT_RX_GREEN_PRIO_4     = 0x28,
    CNT_RX_GREEN_PRIO_5     = 0x29,
    CNT_RX_GREEN_PRIO_6     = 0x2a,
    CNT_RX_GREEN_PRIO_7     = 0x2b,

    CNT_TX_YELLOW_PRIO_0    = 0x4e,
    CNT_TX_YELLOW_PRIO_1    = 0x4f,
    CNT_TX_YELLOW_PRIO_2    = 0x50,
    CNT_TX_YELLOW_PRIO_3    = 0x51,
    CNT_TX_YELLOW_PRIO_4    = 0x52,
    CNT_TX_YELLOW_PRIO_5    = 0x53,
    CNT_TX_YELLOW_PRIO_6    = 0x54,
    CNT_TX_YELLOW_PRIO_7    = 0x55,

    CNT_TX_GREEN_PRIO_0     = 0x56,
    CNT_TX_GREEN_PRIO_1     = 0x57,
    CNT_TX_GREEN_PRIO_2     = 0x58,
    CNT_TX_GREEN_PRIO_3     = 0x59,
    CNT_TX_GREEN_PRIO_4     = 0x5a,
    CNT_TX_GREEN_PRIO_5     = 0x5b,
    CNT_TX_GREEN_PRIO_6     = 0x5c,
    CNT_TX_GREEN_PRIO_7     = 0x5d,
#endif // UNMANAGED_PORT_STATISTICS_QOS

    CNT_TX_OCTETS           = 0x40,
    CNT_TX_PKTS             = 0x41,
    CNT_TX_MCAST_PKTS       = 0x42,
    CNT_TX_BCAST_PKTS       = 0x43,
    CNT_TX_COLLISIONS       = 0x44,
    CNT_TX_DROP             = 0x45,
    CNT_TX_PAUSE            = 0x46,
    CNT_TX_64               = 0x47,
    CNT_TX_65_TO_127        = 0x48,
    CNT_TX_128_TO_255       = 0x49,
    CNT_TX_256_TO_511       = 0x4a,
    CNT_TX_512_TO_1023      = 0x4b,
    CNT_TX_1024_TO_1526     = 0x4c,
    CNT_TX_OVERSIZE_PKTS    = 0x4d,
    CNT_TX_AGED             = 0x5e,

    CNT_DROP_LOCAL          = 0x80,
    CNT_DROP_TAIL           = 0x81,

    /* Combined Counters */
    //Not defined in the FERRET Datasheet
    /*
    CNT_RX_UCAST_PKTS       = 0x7000,
    CNT_RX_NON_UCAST_PKTS   = 0x7001,
    CNT_RX_DISCARD_PKTS     = 0x7002,
    CNT_RX_ERR_PKTS         = 0x7003,
    CNT_RX_UNKNOWN_PROTOS   = 0x7004,
    CNT_RX_DROPEVENTS       = 0x7005,

    CNT_TX_UCAST_PKTS       = 0x7100,
    CNT_TX_NON_UCAST_PKTS   = 0x7101,
    CNT_TX_DISCARDS         = 0x7102,
    CNT_TX_ERR_PKTS         = 0x7103,
*/
//    CNT_RX_PKTS             = 0xfffd,
//    CNT_TX_PKTS             = 0xfffe,
    COUNTER_NONE            = 0xffff,
}port_statistics_t;
#endif
extern BOOL GPARM_break_show_statistic_flag;

uchar h2_stats_counter_exists(port_statistics_t counter_id);
ulong h2_stats_counter_get(vtss_cport_no_t chip_port, port_statistics_t counter_id);
void  h2_stats_counter_clear(vtss_cport_no_t chip_port);
void  print_port_statistics(vtss_cport_no_t chip_port);

#endif





