//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



/**
 * \file h2_packet.h
 * \brief  Injection/extraction packet header
 * \details This header file describes luton26 recieve packet formats.
 */

#ifndef __H2_PACKET_H__
#define __H2_PACKET_H__

/* Internal Extraction Frame Header, IFH0 (bit 63-32) and IFH1 (bit 31-0) */
#define O_IFH_PORT     51     /* 55-51: Port */
#define M_IFH_PORT     0x1f
#define O_IFH_DSCP     45     /* 50-45: DSCP */
#define M_IFH_ACL      0xff
#define O_IFH_SFLOW    32     /* 36-32: Sflow index */
#define M_IFH_SFLOW    0x1f
#define O_IFH_LEARN    28     /* 29-28: Learn */
#define M_IFH_LEARN    0x3
#define O_IFH_CPUQ     20     /* 27-20: CPU extraction queue */
#define M_IFH_CPUQ     0xff
#define O_IFH_PRIO     17     /* 19-17: Prioity */
#define M_IFH_PRIO     0x7
#define O_IFH_TAGTYPE  16     /* 16   : VLAN Tag type */
#define M_IFH_TAGTYPE  0x1
#define O_IFH_UPRIO    13     /* 15-13: Tag priority */
#define M_IFH_UPRIO    0x7
#define O_IFH_CFI      12     /* 12   : CFI */
#define M_IFH_CFI      0x1
#define O_IFH_VID      0      /* 11-0 : VID */
#define M_IFH_VID      0xfff

/* Internal Injection Frame Header, IFH0 (bit 63-32) and IFH1 (bit 31-0) */
#define O_IFH_BYPASS   63     /* 63: Bypass  */
#define M_IFH_BYPASS   0x1
#define O_IFH_PTP      61     /* 62-61: PTP */
#define M_IFH_PTP      0x3
#define O_IFH_DEST     32     /* 58-32: Destination port mask */
#define M_IFH_DEST     0x7ffffff
#define O_IFH_POPCNT   28     /* 29:28: VLAN tag pop count */
#define M_IFH_POPCNT   0x3
#define O_IFH_CPUQ     20     /* 27-20: CPU extraction queue */
#define M_IFH_CPUQ     0xff
#define O_IFH_PRIO     17     /* 19-17: Prioity */
#define M_IFH_PRIO     0x7
#define O_IFH_TAGTYPE  16     /* 16   : VLAN Tag type */
#define M_IFH_TAGTYPE  0x1
#define O_IFH_UPRIO    13     /* 15-13: Tag priority */
#define M_IFH_UPRIO    0x7
#define O_IFH_CFI      12     /* 12   : CFI */
#define M_IFH_CFI      0x1
#define O_IFH_VID      0      /* 11-0 : VID */
#define M_IFH_VID      0xfff

/* Get field from IFH0/IFH1 */
#define IFH_GET(ifh0, ifh1, field) ((O_IFH_##field > 31 ? (ifh0 >> (O_IFH_##field - 32)) : (ifh1 >> O_IFH_##field)) & M_IFH_##field)

/* Put field to IFH0/IFH1 */
#define IFH_PUT(ifh0, ifh1, field, val) { ifh0 |= (O_IFH_##field > 31 ? (((ulong) val)<<(O_IFH_##field - 32)) : 0); ifh1 |= (O_IFH_##field > 31 ? 0 : ((ulong) val)<<O_IFH_##field); }


/** \brief Receive packet IFH header */
typedef struct {
    uchar  port;  /* physical (MAC) source (ingress) port */
    uchar  dscp;
    uchar  acl_idx;
    uchar  sflow_id;
    uchar  acl_hit;
    uchar  dp;
    uchar  learn_flags;
    uchar  cpuq;
    uchar  prio;
    uchar  tag_type;
    uchar  uprio;
    uchar  cfi;
    ushort vid;
    uchar  arrived_tagged;
} vtss_packet_rx_header_t;

/** \brief Structure used for receiving a frame via h2_l26_rx_frame_get. */
typedef struct {
    uchar                   discard;      /**< discard flag */
    uchar                   pruned;       /**< pruned flag */
    uchar                   is_tx_pacekt; /**< tx flag to wait for transmit */
    ushort                  total_bytes;  /**< total_bytes = no_of_ulong * 4 - unused_bytes; */
    vtss_packet_rx_header_t header;       /**< receive IFH header */
    uchar  xdata            *rx_packet;   /**< packet body */
} vtss_rx_frame_t;


#define RECV_BUFSIZE 1514

/** \brief Representation of a 48-bit Ethernet address. */
typedef struct vtss_eth_addr_t {
    mac_addr_t addr;
} vtss_eth_addr;

/** \brief The Ethernet header. */
typedef struct vtss_eth_hdr_t {
    vtss_eth_addr dest;
    vtss_eth_addr src;
    ushort type;
} vtss_eth_hdr;

#define VTSS_ETHTYPE_ARP  0x0806
#define VTSS_ETHTYPE_IP   0x0800
#define VTSS_ETHTYPE_SLOW 0x8809
#define VTSS_ETHTYPE_IP6  0x86dd
#define VTSS_ETHTYPE_LLDP 0x88CC


#define HTONS(n) (n) /* 8051 uses big order */

/** \brief Description: CPU Rx group number
 *  \details This is a value in range [0; VTSS_PACKET_RX_GRP_CNT[.
 */
typedef uchar vtss_packet_rx_grp_t;

/** \brief CPU Rx queue buffer size in bytes */
typedef ulong vtss_packet_rx_queue_size_t;

/** \brief CPU Rx queue configuration */
typedef struct {
    vtss_packet_rx_queue_size_t     size; /**< Queue size */
} vtss_packet_rx_queue_conf_t;

/** \brief CPU Rx packet registration */
typedef struct {
    BOOL bpdu_cpu_only;        /**< Redirect BPDUs (DMAC 01-80-C2-00-00-0X) */
    BOOL garp_cpu_only[16];    /**< Redirect GARPs (DMAC 01-80-C2-00-00-2X) */
    BOOL ipmc_ctrl_cpu_copy;   /**< Copy IP MC control (DIP 224.0.0.x) to CPU */
    BOOL igmp_cpu_only;        /**< Redirect IGMP frames to the CPU */
    BOOL mld_cpu_only;         /**< Redirect MLD frames to the CPU */
    BOOL bridge_cpu_only;      /**< Redirect BRIDGE (DMAC 01-80-C2-00-00-10) */
} vtss_packet_rx_reg_t;

/** \brief CPU Rx queue map */
typedef struct {
    vtss_packet_rx_queue_t bpdu_queue;        /**< BPDUs */
    vtss_packet_rx_queue_t garp_queue;        /**< GARP frames */
    vtss_packet_rx_queue_t learn_queue;       /**< Learn frames */
    vtss_packet_rx_queue_t igmp_queue;        /**< IGMP/MLD frames */
    vtss_packet_rx_queue_t ipmc_ctrl_queue;   /**< IP multicast control frames */
    vtss_packet_rx_queue_t mac_vid_queue;     /**< MAC address table */
} vtss_packet_rx_queue_map_t;

/** \brief CPU Rx configuration */
typedef struct {
    //vtss_packet_rx_queue_conf_t queue[VTSS_PACKET_RX_QUEUE_CNT]; /**< Queue configuration */
    vtss_packet_rx_reg_t        reg;                             /**< Packet registration */
    vtss_packet_rx_queue_map_t  map;                             /**< Queue mapping */
    vtss_packet_rx_grp_t        grp_map[VTSS_PACKET_RX_QUEUE_CNT]; /**< Queue to extraction group map */
} vtss_packet_rx_conf_t;


#define FCS_SIZE_BYTES 4 /* Frame Check Sequence - FCS */

/* Number of extraction queues: 2, 4 or 8 */
#define PACKET_XTR_QU_CNT 8

/* The lowest queue number has highest priority */

#if (PACKET_XTR_QU_CNT == 2)
#define PACKET_XTR_QU_HIGH   (VTSS_PACKET_RX_QUEUE_START + 1)
#define PACKET_XTR_QU_MEDIUM (VTSS_PACKET_RX_QUEUE_START + 0)
#define PACKET_XTR_QU_NORMAL (VTSS_PACKET_RX_QUEUE_START + 0)
#define PACKET_XTR_QU_LOW    (VTSS_PACKET_RX_QUEUE_START + 0)
#endif /* PACKET_XTR_QU_CNT == 2 */

#if (PACKET_XTR_QU_CNT == 4)
#define PACKET_XTR_QU_HIGH   (VTSS_PACKET_RX_QUEUE_START + 3)
#define PACKET_XTR_QU_MEDIUM (VTSS_PACKET_RX_QUEUE_START + 2)
#define PACKET_XTR_QU_NORMAL (VTSS_PACKET_RX_QUEUE_START + 1)
#define PACKET_XTR_QU_LOW    (VTSS_PACKET_RX_QUEUE_START + 0)
#endif /* PACKET_XTR_QU_CNT == 4 */

#if (PACKET_XTR_QU_CNT == 8)
#define PACKET_XTR_QU_HIGH        (VTSS_PACKET_RX_QUEUE_START + 7)
#define PACKET_XTR_QU_MEDIUM_HIGH (VTSS_PACKET_RX_QUEUE_START + 6)
#define PACKET_XTR_QU_MEDIUM      (VTSS_PACKET_RX_QUEUE_START + 5)
#define PACKET_XTR_QU_MEDIUM_LOW  (VTSS_PACKET_RX_QUEUE_START + 4)
#define PACKET_XTR_QU_NORMAL_HIGH (VTSS_PACKET_RX_QUEUE_START + 3)
#define PACKET_XTR_QU_NORMAL      (VTSS_PACKET_RX_QUEUE_START + 2)
#define PACKET_XTR_QU_NORMAL_LOW  (VTSS_PACKET_RX_QUEUE_START + 1)
#define PACKET_XTR_QU_LOW         (VTSS_PACKET_RX_QUEUE_START + 0)
#endif /* PACKET_XTR_QU_CNT == 8 */

// Extraction Queue Allocation
// These are logical queue numbers!
#define PACKET_XTR_QU_SPROUT        PACKET_XTR_QU_HIGH
#define PACKET_XTR_QU_STACK         PACKET_XTR_QU_HIGH
#define PACKET_XTR_QU_BPDU_LLDP     PACKET_XTR_QU_HIGH /* For Ferret, we use the same queue(7) for BPDU and LLDP frame */
#define PACKET_XTR_QU_BPDU          PACKET_XTR_QU_MEDIUM
#define PACKET_XTR_QU_IGMP          PACKET_XTR_QU_MEDIUM
#define PACKET_XTR_QU_IP            PACKET_XTR_QU_NORMAL
#define PACKET_XTR_QU_MGMT_MAC      PACKET_XTR_QU_NORMAL /* For the switch's own MAC address                */
#define PACKET_XTR_QU_MAC           PACKET_XTR_QU_LOW    /* For other MAC addresses that require CPU copies */
#define PACKET_XTR_QU_BC            PACKET_XTR_QU_LOW    /* For Broadcast MAC address frames                */
#define PACKET_XTR_QU_LEARN         PACKET_XTR_QU_LOW /* For the sake of MAC-based Authentication */
#define PACKET_XTR_QU_ACL           PACKET_XTR_QU_LOW

#endif

