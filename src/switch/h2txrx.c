//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#include "common.h"     /* Always include common.h at the first place of user-defined herder files */
#include "vtss_api_base_regs.h"
#include "h2io.h"
#include "h2.h"
#include "h2txrx.h"
#include "timer.h"
#include "misc2.h"

#include "h2packet.h"
#include "h2txrxaux.h"

#include "hwport.h"
#include "swconf.h"
#include "taskdef.h"
#include "vtss_common_os.h"
#include "lldp.h"

#if defined(VTSS_ARCH_OCELOT)
#if TRANSIT_LLDP || TRANSIT_LACP
#define __BASIC_TX_RX__ 1
#include "vtss_lacp.h"
#else
#define __BASIC_TX_RX__ 0
#endif
#elif defined(VTSS_ARCH_LUTON26)
#if TRANSIT_LLDP || LOOPBACK_TEST || TRANSIT_VERIPHY
#define __BASIC_TX_RX__ 1
#else
#define __BASIC_TX_RX__ 0
#endif
#endif

#if __BASIC_TX_RX__
/*****************************************************************************
 *
 *
 * Defines
 *
 *
 *
 ****************************************************************************/
#if defined(H2TXRX_DEBUG_ENABLE)
#include "print.h"
#endif /* H2TXRX_DEBUG_ENABLE */


#define MIN_FRAME_SIZE  64

#define PADDING_PATTERN 0x55555555

#define CPU_XTR_REG		0 /* CPU Extraction GRP - register based */
#define CPU_XTR_DMA		1 /* CPU Extraction GRP - DMA based */

#define CPU_INJ_REG		0 /* CPU Injection GRP - register based */
#define CPU_INJ_DMA		1 /* CPU Injection GRP - DMA based */

/*****************************************************************************
 *
 *
 * Typedefs and enums
 *
 *
 *
 ****************************************************************************/

#define BUF ((vtss_eth_hdr *)&rx_packet[0])

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
 * Public data
 *
 *
 *
 ****************************************************************************/



uchar xdata rx_packet[RECV_BUFSIZE+2];   /* The packet buffer that
                                            contains incoming packets. */
vtss_rx_frame_t vtss_rx_frame;

/*****************************************************************************
 *
 *
 * Local data
 *
 *
 *
 ****************************************************************************/

static uchar rx_packet_tsk_init = 0;
vtss_packet_rx_conf_t rx_conf;



/* ************************************************************************ */
void h2_send_frame (uchar port_no, uchar xdata *frame_ptr, ushort frame_len)
/* ------------------------------------------------------------------------ --
 * Purpose     : Send a frame on the specified port.
 * Remarks     : port_no specifies the Heathrow transmit port.
 *               frame_ptr points to the frame data and frame_len specifies
 *               the length of frame data in number of bytes.
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    uchar ret;
    ret = h2_tx_frame_port(port_no, frame_ptr, frame_len, VTSS_VID_NULL);
    if(!ret) {
        ;
    }
}

/*****************************************************************************
 *
 *
 * Luton26 tx/rx functions
 *
 *
 *
 ****************************************************************************/

static void h2_rx_grp_conf (void)
/* ------------------------------------------------------------------------ --
 * Purpose     : configures a single group for extraction from one or more queues.
 * Remarks     :
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
#if defined(VTSS_ARCH_OCELOT)
    // map the 8 CPU extraction queues to CPU port 11.
    H2_WRITE(VTSS_QSYS_SYSTEM_CPU_GROUP_MAP, 0);

    // Don't do byte-swap and expect status after last data word
    // Extraction: Mode: 0b0100(manual extraction) | POS:0b0010 | Byte_swap: 0b0000 = 0x6
    H2_WRITE(VTSS_DEVCPU_QS_XTR_XTR_GRP_CFG(0), VTSS_F_DEVCPU_QS_XTR_XTR_GRP_CFG_MODE(1)| VTSS_F_DEVCPU_QS_XTR_XTR_GRP_CFG_STATUS_WORD_POS(0)|VTSS_F_DEVCPU_QS_XTR_XTR_GRP_CFG_BYTE_SWAP(0));
    H2_WRITE(VTSS_DEVCPU_QS_XTR_XTR_GRP_CFG(1), VTSS_F_DEVCPU_QS_XTR_XTR_GRP_CFG_MODE(1)| VTSS_F_DEVCPU_QS_XTR_XTR_GRP_CFG_STATUS_WORD_POS(0)|VTSS_F_DEVCPU_QS_XTR_XTR_GRP_CFG_BYTE_SWAP(0));
    
    // Injaction: Mode: 0b0100(manual extraction) | Byte_swap: 0b0000 = 0x4    
    H2_WRITE(VTSS_DEVCPU_QS_INJ_INJ_GRP_CFG(0), VTSS_F_DEVCPU_QS_INJ_INJ_GRP_CFG_MODE(1)| VTSS_F_DEVCPU_QS_INJ_INJ_GRP_CFG_BYTE_SWAP(0));
    H2_WRITE(VTSS_DEVCPU_QS_INJ_INJ_GRP_CFG(1), VTSS_F_DEVCPU_QS_INJ_INJ_GRP_CFG_MODE(1)| VTSS_F_DEVCPU_QS_INJ_INJ_GRP_CFG_BYTE_SWAP(0));
    

    /* Enable IFH insertion/parsing on CPU ports */
    H2_WRITE_MASKED(VTSS_SYS_SYSTEM_PORT_MODE(CPU_CHIP_PORT), VTSS_F_SYS_SYSTEM_PORT_MODE_INCL_INJ_HDR(1)|VTSS_F_SYS_SYSTEM_PORT_MODE_INCL_XTR_HDR(1),
                    VTSS_M_SYS_SYSTEM_PORT_MODE_INCL_XTR_HDR | VTSS_M_SYS_SYSTEM_PORT_MODE_INCL_INJ_HDR);
    H2_WRITE_MASKED(VTSS_SYS_SYSTEM_PORT_MODE(CPU_CHIP_PORT + 1), VTSS_F_SYS_SYSTEM_PORT_MODE_INCL_INJ_HDR(1)|VTSS_F_SYS_SYSTEM_PORT_MODE_INCL_XTR_HDR(1),
                    VTSS_M_SYS_SYSTEM_PORT_MODE_INCL_XTR_HDR | VTSS_M_SYS_SYSTEM_PORT_MODE_INCL_INJ_HDR);;

    /* Setup the CPU port as VLAN aware to support switching frames based on tags */
    H2_WRITE(VTSS_ANA_PORT_VLAN_CFG(CPU_CHIP_PORT),
             VTSS_F_ANA_PORT_VLAN_CFG_VLAN_AWARE_ENA(1)  |
             VTSS_F_ANA_PORT_VLAN_CFG_VLAN_POP_CNT(1) |
             VTSS_F_ANA_PORT_VLAN_CFG_VLAN_VID(1));
    
    
    /* Disable learning (only RECV_ENA must be set) */
    H2_WRITE(VTSS_ANA_PORT_PORT_CFG(CPU_CHIP_PORT), VTSS_F_ANA_PORT_PORT_CFG_RECV_ENA(1));
    // Enable switching to/from cpu port
    H2_WRITE_MASKED(VTSS_QSYS_SYSTEM_SWITCH_PORT_MODE(CPU_CHIP_PORT), VTSS_M_QSYS_SYSTEM_SWITCH_PORT_MODE_PORT_ENA,
                    VTSS_M_QSYS_SYSTEM_SWITCH_PORT_MODE_PORT_ENA);
#if 0
    /* Setup CPU port 0 and 1 to allow for classification of transmission of
    * switched frames into a user-module-specifiable QoS class.
    * For the two CPU ports, we set a one-to-one mapping between a VLAN tag's
    * PCP and a QoS class. When transmitting switched frames, the PCP value
    * of the VLAN tag (which is always inserted to get it switched on a given
    * VID), then controls the priority. */
    /* Enable looking into PCP bits */
    H2_WRITE(VTSS_ANA_PORT_QOS_CFG(CPU_CHIP_PORT), VTSS_F_ANA_PORT_QOS_CFG_QOS_PCP_ENA);

    /* Disable aging of Rx CPU queues to allow the frames to stay there longer than
     * on normal front ports. */
    H2_WRITE_MASKED(VTSS_REW_PORT_PORT_CFG(CPU_CHIP_PORT), VTSS_F_REW_PORT_PORT_CFG_AGE_DIS, VTSS_F_REW_PORT_PORT_CFG_AGE_DIS);

    /* Disallow the CPU Rx queues to use shared memory. */
    //H2_WRITE_MASKED(VTSS_SYS_SYSTEM_EGR_NO_SHARING, VTSS_BIT(CPU_CHIP_PORT), VTSS_BIT(CPU_CHIP_PORT));
#endif

    H2_WRITE_MASKED(VTSS_SYS_PAUSE_CFG_PAUSE_CFG(CPU_CHIP_PORT), 0x0UL,0x1UL);
    // Enable Tail Drop on CPU port.
    //James H2_WRITE(VTSS_SYS_PAUSE_CFG_ATOP(CPU_CHIP_PORT), 0x0UL);
#if 0
    // Enable drop the frame from queue group 1 always.
    H2_WRITE(VTSS_DEVCPU_QS_XTR_XTR_FLUSH, 0x2);
#endif
    // When enabled for a port, frames to the port are discarded, even when the
    // ingress port is enabled for flow control.

    //James H2_WRITE_MASKED(VTSS_QSYS_DROP_CFG_EGR_DROP_MODE, 1UL<<CPU_CHIP_PORT, 1UL<<CPU_CHIP_PORT);
    H2_WRITE_MASKED(VTSS_QSYS_SYSTEM_EGR_NO_SHARING, 1UL<<CPU_CHIP_PORT, 1UL<<CPU_CHIP_PORT);
    H2_WRITE_MASKED(VTSS_REW_PORT_PORT_CFG(CPU_CHIP_PORT+1), VTSS_F_REW_PORT_PORT_CFG_FLUSH_ENA(1),
                    VTSS_F_REW_PORT_PORT_CFG_FLUSH_ENA(1));
#elif defined(VTSS_ARCH_LUTON26)
    // map port 26 to devcpu-group 0 and 27 to devcpu-group 1
    H2_WRITE(VTSS_DEVCPU_QS_XTR_XTR_MAP(0), VTSS_F_DEVCPU_QS_XTR_XTR_MAP_MAP_ENA);
    H2_WRITE(VTSS_DEVCPU_QS_XTR_XTR_MAP(1), VTSS_F_DEVCPU_QS_XTR_XTR_MAP_GRP|VTSS_F_DEVCPU_QS_XTR_XTR_MAP_MAP_ENA);

    // Don't do byte-swap and expect status before last data word
    H2_WRITE(VTSS_DEVCPU_QS_XTR_XTR_GRP_CFG(0), 0x0);
    H2_WRITE(VTSS_DEVCPU_QS_XTR_XTR_GRP_CFG(1), 0x0);


    /* Enable IFH insertion on CPU ports */
    H2_WRITE_MASKED(VTSS_REW_PORT_PORT_CFG(CPU_CHIP_PORT), VTSS_F_REW_PORT_PORT_CFG_IFH_INSERT_ENA,
                    VTSS_F_REW_PORT_PORT_CFG_IFH_INSERT_ENA);
    H2_WRITE_MASKED(VTSS_REW_PORT_PORT_CFG(CPU_CHIP_PORT+1), VTSS_F_REW_PORT_PORT_CFG_IFH_INSERT_ENA,
                    VTSS_F_REW_PORT_PORT_CFG_IFH_INSERT_ENA);

    /* Enable IFH parsing on CPU ports */
    H2_WRITE_MASKED(VTSS_SYS_SYSTEM_PORT_MODE(CPU_CHIP_PORT), VTSS_F_SYS_SYSTEM_PORT_MODE_INCL_INJ_HDR,
                    VTSS_F_SYS_SYSTEM_PORT_MODE_INCL_INJ_HDR);
    H2_WRITE_MASKED(VTSS_SYS_SYSTEM_PORT_MODE(CPU_CHIP_PORT+1), VTSS_F_SYS_SYSTEM_PORT_MODE_INCL_INJ_HDR,
                    VTSS_F_SYS_SYSTEM_PORT_MODE_INCL_INJ_HDR);

    /* Do not swap injection-data on CPU ports */
    H2_WRITE_MASKED(VTSS_DEVCPU_QS_INJ_INJ_GRP_CFG(0), 1,
                    VTSS_F_DEVCPU_QS_INJ_INJ_GRP_CFG_BYTE_SWAP);
    H2_WRITE_MASKED(VTSS_DEVCPU_QS_INJ_INJ_GRP_CFG(1), 1,
                    VTSS_F_DEVCPU_QS_INJ_INJ_GRP_CFG_BYTE_SWAP);

    /* Setup the CPU port as VLAN aware to support switching frames based on tags */
    H2_WRITE(VTSS_ANA_PORT_VLAN_CFG(CPU_CHIP_PORT),
             VTSS_F_ANA_PORT_VLAN_CFG_VLAN_AWARE_ENA  |
             VTSS_F_ANA_PORT_VLAN_CFG_VLAN_POP_CNT(1) |
             VTSS_F_ANA_PORT_VLAN_CFG_VLAN_VID(1));

    /* Disable learning (only RECV_ENA must be set) */
    H2_WRITE(VTSS_ANA_PORT_PORT_CFG(CPU_CHIP_PORT), VTSS_F_ANA_PORT_PORT_CFG_RECV_ENA);

    // Enable switching to/from cpu port
    H2_WRITE_MASKED(VTSS_SYS_SYSTEM_SWITCH_PORT_MODE(CPU_CHIP_PORT), VTSS_F_SYS_SYSTEM_SWITCH_PORT_MODE_PORT_ENA,
                    VTSS_F_SYS_SYSTEM_SWITCH_PORT_MODE_PORT_ENA);

#if 0
    /* Setup CPU port 0 and 1 to allow for classification of transmission of
    * switched frames into a user-module-specifiable QoS class.
    * For the two CPU ports, we set a one-to-one mapping between a VLAN tag's
    * PCP and a QoS class. When transmitting switched frames, the PCP value
    * of the VLAN tag (which is always inserted to get it switched on a given
    * VID), then controls the priority. */
    /* Enable looking into PCP bits */
    H2_WRITE(VTSS_ANA_PORT_QOS_CFG(CPU_CHIP_PORT), VTSS_F_ANA_PORT_QOS_CFG_QOS_PCP_ENA);

    /* Disable aging of Rx CPU queues to allow the frames to stay there longer than
     * on normal front ports. */
    H2_WRITE_MASKED(VTSS_REW_PORT_PORT_CFG(CPU_CHIP_PORT), VTSS_F_REW_PORT_PORT_CFG_AGE_DIS, VTSS_F_REW_PORT_PORT_CFG_AGE_DIS);

    /* Disallow the CPU Rx queues to use shared memory. */
    //H2_WRITE_MASKED(VTSS_SYS_SYSTEM_EGR_NO_SHARING, VTSS_BIT(CPU_CHIP_PORT), VTSS_BIT(CPU_CHIP_PORT));
#endif

    H2_WRITE_MASKED(VTSS_SYS_PAUSE_CFG_PAUSE_CFG(CPU_CHIP_PORT), 0x0UL,0x1UL);
    // Enable Tail Drop on CPU port.
    H2_WRITE(VTSS_SYS_PAUSE_CFG_ATOP(CPU_CHIP_PORT), 0x0UL);
#if 0
    // Enable drop the frame from queue group 1 always.
    H2_WRITE(VTSS_DEVCPU_QS_XTR_XTR_FLUSH, 0x2);
#endif
    // When enabled for a port, frames to the port are discarded, even when the
    // ingress port is enabled for flow control.
    H2_WRITE_MASKED(VTSS_SYS_PAUSE_CFG_EGR_DROP_FORCE, 1UL<<CPU_CHIP_PORT, 1UL<<CPU_CHIP_PORT);
    H2_WRITE_MASKED(VTSS_SYS_SYSTEM_EGR_NO_SHARING, 1UL<<CPU_CHIP_PORT, 1UL<<CPU_CHIP_PORT);
    H2_WRITE_MASKED(VTSS_REW_PORT_PORT_CFG(CPU_CHIP_PORT+1), VTSS_F_REW_PORT_PORT_CFG_FLUSH_ENA,
                    VTSS_F_REW_PORT_PORT_CFG_FLUSH_ENA);


    //todo: QSYS::EQ_PREFER_SRC

#endif
}

#if !defined(VTSS_ARCH_OCELOT)
void h2_bpdu_t_registration (uchar type, uchar enable)
{
    uchar  port_ext;
    ushort bpdu_cfg_redir_mask = 0;
    ushort bpdu_cfg_redir_eab  = 0;

    write_bit_16(type, enable, &bpdu_cfg_redir_eab);
    write_bit_16(type, 1,      &bpdu_cfg_redir_mask);

    for (port_ext = 1; port_ext < NO_OF_BOARD_PORTS; port_ext++) {
        H2_WRITE_MASKED(VTSS_ANA_PORT_CPU_FWD_BPDU_CFG(uport2cport(port_ext)),
                        bpdu_cfg_redir_eab, bpdu_cfg_redir_mask);
    }
}
#endif // VTSS_ARCH_OCELOT

void h2_rx_conf_set (void)
{
    vtss_packet_rx_conf_t       *conf = &rx_conf;
    vtss_packet_rx_reg_t        *reg = &conf->reg;
    vtss_packet_rx_queue_map_t  *map = &conf->map;
    ulong                       i, value;
    uchar                       port_no;
    vtss_iport_no_t             iport_idx;
    vtss_cport_no_t             chip_port;

    /* Rx IPMC registration */
    value =
#if defined(VTSS_ARCH_OCELOT)
        (reg->bridge_cpu_only    ? VTSS_F_ANA_PORT_CPU_FWD_CFG_CPU_ALLBRIDGE_REDIR_ENA(1) : 0) |
        (reg->ipmc_ctrl_cpu_copy ? VTSS_F_ANA_PORT_CPU_FWD_CFG_CPU_IPMC_CTRL_COPY_ENA(1)  : 0) |
        (reg->igmp_cpu_only      ? VTSS_F_ANA_PORT_CPU_FWD_CFG_CPU_IGMP_REDIR_ENA(1)      : 0) |
        (reg->mld_cpu_only       ? VTSS_F_ANA_PORT_CPU_FWD_CFG_CPU_MLD_REDIR_ENA(1)       : 0);
#elif defined(VTSS_ARCH_LUTON26)
        (reg->bridge_cpu_only    ? VTSS_F_ANA_PORT_CPU_FWD_CFG_CPU_ALLBRIDGE_REDIR_ENA : 0) |
        (reg->ipmc_ctrl_cpu_copy ? VTSS_F_ANA_PORT_CPU_FWD_CFG_CPU_IPMC_CTRL_COPY_ENA  : 0) |
        (reg->igmp_cpu_only      ? VTSS_F_ANA_PORT_CPU_FWD_CFG_CPU_IGMP_REDIR_ENA      : 0) |
        (reg->mld_cpu_only       ? VTSS_F_ANA_PORT_CPU_FWD_CFG_CPU_MLD_REDIR_ENA       : 0);
#endif

    for (iport_idx = MIN_PORT; iport_idx < MAX_PORT; iport_idx++) {
        chip_port = iport2cport(iport_idx);
        H2_WRITE(VTSS_ANA_PORT_CPU_FWD_CFG(chip_port), value);
    }

    /* Rx BPDUs registration */
    value = (reg->bpdu_cpu_only ? VTSS_M_ANA_PORT_CPU_FWD_BPDU_CFG_BPDU_REDIR_ENA : 0);
    //print_str("VTSS_ANA_PORT_CPU_FWD_BPDU_CFG: ");    print_dec(value);     print_cr_lf();            
    for (iport_idx = MIN_PORT; iport_idx < MAX_PORT; iport_idx++) {
        chip_port = iport2cport(iport_idx);
        //print_str("chip_port: ");    print_dec(chip_port);     print_cr_lf();
        H2_WRITE(VTSS_ANA_PORT_CPU_FWD_BPDU_CFG(chip_port), value);
    }


    /* Rx GARP registration */
    for (value = i = 0; i < 16; i++)
        if(reg->garp_cpu_only[i])
            value |= VTSS_BIT(i);
    for (iport_idx = MIN_PORT; iport_idx < MAX_PORT; iport_idx++) {
        chip_port = iport2cport(iport_idx);
        H2_WRITE(VTSS_ANA_PORT_CPU_FWD_GARP_CFG(port_no), value);
    }

    /* Fixme - chipset has more queues than the classification the API expose */
    value =
        VTSS_F_ANA_COMMON_CPUQ_CFG_CPUQ_SFLOW(VTSS_PACKET_RX_QUEUE_START) |
        VTSS_F_ANA_COMMON_CPUQ_CFG_CPUQ_MIRROR(map->mac_vid_queue-VTSS_PACKET_RX_QUEUE_START) |
        VTSS_F_ANA_COMMON_CPUQ_CFG_CPUQ_LRN(map->learn_queue-VTSS_PACKET_RX_QUEUE_START) |
        VTSS_F_ANA_COMMON_CPUQ_CFG_CPUQ_MAC_COPY(map->mac_vid_queue-VTSS_PACKET_RX_QUEUE_START) |
        VTSS_F_ANA_COMMON_CPUQ_CFG_CPUQ_SRC_COPY(map->mac_vid_queue-VTSS_PACKET_RX_QUEUE_START) |
        VTSS_F_ANA_COMMON_CPUQ_CFG_CPUQ_LOCKED_PORTMOVE(map->mac_vid_queue-VTSS_PACKET_RX_QUEUE_START) |
        VTSS_F_ANA_COMMON_CPUQ_CFG_CPUQ_ALLBRIDGE(map->bpdu_queue-VTSS_PACKET_RX_QUEUE_START) |
        VTSS_F_ANA_COMMON_CPUQ_CFG_CPUQ_IPMC_CTRL(map->ipmc_ctrl_queue-VTSS_PACKET_RX_QUEUE_START) |
        VTSS_F_ANA_COMMON_CPUQ_CFG_CPUQ_IGMP(map->igmp_queue-VTSS_PACKET_RX_QUEUE_START) |
        VTSS_F_ANA_COMMON_CPUQ_CFG_CPUQ_MLD(map->igmp_queue-VTSS_PACKET_RX_QUEUE_START);
    H2_WRITE(VTSS_ANA_COMMON_CPUQ_CFG, value);

    /* Setup each of the BPDU, GARP and CCM 'address' extraction queues */
    for (i = 0; i < 16; i++) {
        value =
            VTSS_F_ANA_COMMON_CPUQ_8021_CFG_CPUQ_BPDU_VAL(map->bpdu_queue-VTSS_PACKET_RX_QUEUE_START) |
            VTSS_F_ANA_COMMON_CPUQ_8021_CFG_CPUQ_GARP_VAL(map->garp_queue-VTSS_PACKET_RX_QUEUE_START) |
            VTSS_F_ANA_COMMON_CPUQ_8021_CFG_CPUQ_CCM_VAL(VTSS_PACKET_RX_QUEUE_START); /* Fixme */
        H2_WRITE(VTSS_ANA_COMMON_CPUQ_8021_CFG(i), value);
    }

#if TRANSIT_LLDP
    H2_WRITE_MASKED(VTSS_ANA_COMMON_CPUQ_8021_CFG(0xe),
                    VTSS_F_ANA_COMMON_CPUQ_8021_CFG_CPUQ_BPDU_VAL(PACKET_XTR_QU_BPDU_LLDP-VTSS_PACKET_RX_QUEUE_START),
                    VTSS_M_ANA_COMMON_CPUQ_8021_CFG_CPUQ_BPDU_VAL);
#endif

#if TRANSIT_LACP
    H2_WRITE_MASKED(VTSS_ANA_COMMON_CPUQ_8021_CFG(0x2), 
                    VTSS_F_ANA_COMMON_CPUQ_8021_CFG_CPUQ_BPDU_VAL(PACKET_XTR_QU_BPDU_LLDP-VTSS_PACKET_RX_QUEUE_START),
                    VTSS_M_ANA_COMMON_CPUQ_8021_CFG_CPUQ_BPDU_VAL);
#endif
}

#if LOOPBACK_TEST
void h2_rx_flush (void) small
/* ------------------------------------------------------------------------ --
 * Purpose     : Flush frame from rx queue 0.
 * Remarks     :
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    H2_WRITE_MASKED(VTSS_DEVCPU_QS_XTR_XTR_FLUSH, 0x2, 0x2);
    delay(MSEC_20);
    H2_WRITE_MASKED(VTSS_DEVCPU_QS_XTR_XTR_FLUSH, 0x0, 0x2);
}
#endif /* LOOPBACK_TEST */

/* ************************************************************************ */
void h2_rx_init (void)
/* ------------------------------------------------------------------------ --
 * Purpose     : Init rx queues.
 * Remarks     :
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    if(!rx_packet_tsk_init) {
        uchar                      i;
        vtss_packet_rx_conf_t      *conf = &rx_conf;
        vtss_packet_rx_reg_t       *reg = &conf->reg;

        /* reset rx_conf varaible  */
        memset(conf, 0, sizeof(vtss_packet_rx_conf_t));

        /* Setup Group queue mapping */
        h2_rx_grp_conf();

        /* Setup Rx queue mapping */
        conf->map.bpdu_queue      = PACKET_XTR_QU_BPDU;
        conf->map.garp_queue      = PACKET_XTR_QU_BPDU;
        conf->map.learn_queue     = PACKET_XTR_QU_LEARN;
        conf->map.igmp_queue      = PACKET_XTR_QU_IGMP;
        conf->map.ipmc_ctrl_queue = PACKET_XTR_QU_IGMP;
        conf->map.mac_vid_queue   = PACKET_XTR_QU_IP;


        for (i = 0; i < VTSS_PACKET_RX_QUEUE_END; i++) {
            conf->grp_map[i] = 1;
        }

        conf->grp_map[PACKET_XTR_QU_BPDU_LLDP] = 0;


        
        /* Setup Rx queue registration */
        //print_str("TRANSIT_BPDU_PASS_THROUGH: ");    print_dec(TRANSIT_BPDU_PASS_THROUGH);     print_cr_lf();
#if TRANSIT_BPDU_PASS_THROUGH
        reg->bpdu_cpu_only = 0;
#else
        reg->bpdu_cpu_only = 1;
#endif
         //print_str("reg->bpdu_cpu_only: ");    print_dec(reg->bpdu_cpu_only);     print_cr_lf();
        
        h2_rx_conf_set();

        vtss_rx_frame.rx_packet = rx_packet;
        vtss_rx_frame.discard = 1;
        vtss_rx_frame.total_bytes = 0;
        vtss_rx_frame.pruned = 0;
        rx_packet_tsk_init = 1;
    }
}

#if defined(H2TXRX_DEBUG_ENABLE)
void dump_frame(uchar *rx_packet , ulong total_bytes)
{
    ulong i;
    
    print_str("len: "); print_dec(total_bytes); print_cr_lf();
    for (i=0; i< total_bytes; i++) {
    
        if(i!= 0 && (i%16) == 0) {
            print_cr_lf(); 
        } else if(i!= 0 && (i%8) == 0) {
            //print_spaces(3);
        }    
       print_hex_b(*rx_packet++); print_spaces(1);
    } 
    print_cr_lf();   
}   
#endif /* H2TXRX_DEBUG_ENABLE */

/*****************************************************************************
 *
 *
 * Luton26 packet handler
 *
 *
 *
 ****************************************************************************/
void rx_packet_tsk (void)
{
    uchar source_port;
    uchar recv_q;

    //println_str("rx_packet_tsk 1");
    for (recv_q = VTSS_PACKET_RX_GROUP_START; recv_q < VTSS_PACKET_RX_GROUP_END; recv_q++) {
        h2_rx_frame_get(recv_q, &vtss_rx_frame);
        //println_str("rx_packet_tsk 2");
        if(!vtss_rx_frame.discard && vtss_rx_frame.total_bytes) {
            source_port = cport2uport(vtss_rx_frame.header.port);

#if defined(H2TXRX_DEBUG_ENABLE)
         dump_frame(vtss_rx_frame.rx_packet ,vtss_rx_frame.total_bytes); 
#endif /* H2TXRX_DEBUG_ENABLE */

#ifndef  VTSS_COMMON_NDEBUG
            vtss_common_dump_frame(vtss_rx_frame.rx_packet ,vtss_rx_frame.total_bytes);
#endif
            switch (BUF->type) {
#if TRANSIT_LLDP
            case HTONS(VTSS_ETHTYPE_LLDP):
                if (VTSS_COMMON_MACADDR_CMP(BUF->dest.addr, mac_addr_lldp) == 0) {
                    TASK(SUB_TASK_ID_LLDP_RX, lldp_frame_received(source_port, vtss_rx_frame.rx_packet, vtss_rx_frame.total_bytes));
                } else {
                    VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_NOISE, ("Dropping on port %u type 0x%x len %u\n",
                                      (unsigned)source_port, (unsigned)BUF->type, (unsigned)rx_packet));
                    h2_discard_frame(&vtss_rx_frame);
                }
                break;
#endif

#if  TRANSIT_LACP
            case HTONS(VTSS_ETHTYPE_SLOW):
                if (VTSS_COMMON_MACADDR_CMP(BUF->dest.addr, vtss_lacp_protocol_addr) == 0) {
                    TASK(SUB_TASK_ID_LLDP_RX, vtss_lacp_receive(source_port, vtss_rx_frame.rx_packet, vtss_rx_frame.total_bytes));
                } else {
                    VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_NOISE, ("Dropping on port %u type 0x%x len %u\n",
                                      (unsigned)source_port, (unsigned)BUF->type, (unsigned)rx_packet));
                    h2_discard_frame(&vtss_rx_frame);
                }
                break;
#endif
            default:
                VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_NOISE, ("Dropping on port %u type 0x%x len %u\n",
                                  (unsigned)source_port, (unsigned)BUF->type, (unsigned)rx_packet));
                h2_discard_frame(&vtss_rx_frame);

            }
        } else {
            ; /* Already discard in function "h2_rx_frame_get" */
        }
    }
}
#endif //#if __BASIC_TX_RX__
