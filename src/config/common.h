//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#ifndef __COMMON_H__
#define __COMMON_H__

#define __INCLUDE_CONFIGS__ /* This is checked by the following headers */
#include "swconf.h"
#include "hwconf.h"
#include "phyconf.h"
#undef __INCLUDE_CONFIGS__


/****************************************************************************
 * General Types
 ****************************************************************************/
typedef unsigned char   uchar;
typedef unsigned int    uint;
typedef unsigned long   ulong;
typedef unsigned short  ushort;
typedef bit             bool;
typedef unsigned char   BOOL; /**< Boolean implemented as 8-bit unsigned */

typedef unsigned char   u8;
typedef unsigned short  u16;
typedef unsigned long   u32;

/****************************************************************************
 * Port Types
 ****************************************************************************/

/**
 * Port Number Type.
 *
 * This is the generic port type used everywhere. A variable declared with it
 * can not be distinguished by eyes if the variable is not well named.
 *
 * @todo    Modify all source code and remove this definition.
 */
typedef u8              vtss_port_no_t;

/**
 * Port bitmask for generic flags setting.
 */
typedef u32             port_bit_mask_t;

/**
 * Internal port number type.
 *
 * iport is 0-based. It is equal to front port number minus 1. Used for array
 * indexing.
 *
 * Example:
 *  vtss_fport_no_t fport_no = 5;
 *  vtss_iport_no_t iport_no;
 *
 *  iport_no = fport2iport(fport_no);
 *
 * @see vtss_fport_no_t, vtss_sport_no_t.
 */
typedef u8                  vtss_iport_no_t;

/**
 * Switch port number.
 *
 * Switch port is used to access switch ports. Each switch port is mapped to
 * a PHY port, SERDES port, or a dual media port (both PHY and SERDES
 * connectors are supported but only one of them can be used at a time.)
 *
 * Example:
 *  vtss_fport_no_t fport_no = 5;
 *  vtss_iport_no_t iport_no;
 *
 *  iport_no = fport2iport(fport_no);
 *
 * @see vtss_fport_no_t, vtss_iport_no_t.
 */
typedef u8                  vtss_sport_no_t;

/* Actually, chip port is the same as switch port.
 * We start to use the new definition from Ferret project
 * and keep the original definition(vtss_sport_no_t) for backward compatibly
 */
typedef vtss_sport_no_t     vtss_cport_no_t;

/**
 * Front port number.
 *
 * Front port is the only port number recognized by users. Each front port is
 * mapped to a PHY port, SERDES port, or a dual media port (both PHY and
 * SERDES connectors are supported but only one of them can be used at a
 * time.)
 *
 * Example:
 *  vtss_sport_no_t sport_no;
 *  vtss_iport_no_t iport_no = 5;
 *
 *  sport_no = iport2sport(iport_no);
 *
 * @see vtss_sport_no_t, vtss_iport_no_t.
 */
typedef u8                  vtss_fport_no_t;

/* Actually, front port is the same as user port.
 * We start to use the new definition from Ferret project
 * and keep the original definition(vtss_fport_no_t) for backward compatibly
 */
typedef vtss_fport_no_t     vtss_uport_no_t;


/****************************************************************************
 *
 ****************************************************************************/

typedef union {
    uchar  b [4];
    ushort w [2];
    ulong  l;
} ul_union_t;

/****************************************************************************
 * MAC Address Type
 ****************************************************************************/

#define MAC_ADDR_LEN    (6)

typedef u8 mac_addr_t [ MAC_ADDR_LEN ];

typedef struct {
    ushort          vid;
    mac_addr_t      mac_addr;
    port_bit_mask_t port_mask;
} mac_tab_t;

/****************************************************************************
 * Boolean
 ****************************************************************************/

#define FALSE  0
#define TRUE   1

/****************************************************************************
 * Array Sizing
 ****************************************************************************/

#define ARRAY_LENGTH( arr ) (sizeof(arr) / sizeof(arr[0]))

/****************************************************************************
 * MIN, MAX
 ****************************************************************************/

#define MIN(a,b) ((a) < (b) ? (a) : (b))
#define MAX(a,b) ((a) > (b) ? (a) : (b))

/****************************************************************************
 * Link Modes
 ****************************************************************************/

/*
 * Definitions that applies to the link_mode parameter used by setup_port and build_mac_config:
 * The link_mode parameter uses the following bit-mapping:
 *   bit 1-0 : speed (00 = 10 Mbit/s, 01 = 100 Mbit/s, 02 = 1000 Mbit/s)
 *   bit 4   : duplex mode (0 = HDX, 1 = FDX)
 *   bit 5   : enable pause frames (0 = disabled, 1 = enabled)
 *   bit 7   : internal loopback on PHYs (for loopback test only).
 *
 * Setting all bits to 1 is used to indicate that there is no link.
 * For convenience bits 1-0 are coded like PHYs do.
 */

#define LINK_MODE_SPEED_10              0
#define LINK_MODE_SPEED_100             1
#define LINK_MODE_SPEED_1000            2
#define LINK_MODE_SPEED_2500            3

#define LINK_MODE_SPEED_MASK            0x03    /* Bit[0:1] */
#define LINK_MODE_FDX_MASK              0x10    /* Bit[4]   */
#define LINK_MODE_PAUSE_MASK            0x20    /* Bit[5]   */
#define LINK_MODE_POWER_MASK            0xC0    /* Bit[7:6] */
#define LINK_MODE_POWER_MASK_100BASE    0x40
#define LINK_MODE_POWER_MASK_1000BASE   0x80

#define LINK_MODE_INT_LOOPBACK          0X80

#define LINK_MODE_DOWN                  0xFF

#define LINK_MODE_FDX_10                (LINK_MODE_SPEED_10 | LINK_MODE_FDX_MASK)
#define LINK_MODE_HDX_10                (LINK_MODE_SPEED_10)
#define LINK_MODE_FDX_100               (LINK_MODE_SPEED_100 | LINK_MODE_FDX_MASK)
#define LINK_MODE_HDX_100               (LINK_MODE_SPEED_100)
#define LINK_MODE_FDX_1000              (LINK_MODE_SPEED_1000 | LINK_MODE_FDX_MASK)
#define LINK_MODE_FDX_2500              (LINK_MODE_SPEED_2500 | LINK_MODE_FDX_MASK)

#define LINK_MODE_FDX_AND_PAUSE_MASK    (LINK_MODE_FDX_MASK | LINK_MODE_PAUSE_MASK)

#define LINK_MODE_SPEED_AND_FDX_MASK    (LINK_MODE_SPEED_MASK | LINK_MODE_FDX_MASK)

/****************************************************************************
 * IP Address Type
 ****************************************************************************/

#define IP_ADDR_LEN  4
typedef u8 ip_addr_t [IP_ADDR_LEN];

/****************************************************************************
 * Macros for optimizing access to non-byte variables
 ****************************************************************************/

/* for read */
#define LOW_BYTE(v)     ((uchar) (v))
#define HIGH_BYTE(v)    ((uchar) (((ushort) (v)) >> 8))

/* for write */
#define BYTE_LOW(v)     (*(((uchar *) (&v) + 1)))
#define BYTE_HIGH(v)    (*((uchar *) (&v)))

/* convert 2 bytes to an ushort */
#define MK_USHORT(H,L)  ( (ushort) bytes2ushort(L, H) )

/****************************************************************************
 * Bit Operations
 ****************************************************************************/

#define VTSS_BIT(x)                   ((ulong) 1 << (x))
#define VTSS_BITMASK(x)               (((ulong) 1 << (x)) - 1)
#define VTSS_EXTRACT_BITFIELD(x,o,w)  (((ulong) (x) >> (o)) & VTSS_BITMASK(w))
#define VTSS_ENCODE_BITFIELD(x,o,w)   ((ulong) ((x) & VTSS_BITMASK(w)) << (o))
#define VTSS_ENCODE_BITMASK(o,w)      (VTSS_BITMASK(w) << (o))

#define VTSS_BITOPS_DEFINED

/****************************************************************************
 * Number of Ports and Related
 ****************************************************************************/
#define PORT_BIT_MASK(bit_no)                       bit_mask_32((bit_no))
#define WRITE_PORT_BIT_MASK(bit_no,bit_val,dst_ptr) write_bit_32((bit_no), (bit_val), (dst_ptr))
#define TEST_PORT_BIT_MASK(bit_no,dst_ptr)          test_bit_32((bit_no), (dst_ptr))

#if defined(FERRET_F11) || defined(FERRET_F10P) || defined(FERRET_F5) || defined(FERRET_F4P)
    #define CPU_CHIP_PORT           11
    #define SYS_LED_SGPIO_PORT      11
    #define NO_OF_CHIP_PORTS        11

    #if defined(FERRET_F11)
        #if defined(NPI_CHIP_PORT)
            #define NO_OF_BOARD_PORTS   11
            #define ALL_PORTS           0x000007FF
            #define LED_PORTS           (0x000005FF | VTSS_BIT(SYS_LED_SGPIO_PORT))  /* SGPIO doesn't control NPI port(chip_port=9) */
        #else // None NPI
            #define NO_OF_BOARD_PORTS   10
            #define ALL_PORTS           0x000005FF
            #define LED_PORTS           (0x000005FF | VTSS_BIT(SYS_LED_SGPIO_PORT))
        #endif // NPI_CHIP_PORT

    #elif defined(FERRET_F10P)
        #define NO_OF_BOARD_PORTS       10
        #define ALL_PORTS               0x000003FF  /* Port mask for physical ports, SFP2 on chip_port=9 and PCIe on chip_port=10 */
        #define LED_PORTS               (0x000003FF | VTSS_BIT(SYS_LED_SGPIO_PORT))

    #elif defined(FERRET_F5)
        #if defined(NPI_CHIP_PORT)
            #define NO_OF_BOARD_PORTS   5
            #define ALL_PORTS           0x0000040F  /* Port mask for physical ports */
            #define LED_PORTS           (0x000005FF | VTSS_BIT(SYS_LED_SGPIO_PORT)) /* 10 bits should be set to 1 in b[0-10] since system LED SGPIO is 11 */
        #else // None NPI
            #define NO_OF_BOARD_PORTS   4
            #define ALL_PORTS           0x0000000F  /* Port mask for physical ports */
            #define LED_PORTS           (0x000003FF | VTSS_BIT(SYS_LED_SGPIO_PORT)) /* 10 bits should be set to 1 in b[0-10] since system LED SGPIO is 11 */
        #endif // NPI_CHIP_PORT

    #elif defined(FERRET_F4P)
        #define NO_OF_BOARD_PORTS       4
        #define ALL_PORTS               0x0000000F  /* Port mask for physical ports */
        #define LED_PORTS               (0x000003FF | VTSS_BIT(SYS_LED_SGPIO_PORT)) /* 10 bits should be set to 1 in b[0-10] since system LED SGPIO is 11 */

    #endif  // FERRET

#else
    #error "Not defined yet: board ports"
#endif

#define MIN_PORT                0
#if MIN_PORT
#error "MUST NOT change the value to non-zero unless you are understood whole the application of port mapping"
#endif
#define MAX_PORT                NO_OF_BOARD_PORTS

#define MAX_AGGR_GROUP          3

/* The first logical port number is 0. */
#define VTSS_PORT_NO_NONE       (0xffffffff)        /**< Port number none */
#define VTSS_PORT_ARRAY_SIZE    NO_OF_BOARD_PORTS   /**< Port number array size */

#define SYSTEM_MAC_ADDR         MAX_PORT

#define VTSS_IOADDR(t,o)        (t + (4*((ulong) o)))
#define VTSS_IOREG(t,o)         ((ulong) VTSS_IOADDR(t,o))

#define VTSS_BOOL(expr)         ((expr) ? 1 : 0)

/****************************************************************************
 * Queues
 ****************************************************************************/

/** \brief Description: CPU queue Number */
typedef ulong vtss_packet_rx_queue_t;

#define NO_OF_FASTQ                 8

/** \brief Description: CPU Rx queue Number */
#define VTSS_PACKET_TX_QUEUE_CNT    2  /**< Number of Tx packet queues */
#define VTSS_PACKET_TX_QUEUE_START  (0) /**< Rx queue start number */
#define VTSS_PACKET_TX_QUEUE_END    (VTSS_PACKET_TX_QUEUE_START+VTSS_PACKET_TX_QUEUE_CNT) /**< Rx queue end number */

/** \brief Description: CPU Rx queue Number */
#define VTSS_PACKET_RX_GROUP_CNT    2  /**< Number of Rx packet groups */
#define VTSS_PACKET_RX_GROUP_START (0) /**< Rx group start number */
#define VTSS_PACKET_RX_GROUP_END   (VTSS_PACKET_RX_GROUP_START+VTSS_PACKET_RX_GROUP_CNT) /**< Rx group end number */

/** \brief Description: CPU Rx queue Number */
#define VTSS_PACKET_RX_QUEUE_CNT    8  /**< Number of Rx packet queues */

#define VTSS_PACKET_RX_QUEUE_START (0) /**< Rx queue start number */
#define VTSS_PACKET_RX_QUEUE_END   (VTSS_PACKET_RX_QUEUE_START+VTSS_PACKET_RX_QUEUE_CNT) /**< Rx queue end number */

/****************************************************************************
 * VLAN Identifier
 ****************************************************************************/

/** \brief VLAN Identifier */
typedef ushort vtss_vid_t; /* 0-4095 */

#define VTSS_VID_NULL       ((const vtss_vid_t)0)     /**< NULL VLAN ID */
#define VTSS_VID_DEFAULT    ((const vtss_vid_t)1)     /**< Default VLAN ID */
#define VTSS_VID_RESERVED   ((const vtss_vid_t)0xFFF) /**< Reserved VLAN ID */
#define VTSS_VIDS           ((const vtss_vid_t)4096)  /**< Number of VLAN IDs */
#define VTSS_VID_ALL        ((const vtss_vid_t)0x1000)/**< Untagged VID: All VLAN IDs */

/****************************************************************************
 * XDATA NULL Pointer
 ****************************************************************************/

/** NULL xdata pointer */
#define XNULL               ((void xdata*)0)

void sleep_ms(u32 ms);
#endif /* __COMMON_H__ */
