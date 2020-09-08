//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#include "common.h"     /* Always include common.h at the first place of user-defined herder files */

#include <stdio.h>
#include <string.h>
#include "vtss_api_base_regs.h"
#include "hwport.h"
#include "txt.h"
#include "h2io.h"
#include "h2.h"
#include "h2txrx.h"
#include "phydrv.h"
#include "phymap.h"
#include "timer.h"
#include "main.h"
#include "print.h"
#include "phytsk.h"
#include "h2txrxaux.h"
#include "vtss_common_os.h"
#if TRANSIT_LLDP
#include "lldp.h"
#endif


#if PKTTX_TEST

/*****************************************************************************
 *
 *
 * Defines
 *
 *
 *
 ****************************************************************************/

#define PACKET_LENGTH 64 // include dummy CRC

#define pac_len_t uchar
//#define pac_len_t ushort

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

static uchar test_tx_rx (uchar rx_port_no, uchar tx_port_no);
static void  send_packet (uchar port_no);

#if defined(VTSS_ARCH_LUTON26)
static uchar get_and_check_packet (uchar rx_port_no);
#endif // LUTON26_L10 || LUTON26_L16 || LUTON26_L25

static void  init_port (uchar port_no, uchar mode);
static void  init_phys_forced (uchar port_no_1, uchar port_no_2, uchar mode);

/*****************************************************************************
 *
 *
 * Local data
 *
 *
 *
 ****************************************************************************/

// Definition for multicast protocol frame
const uchar  vtss_slow_protocol_addr[6]    = {0x01,0x80,0xC2,0x00,0x00,0x0E}; // shared with LLDP frame MAC address
const uchar  vtss_slow_protocol_type[2]    = {0x88, 0x09};
const uchar  vtss_oui[3]                   = {0x00, 0x01, 0xC1 } ;

static uchar tx_buf[PACKET_LENGTH];


/* ************************************************************************ */
void perform_tx_rx_test (uchar port_no, uchar speed, uchar external_loopback)
/* ------------------------------------------------------------------------ --
 * Purpose     : Perform internal or pair-wise external loopback on ports/PHYs. Dump
 *               results to RS-232.
 * Remarks     : If port_no is in range 0-15, do loopback on this port and, in case
 *               of external loopback, on its counterpart as well. Otherwise do
 *               loopback on all ports.
 *               If speed is in range 0-3, loopback is performed at one speed only
 *               (0 = 10 Mbit/s, 1 = 100 Mbit/s, 2 = 1000 Mbit/s, 3 = TBI). Otherwise
 *               run test at all speeds.
 *               external_loopback selects between internal (0) and external (1)
 *               loopback.
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
#if defined(VTSS_ARCH_OCELOT)
    // Ferret: Use this API do tx packtes test
    // Only the parameter of port_no is used.
    // 0xff means to all ports. It's iport here.
    if(speed || external_loopback); //Make compiler happy
    test_tx_rx(port_no, 0);
#elif defined(VTSS_ARCH_LUTON26)
    uchar port_no_stop;
    uchar mode;
    uchar mode_mask;
    uchar result [2];
    uchar result_tmp;
    uchar j;
    uchar loop_count_1;
    uchar loop_count_2;
    uchar port_no_1;
    uchar port_no_2;
    uchar phy_speed_fdx;
    ushort reg_val;

#define NO_OF_MODES 3
#define MODE_MASK_ALL   0x07 /* 10, 100 and 1000 */

    /*
    ** Display headers.
    */
    if (external_loopback) {
        print_str(txt_09); /* "External " */
    } else {
        print_str(txt_08); /* "Internal " */
    }
    print_str(txt_11); /* header with speed-columns */
    /*
    ** Determine ports to run test on
    */
    if (port_no > NO_OF_BOARD_PORTS) {
        port_no = 1;
        port_no_stop = NO_OF_BOARD_PORTS;
    } else {
        if (external_loopback) {
            if (((port_no & 0x01) == 0) && (port_no > 0)) {
                port_no--;
            }
            port_no_stop = port_no + 1;
        } else {
            port_no_stop = port_no;
        }
    }

    /*
    ** Determine speeds to run test at
    */
    if (speed == 0xff) {
        mode_mask = MODE_MASK_ALL;
    } else {
        mode_mask = 1 << speed;
    }


    /*
    ** Run through ports (2 ports at each run if external loopback)
    */
    do {
        port_no_1 = uport2cport(port_no);
        if (external_loopback) {
            if (port_no < NO_OF_BOARD_PORTS) {
                port_no_2 = uport2cport(port_no + 1);
            } else {
                port_no_2 = MAX_PORT;
            }
        }
        /*
        ** Run through speeds
        */
        memset(result, 0xff, sizeof(result));
        for (mode = (NO_OF_MODES - 1); mode != 0xff; mode--) {
            if (mode_mask & (1 << mode)) {
                if (mode <= 2) {
                    if (phy_map(port_no_1) && ((phy_map(port_no_2) && port_no_2 < MAX_PORT) || !external_loopback)) {

                        if (external_loopback) {
                            if (mode < 2) {
                                init_phys_forced(port_no_1, port_no_2, mode);

                                loop_count_1 = 0;
                                loop_count_2 = 0;
                                while ((!phy_link_status(port_no_1) || !phy_link_status(port_no_2)) &&
                                        (loop_count_2 < 3)) {

                                    delay_1(10);
                                    loop_count_1++;
                                    if (loop_count_1 >= 250) {
                                        init_phys_forced(port_no_1, port_no_2, mode);
                                        loop_count_2++;
                                        loop_count_1 = 0;
                                    }
                                }
                            }
                        } else {
#if 0
                            print_str("\r\nport no ");
                            print_dec(port_no_1);
                            print_str(" set forced speed, lm = ");
                            print_hex_prefix();
                            print_hex_b(mode);
                            print_cr_lf();
#endif
                            phy_set_forced_speed(port_no_1, mode | LINK_MODE_FDX_MASK | LINK_MODE_INT_LOOPBACK);
#if 0
                            print_str("reg 0 ");
                            reg_val = phy_read(port_no_1, 0);
                            print_hex_prefix();
                            print_hex_w(reg_val);
                            print_cr_lf();
#endif
                            while(!phy_link_status(port_no_1))
                                delay_1(2);
                        }


                        reg_val = phy_read(port_no_1, 28);

                        if (!(reg_val & 0x8000) && external_loopback) {
                            print_str("\r\nauto neg. incomplete\r\n");
                        }

                        phy_speed_fdx = ((uchar) reg_val >> 3) & 0x03;

                        if (reg_val & 0x20) {
                            phy_speed_fdx |= LINK_MODE_FDX_MASK;
                        }

                        if (phy_read(port_no_1, 5) & 0x0400) {
                            phy_speed_fdx |= LINK_MODE_PAUSE_MASK;
                        }



#if 0
                        print_str("phy reg 28 ");
                        print_hex_prefix();
                        print_hex_w(reg_val);
                        print_str(", speed = ");
                        print_dec(phy_speed_fdx);
                        print_str(", fdx = ");
                        if (reg_val & 0x20)
                            print_ch('1');
                        else
                            print_ch('0');
                        print_cr_lf();
#endif

                        init_port(port_no_1, phy_speed_fdx);
                        if (external_loopback) {
                            reg_val = phy_read(port_no_2, 28);

                            if (!(reg_val & 0x8000)) {
                                print_str("auto neg. incomplete\r\n");
                            }

                            phy_speed_fdx = ((uchar) reg_val >> 3) & 0x03;

                            if (reg_val & 0x20) {
                                phy_speed_fdx |= LINK_MODE_FDX_MASK;
                            }

                            if (phy_read(port_no_2, 5) & 0x0400) {
                                phy_speed_fdx |= LINK_MODE_PAUSE_MASK;
                            }

#if 0
                            print_str("phy reg 28 ");
                            print_hex_prefix();
                            print_hex_w(reg_val);
                            print_str(", speed = ");
                            print_dec(phy_speed_fdx);
                            print_str(", fdx = ");
                            if (reg_val & 0x20)
                                print_ch('1');
                            else
                                print_ch('0');
                            print_cr_lf();
#endif

                            init_port(port_no_2, phy_speed_fdx);
                        }
                        delay(MSEC_50);
                    } else {
                        continue;
                    }
                }

                if (external_loopback) {
                    result[0] = (result[0] & ~(0x03 << (mode << 1))) |
                                (test_tx_rx(port_no_1, port_no_2) << (mode << 1));
                    result[1] = (result[1] & ~(0x03 << (mode << 1))) |
                                (test_tx_rx(port_no_2, port_no_1) << (mode << 1)); /* swap ports */
                } else {
                    result[0] = (result[0] & ~(0x03 << (mode << 1))) |
                                (test_tx_rx(port_no_1, port_no_1) << (mode << 1));
                }
            }
        }

        /*
        ** Display results for 1 or 2 ports, and update port number
        */
        for (j = 0; (!external_loopback && j < 1) || (external_loopback && j < 2); j++) {
            print_cr_lf();
            print_str("    ");
#ifndef UNMANAGED_REDUCED_DEBUG_IF
            print_dec_8_right_2(port_no);
#else
            print_hex_b(port_no);
#endif
            print_str("    ");
            for (mode = 0; mode < NO_OF_MODES; mode++) {

                result_tmp = (result[j] >> (mode << 1)) & 0x03;

                if (result_tmp == 0x03) {
                    print_str("        "); /* spaces in column if speed not tested */
                } else if (result_tmp == 0) {
                    print_str(txt_12); /* ok txt */
                } else {
                    print_str(txt_13); /* error txt */
#ifndef UNMANAGED_REDUCED_DEBUG_IF
                    print_dec(result_tmp);
#else
                    print_hex_dw(result_tmp);
#endif
                    print_str(" ");
                }
            }

            /* Reset PHY and update port number */
            if (j == 0) {
                if (phy_map(port_no_1)) {
                    phy_restart(port_no_1);
                }
            } else {
                if (phy_map(port_no_2)) {
                    phy_restart(port_no_2);
                }
            }
            port_no++;
            if (port_no > port_no_stop) {
                break;
            }
        }

    } while (port_no <= port_no_stop);

    print_str(txt_14); /* completed message */
#endif
}

#if defined(VTSS_ARCH_LUTON26)
static void init_port (uchar port_no, uchar mode)
{
    /* Set MAC port in specific link speed and duplex. */
    h2_setup_port(port_no, mode);
}

static void init_phys_forced (uchar port_no_1, uchar port_no_2, uchar mode)
{
    phy_reset(port_no_1);
    phy_reset(port_no_2);
    delay(MSEC_50);
    phy_set_forced_speed(port_no_1, mode | LINK_MODE_FDX_MASK);
    phy_set_forced_speed(port_no_2, mode | LINK_MODE_FDX_MASK);
}
#endif

/* ************************************************************************ */
static uchar test_tx_rx (uchar rx_port_no, uchar tx_port_no)
/* ------------------------------------------------------------------------ --
 * Purpose     : Send a frame on tx_port_no and receive the frame on rx_port_no.
 * Remarks     :
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
#if defined(VTSS_ARCH_OCELOT)
    vtss_iport_no_t             iport_idx;
    vtss_cport_no_t             chip_port;

    // Only the parameter of rx_port_no is used.
    // It's iport here. 0xff means to all ports

    if(tx_port_no); //Make complier happy
    //print_str("rx_port_no: ");    print_dec(rx_port_no);     print_cr_lf();
    if (rx_port_no == 0xff) {
        for (iport_idx = MIN_PORT; iport_idx < MAX_PORT; iport_idx++) {
            chip_port = iport2cport(iport_idx);
            send_packet(chip_port);
        }
    } else {
       chip_port = iport2cport(rx_port_no);
       send_packet(chip_port);
    }

    return 1;

#elif defined(VTSS_ARCH_LUTON26)
    uchar result;
    uchar recv_q = 0;
    ulong captured = 0;
    uchar timeout;
#if !TRANSIT_BPDU_PASS_THROUGH
    uchar l2_type;
#endif


    /* Set analyzer to copy frame to CPU capture buffer */

#if !TRANSIT_BPDU_PASS_THROUGH
    for(l2_type = 0; l2_type < 0x10; l2_type++)
        h2_bpdu_t_registration(l2_type, FALSE);
#endif // !TRANSIT_BPDU_PASS_THROUGH

#if TRANSIT_LLDP
#if TRANSIT_BPDU_PASS_THROUGH
    if(lldp_os_get_admin_status(rx_port_no) == LLDP_ENABLED_RX_TX) {
        h2_bpdu_t_registration(0x0E, FALSE);
    }
#else
    h2_bpdu_t_registration(0x0E, FALSE);
#endif // TRANSIT_BPDU_PASS_THROUGH
#endif // TRANSIT_LLDP

    h2_bpdu_t_registration(0x0E, TRUE); // Capture LLDP MAC address frame
    H2_WRITE_MASKED(VTSS_DEVCPU_QS_XTR_XTR_QU_FLUSH, 0x0, 0x2);

    delay(MSEC_20);

    timeout = 0;
    do {

        h2_rx_frame_get(recv_q, &vtss_rx_frame);
        if(!vtss_rx_frame.discard) {
            break;
        }
        if(++timeout > 200) break;
        delay_1(2);
    } while (vtss_rx_frame.discard);
    if (timeout < 201) {
        if (get_and_check_packet(rx_port_no) == 0) {
            result = 0; /* pass */
        } else {
            result = 1; /* packet error */
        }
    } else {
        result = 2; /* timeout */
    }

    h2_rx_flush();


    /* Set analyzer to not copy frame to CPU capture buffer */
    h2_bpdu_t_registration(0x0E, FALSE);

    H2_WRITE_MASKED(VTSS_DEVCPU_QS_XTR_XTR_QU_FLUSH, 0x2, 0x2);

#if !TRANSIT_BPDU_PASS_THROUGH
    for(l2_type = 0; l2_type < 0x10; l2_type++)
        h2_bpdu_t_registration(l2_type, TRUE);
#endif // !TRANSIT_BPDU_PASS_THROUGH

#if TRANSIT_LLDP
#if TRANSIT_BPDU_PASS_THROUGH
    if(lldp_os_get_admin_status(rx_port_no) == LLDP_ENABLED_RX_TX) {
        h2_bpdu_t_registration(0x0E, TRUE);
    }
#else
    h2_bpdu_t_registration(0x0E, TRUE);
#endif // TRANSIT_BPDU_PASS_THROUGH
#endif // TRANSIT_LLDP


    return result;

#endif // FERRET
}

/* ************************************************************************ */
static void send_packet (uchar port_no)
/* ------------------------------------------------------------------------ --
 * Purpose     : Send packet.
 * Remarks     :
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    uchar ret = FALSE;


    /* See packet definition in AS0036 */

    /* Set the destination MAC Address to slow protocol multicast address */
    VTSS_COMMON_MACADDR_ASSIGN(&tx_buf[0], vtss_slow_protocol_addr);

    /* Set the source MAC Address */
    get_mac_addr(SYSTEM_MAC_ADDR, &tx_buf[6]);

    /* Set type length field */
    memcpy(&tx_buf[12], vtss_slow_protocol_type, 2);

    /* Set subtype field */
    tx_buf[14] = 0x0A;

    /* Set OUI */
    memcpy(&tx_buf[15],&vtss_oui[0],3);
    tx_buf[18] = 0x00;
    tx_buf[19] = 0x01;

    tx_buf[20] = 0;
    tx_buf[21] = port_no;
    tx_buf[22] = 's';
    tx_buf[23] = 'p';
    tx_buf[24] = 'e';
    tx_buf[25] = 'e';
    tx_buf[26] = 'd';
    tx_buf[27] = '=';
    tx_buf[28] = '\0';

    ret = h2_tx_frame_port(port_no, tx_buf, (PACKET_LENGTH - 4), VTSS_VID_NULL);
    if(!ret) {
        print_str("send packet error");
        print_cr_lf();
    }
}

#if defined(VTSS_ARCH_LUTON26)
/* ************************************************************************ */
static uchar get_and_check_packet (uchar rx_port_no)
/* ------------------------------------------------------------------------ --
 * Purpose     : Get and check packet.
 * Remarks     :
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    pac_len_t j;
    uchar error;

    error = FALSE;

    /* Check length and source port*/
    if(vtss_rx_frame.total_bytes != PACKET_LENGTH) {
        error = TRUE;
    }
    if(vtss_rx_frame.header.port != rx_port_no) {
        error = TRUE;
    }
    /* Read and check packet */
    if (!error) {
        for (j = 0; j < (PACKET_LENGTH - 4); j++) {
            if (tx_buf[j] != vtss_rx_frame.rx_packet[j]) {
                error = TRUE;
            }
        }
    }
    VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_DEBUG, ("Port %u Rx Test Frame, length=%u, tx from %u",
                      (unsigned)cport2uport(vtss_rx_frame.header.port),
                      (unsigned)vtss_rx_frame.total_bytes));
#ifndef VTSS_COMMON_NDEBUG
    if(error)
        vtss_common_dump_frame(vtss_rx_frame.rx_packet ,vtss_rx_frame.total_bytes);
#endif
    return error;

}
#endif // LUTON26_L10 || LUTON26_L16 || LUTON26_L25

#endif // PKTTX_TEST
