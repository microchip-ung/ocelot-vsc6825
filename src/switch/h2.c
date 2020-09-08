//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT




#include "common.h"     /* Always include common.h at the first place of user-defined herder files */

#include "vtss_common_os.h"
#include "sysutil.h"
#include "vtss_api_base_regs.h"
#include "h2io.h"
#include "h2.h"
#include "timer.h"
#include "main.h"
#include "phydrv.h"
#include "phytsk.h"
#include "phymap.h"
#include "h2flowc.h"
#include "misc1.h"
#include "misc2.h"
#include "hwport.h"
#include "h2mactab.h"
#include "h2vlan.h"
#include "h2packet.h"

#if TRANSIT_LAG
#include "h2aggr.h"
#endif
#if TRANSIT_LACP
#include "vtss_lacp.h"
#endif


#define VTSS_COMMON_ASSERT(EXPR) /* Go away */


#ifndef NDEBUG
#include "txt.h"
#endif /* NDEBUG */
#include "h2sdcfg.h"
#include "print.h"
#include "ledtsk.h"
#if MAC_TO_MEDIA
#include "h2pcs1g.h"
#endif

/*****************************************************************************
 *
 *
 * Defines
 *
 *
 *
 ****************************************************************************/
//#if defined(H2_DEBUG_ENABLE)
#include "print.h"
//#endif /* H2_DEBUG_ENABLE */

/*****************************************************************************
 *
 *
 * Typedefs and enums
 *
 *
 *
 ****************************************************************************/

enum {
    VTSS_PGID_DEST_MASK_START   =   0,
    VTSS_PGID_AGGR_MASK_START   =  64,
    VTSS_PGID_SOURCE_MASK_START =  80
};

/*****************************************************************************
 *
 *
 * Prototypes for local functions
 *
 *
 *
 ****************************************************************************/
#if TRANSIT_LAG
typedef struct {
    port_bit_mask_t h2_current_aggr_groups[TOTAL_AGGRS];
} h2_current_state_t;
#endif 
/*****************************************************************************
 *
 *
 * Local data
 *
 *
 *
 ****************************************************************************/
#if TRANSIT_LAG
static h2_current_state_t   h2_current_state;
#endif

/*****************************************************************************
 *
 *
 * Local functions
 *
 *
 *
 ****************************************************************************/
static void _vtss_update_src_mask(port_bit_mask_t link_mask);
static void _port_reset(vtss_cport_no_t chip_port, uchar link_mode);
static void _setup_mac(vtss_cport_no_t chip_port, uchar link_mode);

#if defined(VTSS_ARCH_LUTON26)
/**
 * Determines ingress forwarding is allowed or not.
 *
 * In unmanaged code base, this function always returns TRUE. In managed code,
 * the return value depends on the result from spanning-tree and 802.1x.
 */
static BOOL _ingr_forwarding (vtss_cport_no_t  sport_no)
{
    sport_no = sport_no; /* avoid compiler warning */
    return TRUE;
}
#endif // LUTON26_L10 || LUTON26_L16 || LUTON26_L25

/**
 * Sets up how to access the switch chip.
 */
static void _h2_setup_cpu_if (void) small
{
}

#if defined(VTSS_ARCH_LUTON26)
static void _l26_buf_conf_set(void)
{
    ulong port_no, q, dp, i = 0;
    ulong buf_q_rsrv_i, buf_q_rsrv_e, ref_q_rsrv_i, ref_q_rsrv_e, buf_prio_shr_i[8], buf_prio_shr_e[8], ref_prio_shr_i[8], ref_prio_shr_e[8];
    ulong buf_p_rsrv_i, buf_p_rsrv_e, ref_p_rsrv_i, ref_p_rsrv_e, buf_col_shr_i, buf_col_shr_e, ref_col_shr_i, ref_col_shr_e;
    ulong prio_mem_rsrv, prio_ref_rsrv, mem, ref, value;

    /*  SYS::RES_CFG : 1024 watermarks for 512 kB shared buffer, unit is 48 byte */
    /*  Is divided into 4 resource consumptions, ingress and egress memory (BUF) and frame reference (REF) blocks */

    /* BUF_xxx_Ingress starts @ offset 0   */
    /* REF_xxx_Ingress starts @ offset 256 */
    /* BUF_xxx_Egress  starts @ offset 512 */
    /* BUF_xxx_Egress  starts @ offset 768 */
    /* xxx = q_rsrv, prio_shr, p_rsrv, col_shr */

    /* Queue reserved (q_rsrv) : starts @ offset 0 within in each BUF and REF */
    /* Prio shared (prio_shr)  : starts @ offset 216 within in each BUF and REF */
    /* Port reserved (p_rsrv)  : starts @ offset 224 within in each BUF and REF */
    /* Colour shared (col_shr) : starts @ offset 254 within in each BUF and REF */

    /* WM values  */
    buf_q_rsrv_i = 10;     /* 500/48 Guarantees reception of at least one frame to all queues  */
    ref_q_rsrv_i = 8;      /* 4 frames can be pending at each ingress port              */
    buf_q_rsrv_e = 4;      /* 200/48 Guarantees all priorities to non-congested traffic stream */
    ref_q_rsrv_e = 8;      /* 4 frames can be pending to each egress port               */

    prio_mem_rsrv = 7000;
    prio_ref_rsrv = 50;

    /* Subtract the reserved amount from the total amount */
    mem = 512000-(MAX_PORT + 1)*(0+10000+8*500+8*200);
    ref = 5500-(MAX_PORT + 1)*(20+20+8*8+8*8);

#ifndef VTSS_PRIOS
#define VTSS_PRIOS 8
#endif

    for (q = 0; q < VTSS_PRIOS; q++) {
        value = (mem-(7-q)*prio_mem_rsrv)/48;
        if (value >= 1024) {
            value = 1024 + value/16;
        }
        buf_prio_shr_i[q] = value;
        ref_prio_shr_i[q] = ref-(7-q)*prio_ref_rsrv;
        value = (mem-(7-q)*prio_mem_rsrv)/48;
        if (value >= 1024) {
            value = 1024 + value/16;
        }
        buf_prio_shr_e[q] = value;
        ref_prio_shr_e[q] = ref-(7-q)*prio_ref_rsrv;
    }

    buf_p_rsrv_i = 0;        /* No quaranteed extra space for ingress ports         */
    ref_p_rsrv_i = 20;       /* 20 extra frames can be pending shared between prios */
    buf_p_rsrv_e = 10000/48; /* 10kB reserved for each egress port                  */
    ref_p_rsrv_e = 20;       /* 20 extra frames can be pending shared between prios */

    buf_col_shr_i = 0x7FF; /* WM max - never reached */
    ref_col_shr_i = 0x7FF; /* WM max - never reached */
    buf_col_shr_e = 0x7FF; /* WM max - never reached */
    ref_col_shr_e = 0x7FF; /* WM max - never reached */

    i = 0;
    do { /* Reset default WM */
        H2_WRITE(VTSS_SYS_RES_CTRL_RES_CFG(i), 0);
        i++;
    } while (i<1024);

    /* Configure reserved space for all QoS classes per port */
    for (port_no = MIN_PORT; port_no <= MAX_PORT; port_no++) {
        for (q = 0; q < VTSS_PRIOS; q++) {
            H2_WRITE(VTSS_SYS_RES_CTRL_RES_CFG(port_no * VTSS_PRIOS + q + 0),   buf_q_rsrv_i);
            H2_WRITE(VTSS_SYS_RES_CTRL_RES_CFG(port_no * VTSS_PRIOS + q + 256), ref_q_rsrv_i);
            H2_WRITE(VTSS_SYS_RES_CTRL_RES_CFG(port_no * VTSS_PRIOS + q + 512), buf_q_rsrv_e);
            H2_WRITE(VTSS_SYS_RES_CTRL_RES_CFG(port_no * VTSS_PRIOS + q + 768), ref_q_rsrv_e);
        }
    }

    /* Configure shared space for all QoS classes */
    for (q = 0; q < VTSS_PRIOS; q++) {
        H2_WRITE(VTSS_SYS_RES_CTRL_RES_CFG((q + 216 + 0)),   buf_prio_shr_i[q]);
        H2_WRITE(VTSS_SYS_RES_CTRL_RES_CFG((q + 216 + 256)), ref_prio_shr_i[q]);
        H2_WRITE(VTSS_SYS_RES_CTRL_RES_CFG((q + 216 + 512)), buf_prio_shr_e[q]);
        H2_WRITE(VTSS_SYS_RES_CTRL_RES_CFG((q + 216 + 768)), ref_prio_shr_e[q]);
        H2_READ(VTSS_SYS_RES_CTRL_RES_CFG((q + 216 + 0)),   dp);
    }

    /* Configure reserved space for all ports */
    for (port_no = MIN_PORT; port_no <= MAX_PORT; port_no++) {
        H2_WRITE(VTSS_SYS_RES_CTRL_RES_CFG(port_no + 224 +   0), buf_p_rsrv_i);
        H2_WRITE(VTSS_SYS_RES_CTRL_RES_CFG(port_no + 224 + 256), ref_p_rsrv_i);
        H2_WRITE(VTSS_SYS_RES_CTRL_RES_CFG(port_no + 224 + 512), buf_p_rsrv_e);
        H2_WRITE(VTSS_SYS_RES_CTRL_RES_CFG(port_no + 224 + 768), ref_p_rsrv_e);
    }

    /* Configure shared space for  both DP levels (green:0 yellow:1) */
    for (dp = 0; dp < 2; dp++) {
        H2_WRITE(VTSS_SYS_RES_CTRL_RES_CFG(dp + 254 +   0), buf_col_shr_i);
        H2_WRITE(VTSS_SYS_RES_CTRL_RES_CFG(dp + 254 + 256), ref_col_shr_i);
        H2_WRITE(VTSS_SYS_RES_CTRL_RES_CFG(dp + 254 + 512), buf_col_shr_e);
        H2_WRITE(VTSS_SYS_RES_CTRL_RES_CFG(dp + 254 + 768), ref_col_shr_e);
    }
    return;
}
#elif defined(VTSS_ARCH_SERVAL_ORG)
#define MULTIPLIER_BIT 256 
static u16 wm_enc(u16 value)
{
    if (value >= MULTIPLIER_BIT) {
        return MULTIPLIER_BIT + value / 16;
    }
    return value;
}

#define FERRET_BUFFER_MEMORY 229380
#define FERRET_BUFFER_REFERENCE 1911
#define FERRET_BUFFER_CELL_SZ 60
#ifndef VTSS_PRIOS
#define VTSS_PRIOS 8
#endif

static void _ferret_buf_conf_set(void)
{
    int q;
    ulong port_no, port, dp;
    ulong buf_q_rsrv_i, buf_q_rsrv_e, ref_q_rsrv_i, ref_q_rsrv_e, buf_prio_shr_i[8], buf_prio_shr_e[8], ref_prio_shr_i[8], ref_prio_shr_e[8];
    ulong buf_p_rsrv_i, buf_p_rsrv_e, ref_p_rsrv_i, ref_p_rsrv_e, buf_col_shr_i, buf_col_shr_e, ref_col_shr_i, ref_col_shr_e;
    ulong buf_prio_rsrv, ref_prio_rsrv, guaranteed, q_rsrv_mask, prio_mem, prio_ref, oversubscription_factor;
    u8 prio_strict,prios=8;

    /* This mode to be configurable by user somehow */
    /* Mode 0: Strict priorities. Higher priorities have right to use all shared before lower */
    /* Mode 1: Classes with equal memory rights */
    ulong shr_mode = 0;

    /* BZ#21592 - 2 to All random frame size (64-9600 bytes).
     *
     * Squeeze a little more out of the buffer resource.
     *
     * In Ferret unmanaged software, only qos 0/7 being in use and
     * qos-7 only has a small amount of memory as a source on the cpu port only
     */
    BOOL is_squeeze_buffer_resource = TRUE;

    /***********************/
    /* Settings for mode 0 */
    /***********************/
    /* SYS::RES_CFG : 1024 watermarks for 1024000 byte shared buffer, unit is 60 byte */
    /* Is divided into 4 resource consumptions, ingress and egress memory (BUF) and ingress and egress frame reference (REF) blocks */
    /* Queue reserved (q_rsrv) : starts at offset 0 within in each BUF and REF   */
    /* Prio shared (prio_shr)  : starts at offset 216 within in each BUF and REF */
    /* Port reserved (p_rsrv)  : starts at offset 224 within in each BUF and REF */
    /* Colour shared (col_shr) : starts at offset 254 within in each BUF and REF */

    /* Buffer values are in BYTES */
    buf_q_rsrv_i = 3000;    /* Guarantees at least 1 MTU  */
    buf_p_rsrv_i = 0;       /* No additional ingress guarantees   */
    buf_q_rsrv_e = 0;       /* Guarantees all QoS classes some space */
    buf_p_rsrv_e = 3000;    /* Guarantees a space to the egress ports */
    buf_col_shr_i = FERRET_BUFFER_MEMORY; /* Green color - disabled for now */
    buf_col_shr_e = FERRET_BUFFER_MEMORY; /* Green color - disabled for now */
    buf_prio_rsrv = 4000;  /* In the shared area, each priority is cut off 12kB before the others. Yellow colour is cut of 12kb before lowest */
    prio_strict   = TRUE;   /* The priorities are treated strict in the shared memory */

    /* Reference values in NUMBER of FRAMES */
    ref_q_rsrv_e = 8;     /* Number of frames that can be pending at each egress queue   */
    ref_q_rsrv_i = 8;     /* Number of frames that can be pending at each ingress queue  */
    ref_p_rsrv_e = 20;    /* Number of frames that can be pending shared between the QoS classes at egress */
    ref_p_rsrv_i = 20;     /* Number of frames that can be pending shared between the QoS classes at ingress */
    ref_col_shr_i = FERRET_BUFFER_REFERENCE; /* Green color - disabled for now */
    ref_col_shr_e = FERRET_BUFFER_REFERENCE; /* Green color - disabled for now */
    ref_prio_rsrv = 50;   /* Number of frames that can be pending for each class */

    /* The memory is oversubsrcribed by this factor (factor 1 = 100) */
    /* Oversubscription is possible (in some degree) because it's rare that all ports use their reserved space at the same time */
    oversubscription_factor = 200; /* No oversubscription */


    /****** User configured mode changes *************/

    /************/
    /* Mode 1   */
    /************/
    if (shr_mode == 1) {
        prio_strict     = FALSE; /* Each priority has its own share */
        buf_prio_rsrv   = 30000; /* 30kB set aside to each active priority */
        buf_p_rsrv_i    = 0;     /* No port reservation allowed in this mode */
        buf_p_rsrv_e    = 0;
        ref_prio_rsrv   = 200;  /* 200 frames set aside to each active priority */
        ref_p_rsrv_i    = 0;    /* No port reservation allowed in this mode */
        ref_p_rsrv_e    = 0;
        oversubscription_factor = 200; /* Assume only half of the reserved memory is in use at any time */
    }
    /*************************************************/


    /* Note, the shared reserved space (buf_prio_shr_i, ref_prio_shr_i, buf_prio_shr_e, ref_prio_shr_e) is calulated based on above */

    /* The number of supported queues is given through the state structure                           */
    /* The supported queues (lowest to higest) are givin reserved buffer space as specified above.   */
    /* Frames in remaining queues (if any) are not getting any reserved space - but are allowed in the system.*/
    if (is_squeeze_buffer_resource) {
        q_rsrv_mask = 0x1 | (1 << PACKET_XTR_QU_BPDU_LLDP);
    } else {
        q_rsrv_mask = 0xff >> (8 - prios);
    }


    /* **************************************************  */
    /* BELOW, everything is calculated based on the above. */
    /* **************************************************  */

    /* Find the amount of guaranteeed space per port */
    guaranteed = buf_p_rsrv_i+buf_p_rsrv_e;
    for (q=0; q<VTSS_PRIOS; q++) {
        if (q_rsrv_mask & (1<<q))
            guaranteed+=(buf_q_rsrv_i+buf_q_rsrv_e);
    }

    prio_mem = FERRET_BUFFER_MEMORY - (MAX_PORT+1)*guaranteed*100/oversubscription_factor;

    /* Find the amount of guaranteeed frame references */
    guaranteed = ref_p_rsrv_i+ref_p_rsrv_e;
    for (q=0; q<VTSS_PRIOS; q++) {
        if (q_rsrv_mask & (1<<q)) {
            guaranteed+=(ref_q_rsrv_i+ref_q_rsrv_e);
        }
    }
    prio_ref = FERRET_BUFFER_REFERENCE - (MAX_PORT+1)*guaranteed*100/oversubscription_factor;

    /* Configuring the prio watermarks */
    if (prio_strict) {
        for (q = prios-1; q>=0; q--) {
            if (q_rsrv_mask & (1<<q)) {
                buf_prio_shr_i[q] = prio_mem;
                ref_prio_shr_i[q] = prio_ref;
                buf_prio_shr_e[q] = prio_mem;
                ref_prio_shr_e[q] = prio_ref;

                prio_mem -= buf_prio_rsrv;
                prio_ref -= ref_prio_rsrv;
            } else {
                buf_prio_shr_i[q] = 0;
                ref_prio_shr_i[q] = 0;
                buf_prio_shr_e[q] = 0;
                ref_prio_shr_e[q] = 0;
            }
        }
        buf_col_shr_i = prio_mem - buf_prio_rsrv;
        buf_col_shr_e = prio_mem - buf_prio_rsrv;
        ref_col_shr_i = prio_ref - ref_prio_rsrv;
        ref_col_shr_e = prio_ref - ref_prio_rsrv;
    } else {
        /* Shared area per prio */
        for (q=0; q<prios; q++) {
            buf_prio_shr_i[q] = buf_prio_rsrv;
            ref_prio_shr_i[q] = ref_prio_rsrv;
            buf_prio_shr_e[q] = buf_prio_rsrv;
            ref_prio_shr_e[q] = ref_prio_rsrv;
            prio_mem -= buf_prio_rsrv;
            prio_ref -= ref_prio_rsrv;
        }
        buf_col_shr_i = prio_mem;
        buf_col_shr_e = prio_mem;
        ref_col_shr_i = prio_ref;
        ref_col_shr_e = prio_ref;
        /* Use per priority shared areas */
        H2_WRITE(VTSS_QSYS_RES_QOS_ADV_RES_QOS_MODE, 0xff);
    }


    /* Port and QoS class reservation watermarks (Q_RSRV):
       Configure reserved space for all QoS classes per port */
    for (port_no = 0; port_no <= MAX_PORT; port_no++) {
        if (port_no == MAX_PORT) {
            port = CPU_CHIP_PORT;
        } else {
            port = iport2cport(port_no);
        }
        for (q = 0; q < VTSS_PRIOS; q++) {
            if (q_rsrv_mask&(1<<q)
                /* && (is_squeeze_buffer_resource && (q==0 || q==7 && port==CPU_CHIP_PORT)) */) {
                H2_WRITE(VTSS_QSYS_RES_CTRL_RES_CFG(port * VTSS_PRIOS + q + 0),   wm_enc(buf_q_rsrv_i / FERRET_BUFFER_CELL_SZ));
                H2_WRITE(VTSS_QSYS_RES_CTRL_RES_CFG(port * VTSS_PRIOS + q + 256), wm_enc(ref_q_rsrv_i));
                H2_WRITE(VTSS_QSYS_RES_CTRL_RES_CFG(port * VTSS_PRIOS + q + 512), wm_enc(buf_q_rsrv_e / FERRET_BUFFER_CELL_SZ));
                H2_WRITE(VTSS_QSYS_RES_CTRL_RES_CFG(port * VTSS_PRIOS + q + 768), wm_enc(ref_q_rsrv_e));
            } else {
                H2_WRITE(VTSS_QSYS_RES_CTRL_RES_CFG(port * VTSS_PRIOS + q + 0),   0);
                H2_WRITE(VTSS_QSYS_RES_CTRL_RES_CFG(port * VTSS_PRIOS + q + 256), 0);
                H2_WRITE(VTSS_QSYS_RES_CTRL_RES_CFG(port * VTSS_PRIOS + q + 512), 0);
                H2_WRITE(VTSS_QSYS_RES_CTRL_RES_CFG(port * VTSS_PRIOS + q + 768), 0);
            }
        }
    }

    /* QoS class sharing watermarks (PRIO_SHR):
       Configure shared space for all QoS classes */
    for (q = 0; q < VTSS_PRIOS; q++) {
        /* Save initial encoded value of shared area for later use by WRED */
        //vtss_state->port.buf_prio_shr[q] = wm_enc(buf_prio_shr_e[q] / FERRET_BUFFER_CELL_SZ);

        H2_WRITE(VTSS_QSYS_RES_CTRL_RES_CFG((q + 216 + 0)),   wm_enc(buf_prio_shr_i[q] / FERRET_BUFFER_CELL_SZ));
        H2_WRITE(VTSS_QSYS_RES_CTRL_RES_CFG((q + 216 + 256)), wm_enc(ref_prio_shr_i[q]));
        H2_WRITE(VTSS_QSYS_RES_CTRL_RES_CFG((q + 216 + 512)), wm_enc(buf_prio_shr_e[q] / FERRET_BUFFER_CELL_SZ));
        H2_WRITE(VTSS_QSYS_RES_CTRL_RES_CFG((q + 216 + 768)), wm_enc(ref_prio_shr_e[q]));
    }

    /* Port reservation watermarks (P_RSRV):
       Configure reserved space for all ports */
    for (port_no = 0; port_no <= MAX_PORT; port_no++) {
        if (port_no == MAX_PORT) {
            port = CPU_CHIP_PORT;
        } else {
            port = iport2cport(port_no);
        }
        H2_WRITE(VTSS_QSYS_RES_CTRL_RES_CFG(port + 224 +   0), wm_enc(buf_p_rsrv_i / FERRET_BUFFER_CELL_SZ));
        H2_WRITE(VTSS_QSYS_RES_CTRL_RES_CFG(port + 224 + 256), wm_enc(ref_p_rsrv_i));
        H2_WRITE(VTSS_QSYS_RES_CTRL_RES_CFG(port + 224 + 512), wm_enc(buf_p_rsrv_e / FERRET_BUFFER_CELL_SZ));
        H2_WRITE(VTSS_QSYS_RES_CTRL_RES_CFG(port + 224 + 768), wm_enc(ref_p_rsrv_e));
    }

    /* Color sharing watermarks (COL_SHR):
       Configure shared space for both DP levels.
       In this context dp:0 is yellow and dp:1 is green */
    for (dp = 0; dp < 2; dp++) {
        H2_WRITE(VTSS_QSYS_RES_CTRL_RES_CFG(dp + 254 +   0), wm_enc(buf_col_shr_i / FERRET_BUFFER_CELL_SZ));
        H2_WRITE(VTSS_QSYS_RES_CTRL_RES_CFG(dp + 254 + 256), wm_enc(ref_col_shr_i));
        H2_WRITE(VTSS_QSYS_RES_CTRL_RES_CFG(dp + 254 + 512), wm_enc(buf_col_shr_e / FERRET_BUFFER_CELL_SZ));
        H2_WRITE(VTSS_QSYS_RES_CTRL_RES_CFG(dp + 254 + 768), wm_enc(ref_col_shr_e));
        // Green watermark should match the highest priority watermark. Otherwise the will be no
        //// strict shared space per qos level
        buf_col_shr_i=buf_col_shr_e=buf_prio_shr_i[7];
        ref_col_shr_i=ref_col_shr_e=ref_prio_shr_i[7];
    }
}
#endif //#if defined(VTSS_ARCH_LUTON26)

#if defined(VTSS_ARCH_OCELOT)
static void _ferret_port_reset(vtss_cport_no_t chip_port, uchar link_mode)
{
    /* Refer to Ferret chip specification: Port Reset Procedure
     * When changing a switch port’s mode of operation or restarting a switch port, the following port reset
     * procedure must be followed:
     *
     * 1. Disable the MAC frame reception in the switch port.
     *      DEV::MAC_ENA_CFG.RX_ENA = 0
     * 2. Disable traffic being sent to or from the switch port.
     *      QSYS:PORT:SWITCH_PORT_MODE_ENA = 0
     * 3. Disable shaping to speed up flushing of frames
     *      QSYS::PORT_MODE.DEQUEUE_DIS = 1
     * 4. Wait at least the time it takes to receive a frame of maximum length at the port.
     *      Worst-case delays for 10 kilobyte jumbo frames are:
     *      8 ms on a 10M port
     *      800 μs on a 100M port
     *      80 μs on a 1G port
     *      32 μs on a 2.5G port
     * 5. Disable HDX backpressure.
     *      SYS::FRONT_PORT_MODE.HDX_MODE = 0
     * 6. Flush the queues associated with the port.
     *      REW:PORT:PORT_CFG.FLUSH_ENA = 1
     * 7. Enable dequeuing from the egress queues.
     *      QSYS::PORT_MODE.DEQUEUE_DIS = 0
     * 8. Wait until flushing is complete.
     *      QSYS:PORT:SW_STATUS.EQ_AVAIL must return 0
     * 9. Reset the switch port by setting the following reset bits in CLOCK_CFG:
     *      DEV::CLOCK_CFG.MAC_TX_RST = 1
     *      DEV::CLOCK_CFG.MAC_RX_RST = 1
     *      DEV::CLOCK_CFG.PORT_RST = 1
     * 10. Clear flushing again.
     *      REW:PORT:PORT_CFG.FLUSH_ENA = 0
     * 11. Set up the switch port to the new mode of operation. Keep the reset bits in CLOCK_CFG set.
     * 12. Release the switch port from reset by clearing the reset bits in CLOCK_CFG.
     */

    ulong reg_val, retry_cnt = 10000;

    /* Select the link speed. Refer to chip sepecification: VTSS_DEV_PORT_MODE_CLOCK_CFG . LINK_SPEED
     * 0: No link
     * 1: 1000/2500 Mbps
     * 2: 100 Mbps
     * 3: 10 Mbps
     */
    u8 speed = (link_mode & LINK_MODE_SPEED_MASK);
    u8 reg_speed_val = (speed == LINK_MODE_SPEED_1000 || speed == LINK_MODE_SPEED_2500) ? 1 :
                       (speed == LINK_MODE_SPEED_100) ? 2 :
                       (speed == LINK_MODE_SPEED_10) ? 3 : 0;

    /* 1. Disable the MAC frame reception in the switch port.
     *      DEV::MAC_ENA_CFG.RX_ENA = 0
     */
    H2_WRITE_MASKED(VTSS_DEV_MAC_CFG_STATUS_MAC_ENA_CFG(VTSS_TO_DEV(chip_port)),
                    VTSS_F_DEV_MAC_CFG_STATUS_MAC_ENA_CFG_RX_ENA(0),
                    VTSS_M_DEV_MAC_CFG_STATUS_MAC_ENA_CFG_RX_ENA);

    /* 2. Disable traffic being sent to or from the switch port.
     *      QSYS:PORT:SWITCH_PORT_MODE_ENA = 0
     */
    H2_WRITE_MASKED(VTSS_QSYS_SYSTEM_SWITCH_PORT_MODE(chip_port),
                    VTSS_F_QSYS_SYSTEM_SWITCH_PORT_MODE_PORT_ENA(0),
                    VTSS_M_QSYS_SYSTEM_SWITCH_PORT_MODE_PORT_ENA);

    /* 3. Disable shaping to speed up flushing of frames
     *      QSYS::PORT_MODE.DEQUEUE_DIS = 1
     */
    H2_WRITE_MASKED(VTSS_QSYS_SYSTEM_PORT_MODE(chip_port),
                    VTSS_F_QSYS_SYSTEM_PORT_MODE_DEQUEUE_DIS(1),
                    VTSS_M_QSYS_SYSTEM_PORT_MODE_DEQUEUE_DIS);

    /* 4. Wait at least the time it takes to receive a frame of maximum length at the port.
    /*      Worst-case delays for 10 kilobyte jumbo frames are:
     *      8 ms on a 10M port
     *      800 μs on a 100M port
     *      80 μs on a 1G port
     *      32 μs on a 2.5G port
     */
    delay_1(10);

    /* 5. Disable HDX backpressure.
     *      SYS::FRONT_PORT_MODE.HDX_MODE = 0
     */
    H2_WRITE_MASKED(VTSS_SYS_SYSTEM_FRONT_PORT_MODE(chip_port),
                    VTSS_F_SYS_SYSTEM_FRONT_PORT_MODE_HDX_MODE(0),
                    VTSS_M_SYS_SYSTEM_FRONT_PORT_MODE_HDX_MODE);

    /* 6. Flush the queues associated with the port.
     *      REW:PORT:PORT_CFG.FLUSH_ENA = 1
     */
    H2_WRITE_MASKED(VTSS_REW_PORT_PORT_CFG(chip_port),
                    VTSS_F_REW_PORT_PORT_CFG_FLUSH_ENA(1),
                    VTSS_M_REW_PORT_PORT_CFG_FLUSH_ENA);

    /* 6a. Disable flow control */
    H2_WRITE(VTSS_SYS_PAUSE_CFG_PAUSE_CFG(chip_port),
             VTSS_F_SYS_PAUSE_CFG_PAUSE_CFG_PAUSE_STOP(0x1ff));
    H2_WRITE_MASKED(VTSS_SYS_PAUSE_CFG_MAC_FC_CFG(chip_port),
                    VTSS_F_SYS_PAUSE_CFG_MAC_FC_CFG_TX_FC_ENA(0),
                    VTSS_M_SYS_PAUSE_CFG_MAC_FC_CFG_TX_FC_ENA);

    /* 7. Enable dequeuing from the egress queues.
     *      QSYS::PORT_MODE.DEQUEUE_DIS = 0
     */
    H2_WRITE_MASKED(VTSS_QSYS_SYSTEM_PORT_MODE(chip_port),
                    VTSS_F_QSYS_SYSTEM_PORT_MODE_DEQUEUE_DIS(0),
                    VTSS_M_QSYS_SYSTEM_PORT_MODE_DEQUEUE_DIS);

    /* 8. Wait until flushing is complete.
    *      QSYS:PORT:SW_STATUS.EQ_AVAIL must return 0
     */
    do {
        delay_1(1);
        H2_READ(VTSS_QSYS_SYSTEM_SW_STATUS(chip_port), reg_val);
        retry_cnt--;
    } while ((reg_val & VTSS_M_QSYS_SYSTEM_SW_STATUS_EQ_AVAIL) && retry_cnt);

//#if defined(H2_DEBUG_ENABLE)
    if (!retry_cnt) {
        print_cr_lf();
        println_str("%% Port Reset - Flush timeout chip_port = ");
        print_dec(chip_port);
        print_cr_lf();
    }
//#endif /* H2_DEBUG_ENABLE */

    /* 9. Reset the switch port by setting the following reset bits in CLOCK_CFG:
     *      DEV::CLOCK_CFG.MAC_TX_RST = 1
     *      DEV::CLOCK_CFG.MAC_RX_RST = 1
     *      DEV::CLOCK_CFG.PORT_RST = 1
     */
    H2_WRITE_MASKED(VTSS_DEV_PORT_MODE_CLOCK_CFG(VTSS_TO_DEV(chip_port)),
                    VTSS_F_DEV_PORT_MODE_CLOCK_CFG_MAC_TX_RST(1) |
                    VTSS_F_DEV_PORT_MODE_CLOCK_CFG_MAC_RX_RST(1) |
                    VTSS_F_DEV_PORT_MODE_CLOCK_CFG_PORT_RST(1),
                    VTSS_M_DEV_PORT_MODE_CLOCK_CFG_MAC_TX_RST |
                    VTSS_M_DEV_PORT_MODE_CLOCK_CFG_MAC_RX_RST |
                    VTSS_M_DEV_PORT_MODE_CLOCK_CFG_PORT_RST);

    /* 10. Clear flushing again.
     *      REW:PORT:PORT_CFG.FLUSH_ENA = 0
     */
    H2_WRITE_MASKED(VTSS_REW_PORT_PORT_CFG(chip_port),
                    VTSS_F_REW_PORT_PORT_CFG_FLUSH_ENA(0),
                    VTSS_M_REW_PORT_PORT_CFG_FLUSH_ENA);

    /* 11. Set up the switch port to the new mode of operation. Keep the reset bits in CLOCK_CFG set.
     */
    _setup_mac(chip_port, link_mode);

    // New mode of duplex
    H2_WRITE_MASKED(VTSS_SYS_SYSTEM_FRONT_PORT_MODE(chip_port),
                    VTSS_F_SYS_SYSTEM_FRONT_PORT_MODE_HDX_MODE((link_mode & LINK_MODE_FDX_MASK) ? 0 : 1),
                    VTSS_M_SYS_SYSTEM_FRONT_PORT_MODE_HDX_MODE);
    // New mode of speed
    H2_WRITE_MASKED(VTSS_DEV_PORT_MODE_CLOCK_CFG(VTSS_TO_DEV(chip_port)),
                    VTSS_F_DEV_PORT_MODE_CLOCK_CFG_LINK_SPEED(reg_speed_val),
                    VTSS_M_DEV_PORT_MODE_CLOCK_CFG_LINK_SPEED);

    /* 12. Release the switch port from reset by clearing the reset bits in CLOCK_CFG. */
    // EA = 0; // Disable interrupt while doing the clock reset.
    H2_WRITE_MASKED(VTSS_DEV_PORT_MODE_CLOCK_CFG(VTSS_TO_DEV(chip_port)),
                    VTSS_F_DEV_PORT_MODE_CLOCK_CFG_MAC_TX_RST(0) |
                    VTSS_F_DEV_PORT_MODE_CLOCK_CFG_MAC_RX_RST(0) |
                    VTSS_F_DEV_PORT_MODE_CLOCK_CFG_PORT_RST(0),
                    VTSS_M_DEV_PORT_MODE_CLOCK_CFG_MAC_TX_RST |
                    VTSS_M_DEV_PORT_MODE_CLOCK_CFG_MAC_RX_RST |
                    VTSS_M_DEV_PORT_MODE_CLOCK_CFG_PORT_RST);
    // EA = 1; // Enable interrupt
    delay_1(1); // Small delay after clock reset
}
#endif // FERRET

static void _setup_port (vtss_cport_no_t chip_port, uchar link_mode)
{
    /* Select the link speed. Refer to chip sepecification: VTSS_DEV_PORT_MODE_CLOCK_CFG . LINK_SPEED
     * 0: No link
     * 1: 1000/2500 Mbps
     * 2: 100 Mbps
     * 3: 10 Mbps
     */
    u8 speed = (link_mode & LINK_MODE_SPEED_MASK);
    u8 reg_speed_val = (speed == LINK_MODE_SPEED_1000 || speed == LINK_MODE_SPEED_2500) ? 1 :
                       (speed == LINK_MODE_SPEED_100) ? 2 :
                       (speed == LINK_MODE_SPEED_10) ? 3 : 0;

    if (link_mode != LINK_MODE_DOWN) {
        /* Set up flow control */
        h2_setup_flow_control(chip_port, link_mode);

        h2_enable_exc_col_drop(chip_port, 0);

        /* Core: Enable port for frame transfer */
#if defined(VTSS_ARCH_LUTON26)
        H2_WRITE_MASKED(VTSS_SYS_SYSTEM_SWITCH_PORT_MODE(chip_port),
                        VTSS_F_SYS_SYSTEM_SWITCH_PORT_MODE_PORT_ENA,
                        VTSS_F_SYS_SYSTEM_SWITCH_PORT_MODE_PORT_ENA);
#elif defined(VTSS_ARCH_OCELOT)
        H2_WRITE_MASKED(VTSS_QSYS_SYSTEM_SWITCH_PORT_MODE(chip_port),
                        VTSS_F_QSYS_SYSTEM_SWITCH_PORT_MODE_PORT_ENA(1),
                        VTSS_M_QSYS_SYSTEM_SWITCH_PORT_MODE_PORT_ENA);
#endif

        /* Core: Enable/disable system HDX */
#if defined(VTSS_ARCH_LUTON26)
        H2_WRITE_MASKED(VTSS_SYS_SYSTEM_FRONT_PORT_MODE(chip_port),
                        ((link_mode & LINK_MODE_FDX_MASK) ? 0UL : VTSS_F_SYS_SYSTEM_FRONT_PORT_MODE_HDX_MODE),
                        VTSS_F_SYS_SYSTEM_FRONT_PORT_MODE_HDX_MODE);
#elif defined(VTSS_ARCH_OCELOT)
        H2_WRITE_MASKED(VTSS_SYS_SYSTEM_FRONT_PORT_MODE(chip_port),
                        VTSS_F_SYS_SYSTEM_FRONT_PORT_MODE_HDX_MODE((link_mode & LINK_MODE_FDX_MASK) ? 0 : 1),
                        VTSS_M_SYS_SYSTEM_FRONT_PORT_MODE_HDX_MODE);
#endif

        /* Take MAC, Port, Phy (intern) and PCS (SGMII/Serdes) clock out of reset */
        /* Since there is no GMII in Ferret only port_mode_clock_config used Sumit */
#if defined(VTSS_ARCH_LUTON26)
        if (chip_port > 9) {
            H2_WRITE(VTSS_DEV_PORT_MODE_CLOCK_CFG(VTSS_TO_DEV(chip_port)),
                     VTSS_F_DEV_PORT_MODE_CLOCK_CFG_LINK_SPEED(3UL-(link_mode & LINK_MODE_SPEED_MASK)));
        } else {
            H2_WRITE(VTSS_DEV_GMII_PORT_MODE_CLOCK_CFG(VTSS_TO_DEV(chip_port)),0UL);
        }
#elif defined(VTSS_ARCH_OCELOT)
        H2_WRITE_MASKED(VTSS_DEV_PORT_MODE_CLOCK_CFG(VTSS_TO_DEV(chip_port)),
                        VTSS_F_DEV_PORT_MODE_CLOCK_CFG_LINK_SPEED(reg_speed_val),
                        VTSS_M_DEV_PORT_MODE_CLOCK_CFG_LINK_SPEED);
#endif

        /* Enable MAC TX/RX domain */
#if defined(VTSS_ARCH_LUTON26)
        H2_WRITE(VTSS_DEV_CMN_MAC_CFG_STATUS_MAC_ENA_CFG(VTSS_TO_DEV(chip_port)),
                 VTSS_F_DEV_MAC_CFG_STATUS_MAC_ENA_CFG_RX_ENA |
                 VTSS_F_DEV_MAC_CFG_STATUS_MAC_ENA_CFG_TX_ENA);
#elif defined(VTSS_ARCH_OCELOT)
        H2_WRITE_MASKED(VTSS_DEV_MAC_CFG_STATUS_MAC_ENA_CFG(VTSS_TO_DEV(chip_port)),
                        VTSS_F_DEV_MAC_CFG_STATUS_MAC_ENA_CFG_RX_ENA(1) |
                        VTSS_F_DEV_MAC_CFG_STATUS_MAC_ENA_CFG_TX_ENA(1),
                        VTSS_M_DEV_MAC_CFG_STATUS_MAC_ENA_CFG_RX_ENA |
                        VTSS_M_DEV_MAC_CFG_STATUS_MAC_ENA_CFG_TX_ENA);
#endif

    } else { // Link-down
#if defined(VTSS_ARCH_OCELOT)
        // Bugzilla#21274, reset port to flush queue again when a port is link-down.
        _ferret_port_reset(chip_port, link_mode);

#elif defined(VTSS_ARCH_LUTON26)
        if (chip_port < 10) {
            H2_WRITE(VTSS_DEV_GMII_PORT_MODE_CLOCK_CFG(VTSS_TO_DEV(chip_port)),0xe);
        } else if (chip_port < 12) {
            H2_WRITE(VTSS_DEV_PORT_MODE_CLOCK_CFG(VTSS_TO_DEV(chip_port)),
                     VTSS_F_DEV_PORT_MODE_CLOCK_CFG_MAC_TX_RST |
                     VTSS_F_DEV_PORT_MODE_CLOCK_CFG_MAC_RX_RST |
                     VTSS_F_DEV_PORT_MODE_CLOCK_CFG_PCS_TX_RST |
                     VTSS_F_DEV_PORT_MODE_CLOCK_CFG_PCS_RX_RST |
                     VTSS_F_DEV_PORT_MODE_CLOCK_CFG_PORT_RST);
        } else {
            /* This register controls reset of different blocks in the device...
             when a *_RST field is set e1f the corresponding block is kept in reset */
            H2_WRITE(VTSS_DEV_PORT_MODE_CLOCK_CFG(VTSS_TO_DEV(chip_port)), 0x1);
        }
#endif
    }

    // Update masks
    VTSS_UPDATE_MASKS_DEBUG();
    vtss_update_masks();
}


static void _setup_pcs(vtss_cport_no_t chip_port, uchar link_mode)
{
    uchar sd_internal, sd_active_high, sd_enable, if_100fx, if_sgmii, if_type;
    ulong value;

    link_mode = link_mode; /* Happy compilier for future usage */
    if_type = phy_map_miim_no(chip_port);

#if defined(VTSS_ARCH_LUTON26)
    /* Fixme: should use a MARCO configuration for user to define instead of static? */
    sd_internal     = TRUE;
    sd_active_high  = TRUE;
    sd_enable       = FALSE;

#elif defined(VTSS_ARCH_OCELOT)
    if (phy_map_serdes(chip_port)) {
        sd_internal     = TRUE;
        sd_active_high  = TRUE;
        sd_enable   = TRUE;
    } else if (if_type == MAC_IF_EXTERNAL) { // External PHY
        sd_internal     = TRUE;
        sd_active_high  = FALSE;
        sd_enable       = FALSE;
    }
#endif

    /* see define for CPORT_MAPTO_PHYMIIMBUS */
    if (if_type == MAC_IF_100FX) {
        if_100fx = TRUE;
        if_sgmii = FALSE;
    } else if (if_type == MAC_IF_SGMII || if_type == MAC_IF_INTERNAL
#if defined(VTSS_ARCH_LUTON26)
               || if_type == MAC_IF_EXTERNAL
#elif defined(VTSS_ARCH_OCELOT)
    // Ferret external PHY uses QSGMII interface
#endif // LUTON26_L10 || LUTON26_L16 || LUTON26_L25
               ) {
        if_100fx = FALSE;
        if_sgmii = TRUE;
    } else {
        if_100fx = FALSE;
        if_sgmii = FALSE;
    }

    /* PSC settings for 100fx/SGMII/SERDES */
    if (if_100fx) {
        /* 100FX PCS */
#if defined(VTSS_ARCH_LUTON26)
        value = VTSS_F_DEV_PCS_FX100_CONFIGURATION_PCS_FX100_CFG_PCS_ENA |
                (sd_internal    ? 0 : VTSS_F_DEV_PCS_FX100_CONFIGURATION_PCS_FX100_CFG_SD_SEL) |
                (sd_active_high ? 1<<25 : 0)  |  /* VTSS_F_DEV_PCS_FX100_CONFIGURATION_PCS_FX100_SD_POL [DBG] */
                (sd_enable      ? VTSS_F_DEV_PCS_FX100_CONFIGURATION_PCS_FX100_CFG_SD_ENA : 0);
        H2_WRITE_MASKED(VTSS_DEV_PCS_FX100_CONFIGURATION_PCS_FX100_CFG(VTSS_TO_DEV(chip_port)),value,
                        VTSS_F_DEV_PCS_FX100_CONFIGURATION_PCS_FX100_CFG_PCS_ENA |
                        VTSS_F_DEV_PCS_FX100_CONFIGURATION_PCS_FX100_CFG_SD_SEL |
                        (1<<25) | /* VTSS_F_DEV_PCS_FX100_CONFIGURATION_PCS_FX100_CFG_SD_POL [DBG] */
                        VTSS_F_DEV_PCS_FX100_CONFIGURATION_PCS_FX100_CFG_SD_ENA);

#elif defined(VTSS_ARCH_OCELOT)
        value = VTSS_F_DEV_PCS_FX100_CONFIGURATION_PCS_FX100_CFG_PCS_ENA(1) |
                VTSS_F_DEV_PCS_FX100_CONFIGURATION_PCS_FX100_CFG_SD_SEL(!sd_internal) |
                VTSS_F_DEV_PCS_FX100_CONFIGURATION_PCS_FX100_CFG_SD_POL(sd_active_high) |
                VTSS_F_DEV_PCS_FX100_CONFIGURATION_PCS_FX100_CFG_SD_ENA(sd_enable);
        H2_WRITE_MASKED(VTSS_DEV_PCS_FX100_CONFIGURATION_PCS_FX100_CFG(VTSS_TO_DEV(chip_port)),
                        value,
                        VTSS_M_DEV_PCS_FX100_CONFIGURATION_PCS_FX100_CFG_PCS_ENA |
                        VTSS_M_DEV_PCS_FX100_CONFIGURATION_PCS_FX100_CFG_SD_SEL |
                        VTSS_M_DEV_PCS_FX100_CONFIGURATION_PCS_FX100_CFG_SD_POL |
                        VTSS_M_DEV_PCS_FX100_CONFIGURATION_PCS_FX100_CFG_SD_ENA);
#endif

    } else { // SGMII or SERDES mode
#if defined(VTSS_ARCH_LUTON26)
        /* Choose SGMII or Serdes PCS mode */
        H2_WRITE(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_MODE_CFG(VTSS_TO_DEV(chip_port)),
                 (if_sgmii ? VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_MODE_CFG_SGMII_MODE_ENA : 0));
        H2_WRITE(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG(VTSS_TO_DEV(chip_port)),
                 (if_sgmii ? VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG_SW_RESOLVE_ENA : 0));
        H2_WRITE(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_SD_CFG(VTSS_TO_DEV(chip_port)),
                 (sd_active_high ? VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_SD_CFG_SD_POL : 0) |
                 (sd_enable ? VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_SD_CFG_SD_ENA : 0) |
                 (sd_internal ? 0 : VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_SD_CFG_SD_SEL));

#elif defined(VTSS_ARCH_OCELOT)
        /* Choose SGMII or Serdes PCS mode
         */
        H2_WRITE_MASKED(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_MODE_CFG(VTSS_TO_DEV(chip_port)),
                        VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_MODE_CFG_SGMII_MODE_ENA(if_sgmii),
                        VTSS_M_DEV_PCS1G_CFG_STATUS_PCS1G_MODE_CFG_SGMII_MODE_ENA);

        /* Software Resolve Abilities
         */
        H2_WRITE_MASKED(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG(VTSS_TO_DEV(chip_port)),
                        VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG_SW_RESOLVE_ENA(if_sgmii),
                        VTSS_M_DEV_PCS1G_CFG_STATUS_PCS1G_ANEG_CFG_SW_RESOLVE_ENA);

        /* Disable Signal Detect, when input pin is '0' to indicate a signal detection.
         */
        H2_WRITE_MASKED(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_SD_CFG(VTSS_TO_DEV(chip_port)),
                        VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_SD_CFG_SD_POL(sd_active_high) |
                        VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_SD_CFG_SD_ENA(sd_enable) |
                        VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_SD_CFG_SD_SEL(!sd_internal),
                        VTSS_M_DEV_PCS1G_CFG_STATUS_PCS1G_SD_CFG_SD_POL |
                        VTSS_M_DEV_PCS1G_CFG_STATUS_PCS1G_SD_CFG_SD_ENA |
                        VTSS_M_DEV_PCS1G_CFG_STATUS_PCS1G_SD_CFG_SD_SEL);
#endif

        /* Enable PCS */
#if defined(VTSS_ARCH_LUTON26)
        H2_WRITE(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_CFG(VTSS_TO_DEV(chip_port)),
                 VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_CFG_PCS_ENA);

#elif defined(VTSS_ARCH_OCELOT)
        h2_pcs1g_clock_set(chip_port, TRUE);    // Enable PCS clock
        H2_WRITE_MASKED(VTSS_DEV_PCS1G_CFG_STATUS_PCS1G_CFG(VTSS_TO_DEV(chip_port)),
                        VTSS_F_DEV_PCS1G_CFG_STATUS_PCS1G_CFG_PCS_ENA(1),
                        VTSS_M_DEV_PCS1G_CFG_STATUS_PCS1G_CFG_PCS_ENA);
#endif
    }
}

static void _setup_mac(vtss_cport_no_t chip_port, uchar link_mode)
{
    u32 fdx_gap, hdx_gap_1, hdx_gap_2, value = 0;
    u8  link_spd_dpx;

    if (link_mode != LINK_MODE_DOWN) {
        link_spd_dpx = link_mode & LINK_MODE_SPEED_AND_FDX_MASK;

#if defined(VTSS_ARCH_LUTON26)
        /* Bugzilla 4388: disabling frame aging when in HDX */
        H2_WRITE_MASKED(VTSS_REW_PORT_PORT_CFG(chip_port),
                        ((link_mode & LINK_MODE_FDX_MASK) ? 0 : VTSS_F_REW_PORT_PORT_CFG_AGE_DIS),
                        VTSS_F_REW_PORT_PORT_CFG_AGE_DIS);
#elif defined(VTSS_ARCH_OCELOT)
        H2_WRITE_MASKED(VTSS_REW_PORT_PORT_CFG(chip_port),
                        ((link_mode & LINK_MODE_FDX_MASK) ? 0 : VTSS_F_REW_PORT_PORT_CFG_AGE_DIS(1)),
                        VTSS_M_REW_PORT_PORT_CFG_AGE_DIS);
#endif

        /* GIGA mode */
        if ((link_mode & LINK_MODE_SPEED_MASK) != LINK_MODE_SPEED_10 && (link_mode & LINK_MODE_SPEED_MASK) != LINK_MODE_SPEED_100) {
#if defined(VTSS_ARCH_LUTON26)
            value |= VTSS_F_DEV_MAC_CFG_STATUS_MAC_MODE_CFG_FDX_ENA |
                     VTSS_F_DEV_MAC_CFG_STATUS_MAC_MODE_CFG_GIGA_MODE_ENA;
#elif defined(VTSS_ARCH_OCELOT)
            value |= VTSS_F_DEV_MAC_CFG_STATUS_MAC_MODE_CFG_FDX_ENA(1) |
                     VTSS_F_DEV_MAC_CFG_STATUS_MAC_MODE_CFG_GIGA_MODE_ENA(1);
#endif
        }

        /* FDX mode */
        if (link_spd_dpx & LINK_MODE_FDX_MASK) {
#if defined(VTSS_ARCH_LUTON26)
            value |= VTSS_F_DEV_MAC_CFG_STATUS_MAC_MODE_CFG_FDX_ENA;
#elif defined(VTSS_ARCH_OCELOT)
            value |= VTSS_F_DEV_MAC_CFG_STATUS_MAC_MODE_CFG_FDX_ENA(1);
#endif
        }

#if defined(VTSS_ARCH_LUTON26)
        H2_WRITE(VTSS_DEV_CMN_MAC_CFG_STATUS_MAC_MODE_CFG(VTSS_TO_DEV(chip_port)), value);
#elif defined(VTSS_ARCH_OCELOT)
        H2_WRITE_MASKED(VTSS_DEV_MAC_CFG_STATUS_MAC_MODE_CFG(VTSS_TO_DEV(chip_port)),
                        value,
                        VTSS_M_DEV_MAC_CFG_STATUS_MAC_MODE_CFG_GIGA_MODE_ENA |
                        VTSS_M_DEV_MAC_CFG_STATUS_MAC_MODE_CFG_FDX_ENA);
#endif

        /* Default FDX gaps */
#if defined(VTSS_ARCH_LUTON26)
        if ((link_spd_dpx & LINK_MODE_SPEED_MASK) == LINK_MODE_SPEED_1000) {
            fdx_gap = 5;
        } else if (chip_port < 10) {
            fdx_gap = 16; // the default value is 17 but the osc is little slower now
        } else if (chip_port < 12) {
            if (link_spd_dpx == LINK_MODE_HDX_10 || link_spd_dpx == LINK_MODE_HDX_100)
                fdx_gap = 14; // the default value is 17 but the osc is little slower now
            else
                fdx_gap = 16; // the default value is 17 but the osc is little slower now
        } else {
            fdx_gap = 15;
        }
#elif defined(VTSS_ARCH_OCELOT)
        // Refer to Serval-1 setting here
        if (link_spd_dpx == LINK_MODE_FDX_10 || link_spd_dpx == LINK_MODE_FDX_100) {
            if (phy_map_miim_no(chip_port) == MAC_IF_INTERNAL) {
                // BZ#21595 - MINI PHY ports receive more broadcast traffic rate when speed is 10MFDX/100MFDX
                fdx_gap = 16;
            } else {
                fdx_gap = 15;
            }
        } else if (link_spd_dpx == LINK_MODE_HDX_10 || link_spd_dpx == LINK_MODE_HDX_100) {
            // BZ#21595 - Receive more broadcast traffic rate when speed is 10MHDX/100MHDX
            if (phy_map_miim_no(chip_port) == MAC_IF_INTERNAL) {
                // BZ#21738 - MINI PHY ports receive more broadcast traffic rate when speed is 10MHDX/100MHDX
                fdx_gap = 16;
            } else {
                fdx_gap = 14;
            }
        } else { // 1G/2.5G
            fdx_gap = 5;
        }
        hdx_gap_1 = 0;
        hdx_gap_2 = 0;
#endif

#if defined(VTSS_ARCH_LUTON26)
        /* Default HDX gaps */
        if (link_spd_dpx == LINK_MODE_HDX_10) {
            hdx_gap_1 = 11;
            hdx_gap_2 = 9;
        } else if (link_spd_dpx == LINK_MODE_HDX_100) {
            hdx_gap_1 = 7;
            hdx_gap_2 = 9;
        } else {
            hdx_gap_1 = 0;
            hdx_gap_2 = 0;
        }
#elif defined(VTSS_ARCH_OCELOT)
        // Refer to Serval-1 setting here
        if (!(link_spd_dpx & LINK_MODE_FDX_MASK)) {
            hdx_gap_1 = 5;
            hdx_gap_2 = 5;
        }
#endif

        /* The "tx_ifg" is used to adjust the duration of the inter-frame
         * gap in the Tx direction. It maybe need to adjust on different board.
         * 
         * Test Case:
         * Send a boardcast frame from one to all ports and then monitor
         * the L1 frame received rate to see if it equal the port line speed.
         * Increase the value when rate > line_speed, otherwise, decrease it.
         */
        /* Set MAC IFG Gaps */
#if defined(VTSS_ARCH_LUTON26)
        H2_WRITE(VTSS_DEV_CMN_MAC_CFG_STATUS_MAC_IFG_CFG(VTSS_TO_DEV(chip_port)),
                 VTSS_F_DEV_MAC_CFG_STATUS_MAC_IFG_CFG_TX_IFG(fdx_gap) |
                 VTSS_F_DEV_MAC_CFG_STATUS_MAC_IFG_CFG_RX_IFG1(hdx_gap_1) |
                 VTSS_F_DEV_MAC_CFG_STATUS_MAC_IFG_CFG_RX_IFG2(hdx_gap_2));
#elif defined(VTSS_ARCH_OCELOT)
        H2_WRITE_MASKED(VTSS_DEV_MAC_CFG_STATUS_MAC_IFG_CFG(VTSS_TO_DEV(chip_port)),
                        VTSS_F_DEV_MAC_CFG_STATUS_MAC_IFG_CFG_TX_IFG(fdx_gap) |
                        VTSS_F_DEV_MAC_CFG_STATUS_MAC_IFG_CFG_RX_IFG1(hdx_gap_1) |
                        VTSS_F_DEV_MAC_CFG_STATUS_MAC_IFG_CFG_RX_IFG2(hdx_gap_2),
                        VTSS_M_DEV_MAC_CFG_STATUS_MAC_IFG_CFG_TX_IFG |
                        VTSS_M_DEV_MAC_CFG_STATUS_MAC_IFG_CFG_RX_IFG1 |
                        VTSS_M_DEV_MAC_CFG_STATUS_MAC_IFG_CFG_RX_IFG2);
#endif

        /* Set MAC HDX Late collision and load PCS seed. */
#if defined(VTSS_ARCH_LUTON26)
        H2_WRITE_MASKED(VTSS_DEV_CMN_MAC_CFG_STATUS_MAC_HDX_CFG(VTSS_TO_DEV(chip_port)),
                        VTSS_F_DEV_MAC_CFG_STATUS_MAC_HDX_CFG_LATE_COL_POS((chip_port < 12) ? 64UL : 67UL) |
                        VTSS_F_DEV_MAC_CFG_STATUS_MAC_HDX_CFG_SEED(chip_port) |
                        VTSS_F_DEV_MAC_CFG_STATUS_MAC_HDX_CFG_SEED_LOAD,
                        VTSS_M_DEV_MAC_CFG_STATUS_MAC_HDX_CFG_LATE_COL_POS|
                        VTSS_M_DEV_MAC_CFG_STATUS_MAC_HDX_CFG_SEED|
                        VTSS_F_DEV_MAC_CFG_STATUS_MAC_HDX_CFG_SEED_LOAD);
#elif defined(VTSS_ARCH_OCELOT)
        H2_WRITE_MASKED(VTSS_DEV_MAC_CFG_STATUS_MAC_HDX_CFG(VTSS_TO_DEV(chip_port)),
                        VTSS_F_DEV_MAC_CFG_STATUS_MAC_HDX_CFG_LATE_COL_POS(67) |
                        VTSS_F_DEV_MAC_CFG_STATUS_MAC_HDX_CFG_SEED(chip_port) |
                        VTSS_F_DEV_MAC_CFG_STATUS_MAC_HDX_CFG_SEED_LOAD(1),
                        VTSS_M_DEV_MAC_CFG_STATUS_MAC_HDX_CFG_LATE_COL_POS|
                        VTSS_M_DEV_MAC_CFG_STATUS_MAC_HDX_CFG_SEED|
                        VTSS_M_DEV_MAC_CFG_STATUS_MAC_HDX_CFG_SEED_LOAD);
#endif

        /* Clear seed-load after a small delay (rising edge is sampled in rx-clock domain). */
        delay_1(2);
#if defined(VTSS_ARCH_LUTON26)
        H2_WRITE_MASKED(VTSS_DEV_CMN_MAC_CFG_STATUS_MAC_HDX_CFG(VTSS_TO_DEV(chip_port)),
                        0,
                        VTSS_F_DEV_MAC_CFG_STATUS_MAC_HDX_CFG_SEED_LOAD);
#elif defined(VTSS_ARCH_OCELOT)
        H2_WRITE_MASKED(VTSS_DEV_MAC_CFG_STATUS_MAC_HDX_CFG(VTSS_TO_DEV(chip_port)),
                        VTSS_F_DEV_MAC_CFG_STATUS_MAC_HDX_CFG_SEED_LOAD(0),
                        VTSS_M_DEV_MAC_CFG_STATUS_MAC_HDX_CFG_SEED_LOAD);
        /* Disable HDX fast control */
        H2_WRITE_MASKED(VTSS_DEV_PORT_MODE_PORT_MISC(VTSS_TO_DEV(chip_port)), 
                        VTSS_F_DEV_PORT_MODE_PORT_MISC_HDX_FAST_DIS(1),
                        VTSS_M_DEV_PORT_MODE_PORT_MISC_HDX_FAST_DIS);
#endif

#if defined(VTSS_ARCH_LUTON26)
        /* Check point: Should be 9 or 11 no matter L10, L16 and L25? */
        if (chip_port > 11) {
#if MAC_TO_MEDIA
            u8   mac_if = phy_map_miim_no(chip_port);

            if ((mac_if != MAC_IF_SERDES_1G) &&
                (mac_if != MAC_IF_100FX)  &&
                (mac_if != MAC_IF_SGMII))
#endif
                _setup_pcs(chip_port, link_mode);
        }
#endif

        if (link_spd_dpx == LINK_MODE_HDX_10 ||
            link_spd_dpx == LINK_MODE_HDX_100)
        {
#if defined(VTSS_ARCH_LUTON26)
            H2_WRITE_MASKED(VTSS_REW_PORT_PORT_CFG(chip_port),
                            VTSS_F_REW_PORT_PORT_CFG_AGE_DIS,
                            VTSS_F_REW_PORT_PORT_CFG_AGE_DIS);
#elif defined(VTSS_ARCH_OCELOT)
            H2_WRITE_MASKED(VTSS_REW_PORT_PORT_CFG(chip_port),
                            VTSS_F_REW_PORT_PORT_CFG_AGE_DIS(1),
                            VTSS_M_REW_PORT_PORT_CFG_AGE_DIS);
#endif
        } else {
#if defined(VTSS_ARCH_LUTON26)
            H2_WRITE_MASKED(VTSS_REW_PORT_PORT_CFG(chip_port),
                            0,
                            VTSS_F_REW_PORT_PORT_CFG_AGE_DIS);
#elif defined(VTSS_ARCH_OCELOT)
            H2_WRITE_MASKED(VTSS_REW_PORT_PORT_CFG(chip_port),
                            VTSS_F_REW_PORT_PORT_CFG_AGE_DIS(0),
                            VTSS_M_REW_PORT_PORT_CFG_AGE_DIS);
#endif
        }
    }
}


/*****************************************************************************
 *
 *
 * Public functions
 *
 *
 *
 ****************************************************************************/
/**
 * Reset switch chip.
 */
void h2_reset (void) small
{

#if defined(VTSS_ARCH_OCELOT)
    H2_WRITE_MASKED(VTSS_DEVCPU_GCB_CHIP_REGS_SOFT_RST,
                                        VTSS_F_DEVCPU_GCB_CHIP_REGS_SOFT_RST_SOFT_CHIP_RST(1),
                                        VTSS_M_DEVCPU_GCB_CHIP_REGS_SOFT_RST_SOFT_CHIP_RST);
#elif defined(VTSS_ARCH_LUTON26)
    H2_WRITE_MASKED(VTSS_DEVCPU_GCB_DEVCPU_RST_REGS_SOFT_CHIP_RST,
                    VTSS_F_DEVCPU_GCB_DEVCPU_RST_REGS_SOFT_CHIP_RST_SOFT_CHIP_RST,
                    VTSS_F_DEVCPU_GCB_DEVCPU_RST_REGS_SOFT_CHIP_RST_SOFT_CHIP_RST);
#endif
    while (TRUE) {
    }
}

/**
 * Do basic initializations of chip needed after a chip reset.
 */
void h2_post_reset (void)
{
    ulong cmd;
    uchar locked;

    _h2_setup_cpu_if();

#if defined(VTSS_ARCH_OCELOT)
    /* Setup IB-Calibration, the process must be done before serdes mode setup */
    /* Setup HSIO PLL */
    locked = h2_serdes_macro_phase_loop_locked();
    if (!locked) {
        println_str("%% Timeout when calling h2_serdes_macro_phase_loop_locked()");
        sysutil_assert_event_set(SYS_ASSERT_EVENT_H2_POST_RESET);
        return;
    }
#endif

    /* Setup serdes mode (SGMII/QSGMII/2.5G and etc.) */
    h2_serdes_macro_config();

    /* Initialize memories */
#if defined(VTSS_ARCH_OCELOT)
    H2_WRITE(VTSS_SYS_SYSTEM_RESET_CFG,
             VTSS_F_SYS_SYSTEM_RESET_CFG_MEM_ENA(1) |
             VTSS_F_SYS_SYSTEM_RESET_CFG_MEM_INIT(1));
#elif defined(VTSS_ARCH_LUTON26)
      H2_WRITE(VTSS_SYS_SYSTEM_RESET_CFG,
             VTSS_F_SYS_SYSTEM_RESET_CFG_MEM_ENA |
             VTSS_F_SYS_SYSTEM_RESET_CFG_MEM_INIT);
#endif

    /* Wait done flag */
    start_timer(MSEC_100);
#if defined(VTSS_ARCH_LUTON26)
    do {
        H2_READ(VTSS_SYS_SYSTEM_RESET_CFG, cmd);
    } while ((cmd & VTSS_F_SYS_SYSTEM_RESET_CFG_MEM_INIT) != 0 && !timeout());

#elif defined(VTSS_ARCH_OCELOT)
    do {
        H2_READ(VTSS_SYS_SYSTEM_RESET_CFG, cmd);
    } while (VTSS_X_SYS_SYSTEM_RESET_CFG_MEM_INIT(cmd) && !timeout());
#endif

    if (timeout()) {
#if defined(H2_DEBUG_ENABLE)
        println_str("%% Timeout when calling h2_post_reset()");
#endif /* H2_DEBUG_ENABLE */
        sysutil_assert_event_set(SYS_ASSERT_EVENT_H2_INIT);
        return;
    }

    /* Enable the switch core */
#if defined(VTSS_ARCH_OCELOT)
    H2_WRITE_MASKED(VTSS_SYS_SYSTEM_RESET_CFG,
                    VTSS_F_SYS_SYSTEM_RESET_CFG_CORE_ENA(1),
                    VTSS_M_SYS_SYSTEM_RESET_CFG_CORE_ENA);
#elif defined(VTSS_ARCH_LUTON26)
    H2_WRITE_MASKED(VTSS_SYS_SYSTEM_RESET_CFG,
                    VTSS_F_SYS_SYSTEM_RESET_CFG_CORE_ENA,
                    VTSS_F_SYS_SYSTEM_RESET_CFG_CORE_ENA);
#endif


#if defined(VTSS_ARCH_LUTON26)
    /* Setup HSIO PLL */
    locked = h2_serdes_macro_phase_loop_locked();
    if (!locked) {
        sysutil_assert_event_set(SYS_ASSERT_EVENT_H2_POST_RESET);
        return;
    }
#elif defined(VTSS_ARCH_OCELOT)
    // Ferret has the different initial proceduces.
    // We need to setup HSIO PLL before calling h2_serdes_macro_config()
#endif

    /* Initialize MAC-table and VLAN-table then wait for done */
    h2_mactab_clear();
    h2_vlan_clear_tab();
    delay(MSEC_40);

    h2_mactab_agetime_set();

    /* Initialize leaky buckets */
#if defined(VTSS_ARCH_LUTON26)
    H2_WRITE(VTSS_SYS_SCH_SCH_LB_CTRL, VTSS_F_SYS_SCH_SCH_LB_CTRL_LB_INIT);
    _l26_buf_conf_set();
    do { /* Wait until leaky buckets initialization is completed  */
        H2_READ(VTSS_SYS_SCH_SCH_LB_CTRL, cmd);
    } while(cmd & VTSS_F_SYS_SCH_SCH_LB_CTRL_LB_INIT);
#elif defined(VTSS_ARCH_SERVAL_ORG)
    // Ferret, TODO. Keep the QoS setting as the chip factory default
	// See srvl_port_buf_conf_set() in VSC7514 API
    _ferret_buf_conf_set();
#endif

    /* Setup frame ageing - "2 sec" */
#if defined(VTSS_ARCH_LUTON26)
    // The unit is 4us on Luton
    H2_WRITE(VTSS_SYS_SYSTEM_FRM_AGING, 0x1dcd6500);
#elif defined(VTSS_ARCH_OCELOT)
    // The unit is 6.5us on Ferret
    H2_WRITE(VTSS_SYS_SYSTEM_FRM_AGING,
            VTSS_F_SYS_SYSTEM_FRM_AGING_AGE_TX_ENA(1) |
            VTSS_F_SYS_SYSTEM_FRM_AGING_MAX_AGE(20000000 / 65));
#endif

    H2_WRITE(VTSS_ANA_ANA_TABLES_ANMOVED, 0);
}


/**
 * Presets link mode of all ports.
 */
void h2_init_ports (void)
{
    vtss_iport_no_t iport_idx;
    vtss_cport_no_t chip_port;
#if defined(VTSS_ARCH_OCELOT)
    ulong  reg_val;
    port_bit_mask_t chip_port_map=0;
#endif

#if TRANSIT_LAG || TRANSIT_LACP
    /* trunk and LACP */
    memset(&h2_current_state, 0, sizeof(h2_current_state));
#endif
    
    /* Max frame length */
   for (iport_idx = MIN_PORT; iport_idx < MAX_PORT; iport_idx++) {
        chip_port = iport2cport(iport_idx);
#if defined(VTSS_ARCH_OCELOT)
        chip_port_map |= (0x1 << chip_port); /* bit map for the use chip ports */
        H2_WRITE(VTSS_DEV_MAC_CFG_STATUS_MAC_MAXLEN_CFG(VTSS_TO_DEV(chip_port)), MAX_FRAME_SIZE);
#elif defined(VTSS_ARCH_LUTON26)
        H2_WRITE(VTSS_DEV_CMN_MAC_CFG_STATUS_MAC_MAXLEN_CFG(VTSS_TO_DEV(chip_port)), MAX_FRAME_SIZE);
#endif
        h2_setup_port(chip_port, LINK_MODE_DOWN);
#if defined(UNMANAGED_ENHANCEMENT) && defined(VTSS_ARCH_OCELOT)
        h2_vlan_setup(iport_idx);
#endif
    }

#if defined(VTSS_ARCH_OCELOT)
    H2_READ(VTSS_HSIO_HW_CFGSTAT_HW_CFG,reg_val);
    if (reg_val&VTSS_M_HSIO_HW_CFGSTAT_HW_CFG_QSGMII_ENA) {
        for (chip_port = 4; chip_port < 8; chip_port++) { /* check QSGMII ports only */
            if (!TEST_PORT_BIT_MASK(chip_port, &chip_port_map)) {
                //print_str("Unmap port idx : ");
                //print_hex_b(chip_port);
                //print_cr_lf();
                /* Port Reset for Unused QSGMII ports */
                _port_reset(chip_port, LINK_MODE_DOWN);
                H2_WRITE_MASKED(VTSS_DEV_PORT_MODE_CLOCK_CFG( VTSS_TO_DEV(chip_port)),
                                VTSS_F_DEV_PORT_MODE_CLOCK_CFG_PCS_TX_RST(0) |
                                VTSS_F_DEV_PORT_MODE_CLOCK_CFG_PCS_RX_RST(0) |
                                VTSS_F_DEV_PORT_MODE_CLOCK_CFG_PORT_RST(0),
                                VTSS_M_DEV_PORT_MODE_CLOCK_CFG_PCS_TX_RST |
                                VTSS_M_DEV_PORT_MODE_CLOCK_CFG_PCS_RX_RST |
                                VTSS_M_DEV_PORT_MODE_CLOCK_CFG_PORT_RST);
            }
        }
    }
#endif
    /* Update source mask */
    // No need to update when initial as the default value works well
    //_vtss_update_src_mask(0);
    VTSS_UPDATE_MASKS_DEBUG();
    vtss_update_masks();
}

/**
 * Set up port including MAC according to link_mode parameter.
 *
 * @see Please see main.h for a description of link_mode.
 */
void h2_setup_port(vtss_cport_no_t chip_port, uchar link_mode)
{
    _port_reset(chip_port, link_mode);
    _setup_port(chip_port, link_mode);

#if defined(VTSS_ARCH_OCELOT)
    _setup_pcs(chip_port, link_mode);
#elif defined(VTSS_ARCH_LUTON26)
        if (chip_port > 11) {
#if MAC_TO_MEDIA
            u8   mac_if = phy_map_miim_no(chip_port);

            if ((mac_if != MAC_IF_SERDES_1G) &&
                (mac_if != MAC_IF_100FX)  &&
                (mac_if != MAC_IF_SGMII))
#endif
                {
                    _setup_pcs(chip_port, link_mode);
                }
        }
#endif // VTSS_ARCH_OCELOT
}


/**
 * Port reset procedure and flush MAC address entries when link-down.
 *
 */
static void _port_reset(vtss_cport_no_t chip_port, uchar link_mode)
{
#if defined(VTSS_ARCH_OCELOT)
    /* Note: It is not necessary to reset the SerDes macros. */
    _ferret_port_reset(chip_port, link_mode);

#elif defined(VTSS_ARCH_LUTON26)
    u32 value;
    u8  mac_if = phy_map_miim_no(chip_port);

    /* Port disable and flush procedure: */
    /* 0.1: Reset the PCS */
    if (chip_port > 9) {
        H2_WRITE_MASKED(VTSS_DEV_PORT_MODE_CLOCK_CFG(VTSS_TO_DEV(chip_port)), 0,
                        VTSS_F_DEV_PORT_MODE_CLOCK_CFG_PCS_RX_RST);
    }

    /* 1: Disable MAC frame reception */
    H2_WRITE_MASKED(VTSS_DEV_CMN_MAC_CFG_STATUS_MAC_ENA_CFG(VTSS_TO_DEV(chip_port)), 0,
                    VTSS_F_DEV_MAC_CFG_STATUS_MAC_ENA_CFG_RX_ENA);


    /* 2: Disable traffic being sent to or from switch port */
    H2_WRITE_MASKED(VTSS_SYS_SYSTEM_SWITCH_PORT_MODE(chip_port), 0,
                    VTSS_F_SYS_SYSTEM_SWITCH_PORT_MODE_PORT_ENA);

    /* 3: Disable dequeuing from the egress queues */
    H2_WRITE_MASKED(VTSS_SYS_SYSTEM_PORT_MODE(chip_port), VTSS_F_SYS_SYSTEM_PORT_MODE_DEQUEUE_DIS,
                    VTSS_F_SYS_SYSTEM_PORT_MODE_DEQUEUE_DIS);

    /* 4: Wait a worst case time 8ms (jumbo/10Mbit) */
    delay_1(10);

    /* 5: Disable HDX backpressure (Bugzilla 3203) */
    H2_WRITE_MASKED(VTSS_SYS_SYSTEM_FRONT_PORT_MODE(chip_port), 0UL,
                    VTSS_F_SYS_SYSTEM_FRONT_PORT_MODE_HDX_MODE);

    /* 6: Flush the queues accociated with the port */
    H2_WRITE_MASKED(VTSS_REW_PORT_PORT_CFG(chip_port), VTSS_F_REW_PORT_PORT_CFG_FLUSH_ENA,
                    VTSS_F_REW_PORT_PORT_CFG_FLUSH_ENA);

    /* 7: Enable dequeuing from the egress queues */
    H2_WRITE_MASKED(VTSS_SYS_SYSTEM_PORT_MODE(chip_port), 0UL,
                    VTSS_F_SYS_SYSTEM_PORT_MODE_DEQUEUE_DIS);

    /* 9: Reset the clock */
    if (chip_port > 9) {
#if MAC_TO_MEDIA
        if (mac_if == MAC_IF_100FX) {
            H2_WRITE_MASKED(VTSS_DEV_PORT_MODE_CLOCK_CFG(VTSS_TO_DEV(chip_port)),
                 VTSS_F_DEV_PORT_MODE_CLOCK_CFG_MAC_TX_RST |
                 VTSS_F_DEV_PORT_MODE_CLOCK_CFG_MAC_RX_RST |
                 VTSS_F_DEV_PORT_MODE_CLOCK_CFG_PHY_RST, 0x000000c4);
        } else
#endif
        {
            H2_WRITE(VTSS_DEV_PORT_MODE_CLOCK_CFG(VTSS_TO_DEV(chip_port)),
                 VTSS_F_DEV_PORT_MODE_CLOCK_CFG_MAC_TX_RST |
                 VTSS_F_DEV_PORT_MODE_CLOCK_CFG_MAC_RX_RST |
                 VTSS_F_DEV_PORT_MODE_CLOCK_CFG_PORT_RST);
        }
    } else {
        H2_WRITE(VTSS_DEV_GMII_PORT_MODE_CLOCK_CFG(VTSS_TO_DEV(chip_port)),
                 VTSS_F_DEV_GMII_PORT_MODE_CLOCK_CFG_MAC_TX_RST |
                 VTSS_F_DEV_GMII_PORT_MODE_CLOCK_CFG_MAC_RX_RST |
                 VTSS_F_DEV_GMII_PORT_MODE_CLOCK_CFG_PORT_RST);
    }

    /* 8. Wait until flushing is complete */
    start_timer(MSEC_2000);
    do {
        H2_READ(VTSS_SYS_SYSTEM_SW_STATUS(chip_port), value);
    } while ((value & VTSS_M_SYS_SYSTEM_SW_STATUS_EQ_AVAIL) && !timeout());

    if (timeout()) {
#if defined(H2_DEBUG_ENABLE)
        print_cr_lf();
        println_str("%% Timeout when calling _port_reset(), chip_port = ");
        print_dec(chip_port);
        print_cr_lf();
#endif /* H2_DEBUG_ENABLE */
        return;
    }

    /* 10: Clear flushing */
    H2_WRITE_MASKED(VTSS_REW_PORT_PORT_CFG(chip_port), 0, VTSS_F_REW_PORT_PORT_CFG_FLUSH_ENA);

    _setup_mac(chip_port, link_mode);
#endif

    /* The port is disabled and flushed, now set up the port in the new operating mode */
    if (link_mode == LINK_MODE_DOWN) {
        h2_mactab_flush_port(chip_port);  // Flush MAC address entries
    }
}

#if H2_ID_CHECK
/**
 * Check health status of switch chip.
 *
 * @return  Returns TRUE, if chip is ok, otherwise <> FALSE.
 */
BOOL h2_chip_family_support(void) small
{
    ulong chip_id = 0x0;

#if defined(LUTON26_L25)
#define EXPECTED_CHIPID 0x07422
#elif defined(LUTON26_L16)
#define EXPECTED_CHIPID 0x07421
#elif defined(LUTON26_L10)
#define EXPECTED_CHIPID 0x07420
#elif defined(VTSS_ARCH_OCELOT)
#define EXPECTED_CHIPID 0X7514
#else
#error "Not defined yet: EXPECTED_CHIPID"
#endif

    H2_READ(VTSS_DEVCPU_GCB_CHIP_REGS_CHIP_ID, chip_id);
    if ((VTSS_X_DEVCPU_GCB_CHIP_REGS_CHIP_ID_PART_ID(chip_id) & EXPECTED_CHIPID) != EXPECTED_CHIPID) {
#if !defined(NO_DEBUG_IF)
        print_str("%% Chip ID is unsupport");
        print_hex_dw(chip_id);
        print_cr_lf();
#endif

        return FALSE;
    }

    return TRUE;
}
#endif // H2_ID_CHECK

/*****************************************************************************
 *
 *
 * Public register access functions (primarily for code space optimization)
 *
 *
 *
 ****************************************************************************/


void h2_enable_exc_col_drop (vtss_cport_no_t chip_port, uchar drop_enable)
{
#if defined(VTSS_ARCH_OCELOT)
    H2_WRITE_MASKED(VTSS_DEV_MAC_CFG_STATUS_MAC_HDX_CFG(VTSS_TO_DEV(chip_port)),
                    VTSS_F_DEV_MAC_CFG_STATUS_MAC_HDX_CFG_RETRY_AFTER_EXC_COL_ENA(!drop_enable),
                    VTSS_M_DEV_MAC_CFG_STATUS_MAC_HDX_CFG_RETRY_AFTER_EXC_COL_ENA);
#elif defined(VTSS_ARCH_LUTON26)
    H2_WRITE_MASKED(VTSS_DEV_CMN_MAC_CFG_STATUS_MAC_HDX_CFG(VTSS_TO_DEV(chip_port)),
                    (drop_enable? 0UL:VTSS_F_DEV_MAC_CFG_STATUS_MAC_HDX_CFG_RETRY_AFTER_EXC_COL_ENA),
                    VTSS_F_DEV_MAC_CFG_STATUS_MAC_HDX_CFG_RETRY_AFTER_EXC_COL_ENA);
#endif
}

/*****************************************************************************
 *
 *
 * Help functions
 *
 *
 *
 ****************************************************************************/

/**
 * In Unmanaged code, this function returns the the port mask without sport
 * set. In managed code, the result mask is determined by the User defined
 * Private VLAN group (with the aggregation in mind).
 */
#if TRANSIT_LAG
static port_bit_mask_t _vtss_get_pvlan_mask(vtss_cport_no_t chip_port)
{
    return ~PORT_BIT_MASK(chip_port);
}

static port_bit_mask_t vtss_aggr_find_group(uchar port_no)
{
    uchar group;

    VTSS_COMMON_ASSERT(port_no < MAX_PORT);
    for (group = 0; group < TOTAL_AGGRS; group++) {
        //print_str(" group: "); print_dec(group); print_spaces(2); print_hex_dw(h2_current_state.h2_current_aggr_groups[group]); 
        if (TEST_PORT_BIT_MASK(port_no, &h2_current_state.h2_current_aggr_groups[group]))
            return h2_current_state.h2_current_aggr_groups[group];
    }        
    return PORT_BIT_MASK(port_no);
}


static void _vtss_update_dest_mask(port_bit_mask_t link_mask)
{
    // Fix compilation warning.
    
    vtss_iport_no_t iport_idx;
    vtss_cport_no_t chip_port;
    port_bit_mask_t member;

    for (iport_idx = MIN_PORT; iport_idx < MAX_PORT; iport_idx++) {
        chip_port = iport2cport(iport_idx);
        member = vtss_aggr_find_group(chip_port) & link_mask;
        H2_WRITE(VTSS_ANA_PGID_PGID(chip_port + VTSS_PGID_DEST_MASK_START), member);
        //print_dec(chip_port + VTSS_PGID_DEST_MASK_START); print_spaces(3); print_hex_dw(member); print_cr_lf();
    }    
    
}

static void _vtss_update_src_mask(port_bit_mask_t link_mask)
{
    vtss_iport_no_t iport_idx;
    vtss_cport_no_t chip_port;
    port_bit_mask_t member;

    for (iport_idx = MIN_PORT; iport_idx < MAX_PORT; iport_idx++) {
        chip_port = iport2cport(iport_idx);
#if defined(VTSS_ARCH_OCELOT)
        member = link_mask & _vtss_get_pvlan_mask(chip_port) & (~vtss_aggr_find_group(chip_port));
        //member = member | PORT_BIT_MASK(CPU_CHIP_PORT);
        H2_WRITE(VTSS_ANA_PGID_PGID(chip_port + VTSS_PGID_SOURCE_MASK_START), member);
        //print_dec(chip_port + VTSS_PGID_SOURCE_MASK_START); print_spaces(3); print_hex_dw(member); print_cr_lf();
#elif defined(VTSS_ARCH_LUTON26)
        /* STP and Authentication state allow forwarding from port. */
        if (_ingr_forwarding(chip_port)) {
            member = _vtss_get_pvlan_mask(chip_port);
        } else {
            /* Exclude all ports by default */
            member = 0;
        }

        H2_WRITE_MASKED(VTSS_ANA_ANA_TABLES_PGID(chip_port + VTSS_PGID_SOURCE_MASK_START),
                        member & link_mask,
                        ALL_PORTS);
#endif

#if defined(H2_DEBUG_ENABLE)
{
        u32 reg_val;

        print_str("link_mask=0x");
        print_hex_b(link_mask);

        print_str(", chip_port=");
        print_dec_8_right_2(chip_port);

        print_str(", member=0x");
        print_hex_b(member);

        H2_READ(VTSS_ANA_PGID_PGID(chip_port + VTSS_PGID_SOURCE_MASK_START), reg_val);
        print_str(", ANA:PGID:PGID=0x");
        print_hex_dw(reg_val);
        print_cr_lf();
}
#endif /* H2_DEBUG_ENABLE */
    }

    delay(MSEC_100);
}

static void _vtss_update_aggr_mask(port_bit_mask_t link_mask)
{
    vtss_iport_no_t i_port_no, e_port_no;
    vtss_cport_no_t ic_port_no, ec_port_no;
    // The size of aggr_xxx array must be NO_OF_CHIP_PORTS
    uchar           aggr_count[NO_OF_CHIP_PORTS], aggr_index[NO_OF_CHIP_PORTS], n;
    uchar           aggr_lport[NO_OF_CHIP_PORTS]; 
    port_bit_mask_t member_mask = 0;
    port_bit_mask_t test_member_mask = 0;
    uchar ix;
    
    /* Update aggregation masks */
    //memset(aggr_count, 0, sizeof(aggr_count));
    //memset(aggr_index, 0, sizeof(aggr_index));

    /* Count number of operational ports and index of each port */
    for (i_port_no = MIN_PORT; i_port_no < MAX_PORT; i_port_no++) {
        ic_port_no = iport2cport(i_port_no);
        aggr_count[ic_port_no] = 0; 
        aggr_index[ic_port_no] = 0;
        aggr_lport[ic_port_no] = ic_port_no;

        /* If port is up and forwarding */
        if (TEST_PORT_BIT_MASK(ic_port_no, &link_mask)) {
            member_mask = vtss_aggr_find_group(ic_port_no) & link_mask;
            //print_cr_lf(); print_str("yyy "); print_dec(ic_port_no); print_spaces(3); print_hex_dw(member_mask); print_cr_lf();
            for (e_port_no = MIN_PORT; e_port_no < MAX_PORT; e_port_no++) {
                ec_port_no = iport2cport(e_port_no);
                if (TEST_PORT_BIT_MASK(ec_port_no, &member_mask)) {
                    /* Port is forwarding and member of the same aggregation */
                    aggr_count[ic_port_no] += 1;
                    if (ec_port_no < ic_port_no) {
                        aggr_index[ic_port_no] += 1;
                        if (ec_port_no < aggr_lport[ic_port_no] ) {
                            aggr_lport[ic_port_no] = ec_port_no;
                        }    
                    }    
                }
            }
            //print_str("xxx ");
            //print_dec(ic_port_no); print_spaces(3); print_dec(aggr_count[ic_port_no]);
            //print_spaces(3); print_dec(aggr_index[ic_port_no]);
            //print_str("lport: "); print_dec(aggr_lport[ic_port_no]);print_cr_lf();
            
        }
    }

    for (ix = 0; ix < MAX_KEY; ix++) {
        /* Include one forwarding port from each aggregation */
        for (i_port_no = MIN_PORT; i_port_no < MAX_PORT; i_port_no++) {
            ic_port_no = iport2cport(i_port_no);
            n = aggr_index[ic_port_no] + ix;
            WRITE_PORT_BIT_MASK(ic_port_no,
                                (aggr_count[ic_port_no] && (n % aggr_count[ic_port_no]) == 0) ? 1 : 0,
                                &member_mask);
        }

        /* Write to aggregation table */
        H2_WRITE(VTSS_ANA_PGID_PGID(VTSS_PGID_AGGR_MASK_START + ix), member_mask);
        H2_READ(VTSS_ANA_PGID_PGID(VTSS_PGID_AGGR_MASK_START + ix), test_member_mask);
        if (test_member_mask != member_mask) {
            //delay_1(10);
            print_dec(VTSS_PGID_AGGR_MASK_START + ix); print_spaces(3); print_hex_dw(member_mask);
            print_spaces(3); print_hex_dw(test_member_mask); print_cr_lf();
        }
    }
    
    /* Update port map table on aggregation changes */
    for (i_port_no = MIN_PORT; i_port_no < MAX_PORT; i_port_no++) {    
        ic_port_no = iport2cport(i_port_no);
        H2_WRITE_MASKED(VTSS_ANA_PORT_PORT_CFG(ic_port_no),
                            VTSS_F_ANA_PORT_PORT_CFG_PORTID_VAL(aggr_lport[ic_port_no]), 
                            VTSS_M_ANA_PORT_PORT_CFG_PORTID_VAL);
    }
}

void vtss_show_masks(void)
{
    uchar ix;
    ulong reg_val;
    
    for(ix=0; ix<= VTSS_PGID_SOURCE_MASK_START + NO_OF_CHIP_PORTS; ix++) {
        print_dec(ix);print_str(": 0x"); 
        H2_READ(VTSS_ANA_PGID_PGID(ix), reg_val);
        print_hex_dw(reg_val);
        print_cr_lf();
    }    
}    
#endif //TRANSIT_LAG

void vtss_update_masks(void)
{
#if TRANSIT_LAG     
    port_bit_mask_t     link_mask;

#ifndef VTSS_COMMON_NDEBUG
    vtss_printf("vtss_update_masks: from file \"%s\" line %u\n",
                _common_file, _common_line);
#endif /* !VTSS_COMMON_NDEBUG */

    link_mask = linkup_cport_mask_get();

    /*
     * Update source mask
     */
     _vtss_update_src_mask(link_mask);

    /*
     * Update destination table
     */
    _vtss_update_dest_mask(link_mask);

    /*
     * Update aggregation masks
     */
    _vtss_update_aggr_mask(link_mask);
#endif //TRANSIT_LAG
}

#if TRANSIT_LAG 
void vtss_set_aggr_group(uchar group, port_bit_mask_t members)
/* ------------------------------------------------------------------------ --
 * Purpose     : Save aggregation group info.
 * Remarks     :
 * Restrictions:
 * See also    :
 * Example     :
 * ************************************************************************ */
{
    VTSS_COMMON_ASSERT(group < TOTAL_AGGRS);
    h2_current_state.h2_current_aggr_groups[group] = members;
    //print_str("group: "); print_dec(group); print_spaces(2); print_hex_dw(members);
    vtss_update_masks();
}

/* ************************************************************************ */
port_bit_mask_t vtss_get_aggr_group(uchar group)
/* ------------------------------------------------------------------------ --
 * Purpose     : Return aggregation group info.
 * Remarks     :
 * Restrictions:
 * See also    :
 * Example     :
 * ************************************************************************ */
{
    VTSS_COMMON_ASSERT(group < TOTAL_AGGRS);
    return h2_current_state.h2_current_aggr_groups[group];
}
#endif //TRANSIT_LAG

void h2_chip_reset(void)
{
#if defined(VTSS_ARCH_OCELOT)
    H2_WRITE(VTSS_DEVCPU_GCB_CHIP_REGS_SOFT_RST,
             VTSS_F_DEVCPU_GCB_CHIP_REGS_SOFT_RST_SOFT_CHIP_RST(1));

#elif defined(VTSS_ARCH_LUTON26)
    H2_WRITE(VTSS_DEVCPU_GCB_DEVCPU_RST_REGS_SOFT_CHIP_RST,
             VTSS_F_DEVCPU_GCB_DEVCPU_RST_REGS_SOFT_CHIP_RST_SOFT_CHIP_RST);
#endif // VTSS_ARCH_OCELOT
}

#if defined(NPI_CHIP_PORT) && NPI_CHIP_PORT != NPI_ACT_NORMAL_PORT
/* Set NPI configuration
 * Notices that the original NPI port will be inactive after applied the new setting.
 */
void npi_port_set(h2_npi_conf_t *npi_conf)
{
#if defined(VTSS_ARCH_OCELOT)
    H2_WRITE(VTSS_QSYS_SYSTEM_EXT_CPU_CFG,
             VTSS_F_QSYS_SYSTEM_EXT_CPU_CFG_EXT_CPU_PORT(npi_conf->mode_enabled ? npi_conf->chip_port : 0) |
             VTSS_F_QSYS_SYSTEM_EXT_CPU_CFG_EXT_CPUQ_MSK(npi_conf->mode_enabled ? npi_conf->queue_mask : 0));

     /* Enable/Disable IFH parsing upon injection / extraction */
    H2_WRITE_MASKED(VTSS_SYS_SYSTEM_PORT_MODE(npi_conf->chip_port), 
                    VTSS_F_SYS_SYSTEM_PORT_MODE_INCL_INJ_HDR(npi_conf->mode_enabled ? npi_conf->prefix_header_mode : 0) |
                    VTSS_F_SYS_SYSTEM_PORT_MODE_INCL_XTR_HDR(npi_conf->mode_enabled ? npi_conf->prefix_header_mode : 0),
                    VTSS_M_SYS_SYSTEM_PORT_MODE_INCL_INJ_HDR |
                    VTSS_M_SYS_SYSTEM_PORT_MODE_INCL_XTR_HDR);
#endif // VTSS_ARCH_OCELOT
}
#endif // NPI_CHIP_PORT && NPI_CHIP_PORT != NPI_ACT_NORMAL_PORT

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
