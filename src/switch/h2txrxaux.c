//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#include "common.h"     /* Always include common.h at the first place of user-defined herder files */
#include "vtss_api_base_regs.h"
#include "h2io.h"
#include "timer.h"
#include "h2packet.h"
#include "h2txrxaux.h"
#include "misc2.h"
#include "misc3.h"
#include "hwport.h"
#include <string.h>
#include "spiflash.h"
#include "print.h"
#include "vtss_common_os.h"
#if TRANSIT_LOOPDETECT
#include "loopdet.h"
#endif

#if defined(VTSS_ARCH_OCELOT)
#if TRANSIT_LLDP || TRANSIT_LACP
#define __BASIC_TX_RX__ 1
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


#define XTR_EOF_0     0x80000000UL
#define XTR_EOF_1     0x80000001UL
#define XTR_EOF_2     0x80000002UL
#define XTR_EOF_3     0x80000003UL
#define XTR_PRUNED    0x80000004UL
#define XTR_ABORT     0x80000005UL
#define XTR_ESCAPE    0x80000006UL
#define XTR_NOT_READY 0x80000007UL



static ulong rx_word (uchar qno);
static void  tx_word (uchar qno, ulong value);
static bool  fifo_status(uchar qno);

#if 0 //uncalled function
#if TRANSIT_LLDP
static void h2_rx_frame_discard(const uchar qno)
{
    uchar done = FALSE;

    while(!done) {
        ulong val;
        val = rx_word(qno);
        switch(val) {
        case XTR_ABORT:
        case XTR_PRUNED:
        case XTR_EOF_3:
        case XTR_EOF_2:
        case XTR_EOF_1:
        case XTR_EOF_0:
            val = rx_word(qno); /* Last data */
            done = TRUE;        /* Last 1-4 bytes */
            break;
        case XTR_ESCAPE:
            val = rx_word(qno); /* Escaped data */
            break;
        case XTR_NOT_READY:
        default:
            ;
        }
    }
}
#endif // TRANSIT_LLDP
#endif // uncalled function

#if defined(VTSS_ARCH_OCELOT)
void h2_rx_frame_get (const uchar qno, vtss_rx_frame_t xdata * rx_frame_ptr)
/* ------------------------------------------------------------------------ --
 * Purpose     : Receive frame.
 * Remarks     : The structure pointed to by rx_frame_ptr is updated with the
 *               data received, see h2packet.h for a description of the structure.
 * Restrictions: Only to be called if h2_frame_received has returned TRUE.
 * See also    :
 * Example     :
 ****************************************************************************/
{
    ulong  qstat;
    ulong  ifh0, ifh1, ifh2, ifh3;
    ulong  xdata *packet;
    uchar  eof_flag, escape_flag, abort_flag, pruned_flag;
    ulong wlen, llen, ifh_reported_len_incl_fcs;

#define MAX_LENGTH (RECV_BUFSIZE+2)

    vtss_eth_hdr xdata * eth_hdr = rx_frame_ptr->rx_packet;
    mac_addr_t self_mac_addr;

    get_mac_addr(SYSTEM_MAC_ADDR, self_mac_addr);

    h2_discard_frame(rx_frame_ptr);

    H2_READ(VTSS_DEVCPU_QS_XTR_XTR_DATA_PRESENT, qstat);
       //print_str("VTSS_DEVCPU_QS_XTR_XTR_DATA_PRESENT: ");    print_dec(qstat);     print_cr_lf();
    
    if(qstat & VTSS_F_DEVCPU_QS_XTR_XTR_DATA_PRESENT_DATA_PRESENT(VTSS_BIT(qno))) {
        //println_str(" pkt coming !!!"); 
        ifh0 = rx_word(qno);
        ifh1 = rx_word(qno);
        ifh2 = rx_word(qno);
        ifh3 = rx_word(qno);

        memset(&rx_frame_ptr->header, 0, sizeof(vtss_packet_rx_header_t));
        rx_frame_ptr->header.port = VTSS_EXTRACT_BITFIELD(ifh2, 43 - 32, 4);
        //print_str("rx_frame_ptr->header.port: ");    print_dec(rx_frame_ptr->header.port);     print_cr_lf();
        rx_frame_ptr->header.vid  = VTSS_EXTRACT_BITFIELD(ifh3,  0, 12);
        //print_str("rx_frame_ptr->header.vid: ");    print_dec(rx_frame_ptr->header.vid);     print_cr_lf();
        /* Check actual extracted length vs. the length reported in the IFH */
        wlen = VTSS_EXTRACT_BITFIELD(ifh1, 71 - 64, 8);
        llen = VTSS_EXTRACT_BITFIELD(ifh1, 79 - 64, 6);
        ifh_reported_len_incl_fcs = 60 * wlen + llen - 80;
        //print_str("wlen: ");    print_dec(wlen);     print_cr_lf();
        //print_str("llen: ");    print_dec(llen);     print_cr_lf();
        //print_str("ifh_reported_len_incl_fcs: ");    print_dec(ifh_reported_len_incl_fcs);     print_cr_lf();


        packet = (ulong *) rx_frame_ptr->rx_packet;

        eof_flag = 0;
        escape_flag = 0;
        abort_flag = 0;
        pruned_flag = 0;

        while(!eof_flag) {
            *packet = rx_word(qno);
            //print_hex_dw(*packet); print_cr_lf();
            switch(*packet) {
            case XTR_NOT_READY:
                break;              /* Try again... */
            case XTR_ABORT:
                *packet = rx_word(qno); /* Unused */
                abort_flag = 1;
                eof_flag = TRUE;
                //print_str("XTR_ABORT: "); 
                break;
            case XTR_EOF_3:
                *packet = rx_word(qno);
                rx_frame_ptr->total_bytes += 1;
                eof_flag = TRUE;
                //print_str("XTR_EOF_3: ");
                break;
            case XTR_EOF_2:
                *packet = rx_word(qno);
                rx_frame_ptr->total_bytes += 2;
                eof_flag = TRUE;
                //print_hex_dw(*packet); print_cr_lf();
                //print_str("XTR_EOF_2: ");
                break;
            case XTR_EOF_1:
                *packet = rx_word(qno);
                rx_frame_ptr->total_bytes += 3;
                eof_flag = TRUE;
                //print_str("XTR_EOF_1: ");
                break;
            case XTR_PRUNED:
                pruned_flag = 1; /* But get the last 4 bytes as well */
                //print_str("XTR_PRUNED: ");
                /* FALLTHROUGH */
            case XTR_EOF_0:
                eof_flag = TRUE;
                //print_str("XTR_EOF_0: ");
                /* FALLTHROUGH */
            case XTR_ESCAPE:
                *packet = rx_word(qno);
                /* FALLTHROUGH */
            default:
#if defined(VTSS_ARCH_LUTON26)                
                if(rx_frame_ptr->total_bytes == 12) {

                    if((eth_hdr->dest.addr[0] == 0x01) &&
                            (eth_hdr->dest.addr[1] == 0x80) &&
                            (eth_hdr->dest.addr[2] == 0xc2) &&
                            (eth_hdr->dest.addr[3] == 0x00) &&
                            (eth_hdr->dest.addr[4] == 0x00)) {
#if TRANSIT_LLDP
                        if(eth_hdr->dest.addr[5] != 0x0e) { // LLDP frame
                            h2_rx_frame_discard(qno);
                            abort_flag = 1;
                            goto discard_packet;
                        }
#endif
                    }
                }
#endif //defined(VTSS_ARCH_LUTON26)               
                rx_frame_ptr->total_bytes += 4;
                packet++;
            }
        }

        if(eof_flag) {
            rx_frame_ptr->discard = 0;
        }

        if(pruned_flag) {
            rx_frame_ptr->pruned = 1;
        }
#if 0        
        print_str("total: ");    print_dec(rx_frame_ptr->total_bytes);     print_cr_lf();
        if (rx_frame_ptr->total_bytes != ifh_reported_len_incl_fcs) {
             println_str("total_bytes != ifh_reported_len_incl_fcs");
       }    
#endif 

#if defined(VTSS_ARCH_LUTON26)          
#if TRANSIT_LLDP
discard_packet:
#endif //TRANSIT_LLDP
#endif //VTSS_ARCH_LUTON26 

        if(abort_flag || !eof_flag) {
            h2_discard_frame(rx_frame_ptr);
            return;
        }
    }
}
#elif defined(VTSS_ARCH_LUTON26)
void h2_rx_frame_get (const uchar qno, vtss_rx_frame_t xdata * rx_frame_ptr)
/* ------------------------------------------------------------------------ --
 * Purpose     : Receive frame.
 * Remarks     : The structure pointed to by rx_frame_ptr is updated with the
 *               data received, see h2packet.h for a description of the structure.
 * Restrictions: Only to be called if h2_frame_received has returned TRUE.
 * See also    :
 * Example     :
 ****************************************************************************/
{
    ulong  qstat;
    ulong  ifh0, ifh1;
    ulong  xdata *packet;
    uchar  eof_flag, escape_flag, abort_flag, pruned_flag;

#define MAX_LENGTH (RECV_BUFSIZE+2)

    vtss_eth_hdr xdata * eth_hdr = rx_frame_ptr->rx_packet;
    mac_addr_t self_mac_addr;

    get_mac_addr(SYSTEM_MAC_ADDR, self_mac_addr);

    h2_discard_frame(rx_frame_ptr);

    H2_READ(VTSS_DEVCPU_QS_XTR_XTR_DATA_PRESENT, qstat);

    if(test_bit_32(qno, &qstat)) {
        ifh0 = rx_word(qno);
        ifh1 = rx_word(qno);

        memset(&rx_frame_ptr->header, 0, sizeof(vtss_packet_rx_header_t));
        rx_frame_ptr->header.port = IFH_GET(ifh0, ifh1, PORT);
        rx_frame_ptr->header.vid  = IFH_GET(ifh0, ifh1, VID);

        packet = (ulong *) rx_frame_ptr->rx_packet;

        eof_flag = 0;
        escape_flag = 0;
        abort_flag = 0;
        pruned_flag = 0;

        while(!eof_flag) {
            *packet = rx_word(qno);
            switch(*packet) {
            case XTR_NOT_READY:
                break;              /* Try again... */
            case XTR_ABORT:
                *packet = rx_word(qno); /* Unused */
                abort_flag = 1;
                eof_flag = TRUE;
                break;
            case XTR_EOF_3:
                *packet = rx_word(qno);
                rx_frame_ptr->total_bytes += 1;
                eof_flag = TRUE;
                break;
            case XTR_EOF_2:
                *packet = rx_word(qno);
                rx_frame_ptr->total_bytes += 2;
                eof_flag = TRUE;
                break;
            case XTR_EOF_1:
                *packet = rx_word(qno);
                rx_frame_ptr->total_bytes += 3;
                eof_flag = TRUE;
                break;
            case XTR_PRUNED:
                pruned_flag = 1; /* But get the last 4 bytes as well */
                /* FALLTHROUGH */
            case XTR_EOF_0:
                eof_flag = TRUE;
                /* FALLTHROUGH */
            case XTR_ESCAPE:
                *packet = rx_word(qno);
                /* FALLTHROUGH */
            default:
                if(rx_frame_ptr->total_bytes == 12) {

                    if((eth_hdr->dest.addr[0] == 0x01) &&
                            (eth_hdr->dest.addr[1] == 0x80) &&
                            (eth_hdr->dest.addr[2] == 0xc2) &&
                            (eth_hdr->dest.addr[3] == 0x00) &&
                            (eth_hdr->dest.addr[4] == 0x00)) {
#if TRANSIT_LLDP
                        if(eth_hdr->dest.addr[5] != 0x0e) { // LLDP frame
                            h2_rx_frame_discard(qno);
                            abort_flag = 1;
                            goto discard_packet;
                        }
#endif // TRANSIT_LLDP
                    }

                    if(mac_cmp(eth_hdr->src.addr, self_mac_addr) == 0) {
#if TRANSIT_LOOPDETECT
#if LOOPBACK_TEST
                        if (eth_hdr->type != 0x8809)
#endif
                            ldet_add_cpu_found(rx_frame_ptr->header.port);  //Local loop was found on port
#endif
                    }


                }
                rx_frame_ptr->total_bytes += 4;
                packet++;
            }
        }

        if(eof_flag) {
            rx_frame_ptr->discard = 0;
        }

        if(pruned_flag) {
            rx_frame_ptr->pruned = 1;
        }

#if TRANSIT_LLDP
discard_packet:
#endif
        if(abort_flag || !eof_flag) {
            h2_discard_frame(rx_frame_ptr);
            return;
        }
    }
}
#endif

#if defined(VTSS_ARCH_OCELOT)
bool h2_tx_frame_port(const uchar port_no,
                      const uchar *const frame,
                      const ushort length,
                      const vtss_vid_t vid)
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
    ulong ifh0 = 0, ifh1 = 0, ifh2 = 0, ifh3 = 0, value;
    port_bit_mask_t  dest_port_mask = 0;
    vtss_vid_t tx_vid = vid;

    const ulong *bufptr = (ulong *) frame;
    ulong        buflen = length, count, w, last, val;

    uchar qno;
    
    //print_str("port_no: ");    print_dec(port_no);     print_cr_lf();
    //print_str("length: ");    print_dec(length);     print_cr_lf();
    //print_str("vid: ");    print_dec(vid);     print_cr_lf();   

    /* ifh0 bit96 - 127 */
    ifh0 = VTSS_ENCODE_BITFIELD(1, 127 - 96, 1); /* BYPASS */
           
    /* ifh1 bit64 - 95 */
    /* DEST */
    // 64(p8) 65(p9) 66(p10) 67(p11) 
    if (port_no >= 8) {
        ifh1 = VTSS_ENCODE_BITFIELD(1, (port_no - 8), 1);  /* DEST */;
    }
    
    /* ifh2 bit32 - 63 */
    /* DEST */
    // 56(p0) 57(p1) 58(p2) 59(p3) 60(p4) 61(p5) 62(p6) 63(p7) 
    if (port_no < 8) {
        value = VTSS_BIT(port_no);
        ifh2 = VTSS_ENCODE_BITFIELD(VTSS_BIT(port_no), 56-32, VTSS_BIT(port_no));  /* DEST */
    }
    
    /* ifh3 bit0 - 31 */
    //ifh3 = 0;
     ifh3 = VTSS_ENCODE_BITFIELD(3, 28, 2) | /* POP_CNT=3 disables rewriter */
            VTSS_ENCODE_BITFIELD(7, 13, 3) | /* PCP=3 high queue */
            VTSS_ENCODE_BITFIELD(7, 17, 3); /* QoS_Class=7 high queue  */
    
    /* Select a tx queue */
    for(qno = VTSS_PACKET_TX_QUEUE_START; qno < VTSS_PACKET_TX_QUEUE_END; qno++) {
        if(fifo_status(qno))
            break;
    }
    if(qno == VTSS_PACKET_TX_QUEUE_END)
    {
         println_str(VTSS_PACKET_TX_QUEUE_END);
         return FALSE; // No tx queue available.
    }

#if defined(VTSS_ARCH_OCELOT)
    H2_WRITE(VTSS_DEVCPU_QS_INJ_INJ_CTRL(qno), VTSS_F_DEVCPU_QS_INJ_INJ_CTRL_GAP_SIZE(1) | VTSS_F_DEVCPU_QS_INJ_INJ_CTRL_SOF(1));
#elif defined(VTSS_ARCH_LUTON26)
    H2_WRITE(VTSS_DEVCPU_QS_INJ_INJ_CTRL(qno), VTSS_F_DEVCPU_QS_INJ_INJ_CTRL_SOF);
#endif
    tx_word(qno, ifh0);
    tx_word(qno, ifh1);
    tx_word(qno, ifh2);
    tx_word(qno, ifh3);

    //print_str("qno: ");   print_dec(qno);     print_cr_lf();   
    //print_str("ifh0:0x ");   print_hex_dw(ifh0);     print_cr_lf();   
    //print_str("ifh1:0x ");   print_hex_dw(ifh1);     print_cr_lf();   
    //print_str("ifh2:0x ");   print_hex_dw(ifh2);     print_cr_lf();   
    //print_str("ifh3:0x ");   print_hex_dw(ifh3);     print_cr_lf();   


    count = buflen / 4;
    last  = buflen % 4;

    w = 0;
    while(count) {
        if (w == 3 && vid != VTSS_VID_NULL) {
            /* Insert C-tag */
            val = ushorts2ulong(0x8100, vid);
            tx_word(qno, val);
            //print_hex_dw (val);print_spaces(2);
            w++;
        }
        val = *bufptr;
        tx_word(qno, val);
        //print_hex_dw (val);print_spaces(2);
        bufptr++;
        count--;
        w++;
    }

    if(last) {
        val = *bufptr;
        tx_word(qno, val);
        //print_hex_dw (val);print_spaces(2);
        w++;
    }

    /* Add padding */
    while (w < 15 /*(60/4)*/ ) {
        tx_word(qno, 0);
        //print_hex_dw (0);print_spaces(2);
        w++;
    }

    /* Indicate EOF and valid bytes in last word */
#if defined(VTSS_ARCH_OCELOT)
    H2_WRITE(VTSS_DEVCPU_QS_INJ_INJ_CTRL(qno),
             VTSS_F_DEVCPU_QS_INJ_INJ_CTRL_VLD_BYTES(length < 60 ? 0 : last) |
             VTSS_M_DEVCPU_QS_INJ_INJ_CTRL_EOF);
#elif defined(VTSS_ARCH_LUTON26)
    H2_WRITE(VTSS_DEVCPU_QS_INJ_INJ_CTRL(qno),
             VTSS_F_DEVCPU_QS_INJ_INJ_CTRL_VLD_BYTES(length < 60 ? 0 : last) |
             VTSS_F_DEVCPU_QS_INJ_INJ_CTRL_EOF);
#endif

    /* Add dummy CRC */
    tx_word(qno, 0);
    //print_hex_dw (0);print_spaces(2); print_cr_lf();
    return TRUE;
}
#elif defined(VTSS_ARCH_LUTON26)
bool h2_tx_frame_port(const uchar port_no,
                      const uchar *const frame,
                      const ushort length,
                      const vtss_vid_t vid)
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
    ulong ifh0 = 0, ifh1 = 0;
    port_bit_mask_t  dest_port_mask = 0;
    vtss_vid_t tx_vid = vid;

    const ulong *bufptr = (ulong *) frame;
    ulong        buflen = length, count, w, last, val;

    uchar qno;

#if 0
    WRITE_PORT_BIT_MASK(port_no, 1, &dest_port_mask);

    /* Calculate and send IFH0 and IFH1 */
    /* In order for IFH-DEST-field, then set the BYPASS. */
    IFH_PUT(ifh0, ifh1, BYPASS, 1);
    IFH_PUT(ifh0, ifh1, DEST, dest_port_mask);
#endif

    ifh0 = VTSS_ENCODE_BITFIELD(1, 63 - 32, 1) | /* BYPASS */
           VTSS_ENCODE_BITFIELD(1, port_no, 1);  /* DEST */

    ifh1 = VTSS_ENCODE_BITFIELD(3, 28, 2); /* POP_CNT=3 disables rewriter */

    /* Select a tx queue */
    for(qno = VTSS_PACKET_TX_QUEUE_START; qno < VTSS_PACKET_TX_QUEUE_END; qno++) {
        if(fifo_status(qno))
            break;
    }
    if(qno == VTSS_PACKET_TX_QUEUE_END) return FALSE; // No tx queue available.

#if defined(VTSS_ARCH_OCELOT)
    H2_WRITE(VTSS_DEVCPU_QS_INJ_INJ_CTRL(qno), VTSS_F_DEVCPU_QS_INJ_INJ_CTRL_SOF(1));
#elif defined(VTSS_ARCH_LUTON26)
    H2_WRITE(VTSS_DEVCPU_QS_INJ_INJ_CTRL(qno), VTSS_F_DEVCPU_QS_INJ_INJ_CTRL_SOF);
#endif
    tx_word(qno, ifh0);
    tx_word(qno, ifh1);

    count = buflen / 4;
    last  = buflen % 4;

    w = 0;
    while(count) {
        if (w == 3 && vid != VTSS_VID_NULL) {
            /* Insert C-tag */
            val = ushorts2ulong(0x8100, vid);
            tx_word(qno, val);
            w++;
        }
        val = *bufptr;
        tx_word(qno, val);
        bufptr++;
        count--;
        w++;
    }

    if(last) {
        val = *bufptr;
        tx_word(qno, val);
        w++;
    }

    /* Add padding */
    while (w < 15 /*(60/4)*/ ) {
        tx_word(qno, 0);
        w++;
    }

    /* Indicate EOF and valid bytes in last word */
#if defined(VTSS_ARCH_OCELOT)
    H2_WRITE(VTSS_DEVCPU_QS_INJ_INJ_CTRL(qno),
             VTSS_F_DEVCPU_QS_INJ_INJ_CTRL_VLD_BYTES(length < 60 ? 0 : last) |
             VTSS_M_DEVCPU_QS_INJ_INJ_CTRL_EOF);
#elif defined(VTSS_ARCH_LUTON26)
    H2_WRITE(VTSS_DEVCPU_QS_INJ_INJ_CTRL(qno),
             VTSS_F_DEVCPU_QS_INJ_INJ_CTRL_VLD_BYTES(length < 60 ? 0 : last) |
             VTSS_F_DEVCPU_QS_INJ_INJ_CTRL_EOF);
#endif

    /* Add dummy CRC */
    tx_word(qno, 0);

    return TRUE;
}
#endif





void h2_discard_frame( vtss_rx_frame_t xdata * rx_frame_ptr)
{
    rx_frame_ptr->total_bytes = 0;
    rx_frame_ptr->discard = 1;
    rx_frame_ptr->pruned = 0;
}

static void tx_word (uchar qno, ulong value)
/* ------------------------------------------------------------------------ --
 * Purpose     : Write a 32-bit chunk to transmit fifo.
 * Remarks     :
 * Restrictions: See h2_tx_init
 * See also    :
 * Example     :
 ****************************************************************************/
{
    H2_WRITE(VTSS_DEVCPU_QS_INJ_INJ_WR(qno), value);
}


static ulong rx_word (const uchar qno)
{
    ulong value;
    H2_READ(VTSS_DEVCPU_QS_XTR_XTR_RD(qno), value);
    return value;
}

static bool fifo_status(uchar qno)
{
    ulong qstat;
    bool  wmark;
    bool  ready;

    H2_READ(VTSS_DEVCPU_QS_INJ_INJ_STATUS, qstat);

    //print_str("VTSS_DEVCPU_QS_INJ_INJ_STATUS: ");    print_dec(qstat);     print_cr_lf();
    wmark = test_bit_32((qno+4), &qstat);
    ready = test_bit_32((qno+2), &qstat);

    return (!wmark & ready);
}

#endif //__BASIC_TX_RX__