//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#include "common.h"     /* Always include common.h at the first place of user-defined herder files */
#include "vtss_api_base_regs.h"
#include "h2io.h"
#include "h2stats.h"
#include "txt.h"
#include "print.h"

#if UNMANAGED_PORT_STATISTICS_IF

/*****************************************************************************
 *
 *
 * Defines
 *
 *
 *
 ****************************************************************************/


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
BOOL GPARM_break_show_statistic_flag = FALSE;

/*****************************************************************************
 *
 *
 * Local data
 *
 *
 *
 ****************************************************************************/


static const ushort tx_counters[6] = {  CNT_TX_64,
                                        CNT_TX_65_TO_127,
                                        CNT_TX_128_TO_255,
                                        CNT_TX_256_TO_511,
                                        CNT_TX_512_TO_1023,
                                        CNT_TX_1024_TO_1526 };


static const ushort rx_counters[6] = {  CNT_RX_64,
                                        CNT_RX_65_TO_127,
                                        CNT_RX_128_TO_255,
                                        CNT_RX_256_TO_511,
                                        CNT_RX_512_TO_1023,
                                        CNT_RX_1024_TO_1526 };

static ulong _h2_stats_counter_get_private(vtss_cport_no_t chip_port, port_statistics_t counter_id)
{
    ulong temp = 0, tgt;
#if defined(VTSS_ARCH_LUTON26)
    ulong port_offset;
#endif // VTSS_ARCH_LUTON26

    /* Reserved */
    if (counter_id >= 0x7000) {
        return 0;
    }

    // EA = 0; // Disable interrupt while reading the port statistics.
#if defined(VTSS_ARCH_LUTON26)
    if (counter_id >= 0xC00) {
        port_offset = 18UL * ((ulong) chip_port);
    } else if (counter_id >= 0x800) {
        port_offset = 31UL * ((ulong) chip_port);
    } else {
        port_offset = 43UL * ((ulong) chip_port);
    }

    tgt = VTSS_SYS_STAT_CNT(counter_id + port_offset);
    H2_READ(tgt, temp);

#elif defined(VTSS_ARCH_OCELOT)
    //select the port to be read.
    H2_WRITE_MASKED(VTSS_SYS_SYSTEM_STAT_CFG,
                    VTSS_F_SYS_SYSTEM_STAT_CFG_STAT_VIEW(chip_port),
                    VTSS_M_SYS_SYSTEM_STAT_CFG_STAT_VIEW);
    tgt = VTSS_SYS_STAT_CNT(counter_id);
    H2_READ(tgt, temp);
#endif

    // EA = 1; // Enable interrupt
    return temp;
}

ulong h2_stats_counter_get(vtss_cport_no_t chip_port, port_statistics_t counter_id)
{
    ulong cnt;
    uchar i;
    ushort*p;

    if (counter_id == CNT_TX_PKTS) {
        p = &tx_counters[0];
    } else {
        if (counter_id == CNT_RX_PKTS) {
            p = &rx_counters[0];
        } else {
            return _h2_stats_counter_get_private( chip_port, counter_id);
        }
    }

    /*
     * For Luton26 there is no counter that counts all frames,
     * so we have to calculate it ourselves.
     */

    for (i = 0, cnt = 0; i < 6; i++) {
        cnt += _h2_stats_counter_get_private(chip_port, p[i]);
    }

    return cnt;
}

void h2_stats_counter_clear(vtss_cport_no_t chip_port)
{
#if defined(VTSS_ARCH_OCELOT)
    H2_WRITE(VTSS_SYS_SYSTEM_STAT_CFG,
             VTSS_F_SYS_SYSTEM_STAT_CFG_STAT_CLEAR_SHOT(0x7) | VTSS_F_SYS_SYSTEM_STAT_CFG_STAT_VIEW(chip_port));
#endif // VTSS_ARCH_OCELOT
}

/* ************************************************************************ */
void print_port_statistics(vtss_cport_no_t chip_port)
/* ------------------------------------------------------------------------ --
 * Purpose     :
 * Remarks     :
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
#if UNMANAGED_PORT_STATISTICS_QOS
#define NO_OF_LINES 44
#define QOS_COUNTERS_START 21
#else
#define NO_OF_LINES 20
#endif // UNMANAGED_PORT_STATISTICS_QOS
#define SIZE_COUNTERS_START   7
#define ERROR_COUNTERS_START 13

    code std_txt_t prefix_txt [2] = {TXT_NO_RX_PREFIX , TXT_NO_TX_PREFIX};
    code struct {
        std_txt_t txt_no [2];
        port_statistics_t counter_id [2];
    }  display_tab_1 [NO_OF_LINES] = {
        TXT_NO_PACKETS,         TXT_NO_PACKETS,         CNT_RX_PKTS,            CNT_TX_PKTS,
        TXT_NO_OCTETS,          TXT_NO_OCTETS,          CNT_RX_OCTETS,          CNT_TX_OCTETS,
        TXT_NO_BC_PACKETS,      TXT_NO_BC_PACKETS,      CNT_RX_BCAST_PKTS,      CNT_TX_BCAST_PKTS,
        TXT_NO_MC_PACKETS,      TXT_NO_MC_PACKETS,      CNT_RX_MCAST_PKTS,      CNT_TX_MCAST_PKTS,
        TXT_NO_PAUSE,           TXT_NO_PAUSE,           CNT_RX_PAUSE,           CNT_TX_PAUSE,
        TXT_NO_ERR_PACKETS,     TXT_NO_ERR_PACKETS,     CNT_RX_CRC_ALIGN_ERRS,  CNT_TX_DROP,
        TXT_NO_MAC_CTRL,        TXT_NO_DASH,            CNT_RX_CTRL,            COUNTER_NONE,

        TXT_NO_64_BYTES,        TXT_NO_64_BYTES,        CNT_RX_64,              CNT_TX_64,
        TXT_NO_65_BYTES,        TXT_NO_65_BYTES,        CNT_RX_65_TO_127,       CNT_TX_65_TO_127,
        TXT_NO_128_BYTES,       TXT_NO_128_BYTES,       CNT_RX_128_TO_255,      CNT_TX_128_TO_255,
        TXT_NO_256_BYTES,       TXT_NO_256_BYTES,       CNT_RX_256_TO_511,      CNT_TX_256_TO_511,
        TXT_NO_512_BYTES,       TXT_NO_512_BYTES,       CNT_RX_512_TO_1023,     CNT_TX_512_TO_1023,
        TXT_NO_1024_BYTES,      TXT_NO_1024_BYTES,      CNT_RX_1024_TO_1526,    CNT_TX_1024_TO_1526,
        TXT_NO_CRC_ALIGN,       TXT_NO_COLLISIONS,      CNT_RX_CRC_ALIGN_ERRS,  CNT_TX_COLLISIONS,
        TXT_NO_UNDERSIZE,       TXT_NO_DROPS,           CNT_RX_UNDERSIZE_PKTS,  CNT_TX_DROP,
        TXT_NO_OVERSIZE,        TXT_NO_OVERFLOW,        CNT_RX_OVERSIZE_PKTS,   CNT_DROP_TAIL,
        TXT_NO_FRAGMENTS,       TXT_NO_AGED,            CNT_RX_FRAGMENTS,       CNT_TX_AGED,
        TXT_NO_JABBERS,         TXT_NO_DASH,            CNT_RX_JABBERS,         COUNTER_NONE,
        TXT_NO_DROPS,           TXT_NO_DASH,            CNT_DROP_LOCAL,         COUNTER_NONE,
        TXT_NO_CAT_DROPS,       TXT_NO_DASH,            CNT_RX_CAT_DROP,        COUNTER_NONE,

#if UNMANAGED_PORT_STATISTICS_QOS
        TXT_NO_RED_PRIO_0,      TXT_NO_DASH,            CNT_RX_RED_PRIO_0,      COUNTER_NONE,
        TXT_NO_RED_PRIO_1,      TXT_NO_DASH,            CNT_RX_RED_PRIO_1,      COUNTER_NONE,
        TXT_NO_RED_PRIO_2,      TXT_NO_DASH,            CNT_RX_RED_PRIO_2,      COUNTER_NONE,
        TXT_NO_RED_PRIO_3,      TXT_NO_DASH,            CNT_RX_RED_PRIO_3,      COUNTER_NONE,
        TXT_NO_RED_PRIO_4,      TXT_NO_DASH,            CNT_RX_RED_PRIO_4,      COUNTER_NONE,
        TXT_NO_RED_PRIO_5,      TXT_NO_DASH,            CNT_RX_RED_PRIO_5,      COUNTER_NONE,
        TXT_NO_RED_PRIO_6,      TXT_NO_DASH,            CNT_RX_RED_PRIO_6,      COUNTER_NONE,
        TXT_NO_RED_PRIO_7,      TXT_NO_DASH,            CNT_RX_RED_PRIO_7,      COUNTER_NONE,

        TXT_NO_YELLOW_PRIO_0,   TXT_NO_YELLOW_PRIO_0,   CNT_RX_YELLOW_PRIO_0,   CNT_TX_YELLOW_PRIO_0,
        TXT_NO_YELLOW_PRIO_1,   TXT_NO_YELLOW_PRIO_1,   CNT_RX_YELLOW_PRIO_1,   CNT_TX_YELLOW_PRIO_1,
        TXT_NO_YELLOW_PRIO_2,   TXT_NO_YELLOW_PRIO_2,   CNT_RX_YELLOW_PRIO_2,   CNT_TX_YELLOW_PRIO_2,
        TXT_NO_YELLOW_PRIO_3,   TXT_NO_YELLOW_PRIO_3,   CNT_RX_YELLOW_PRIO_3,   CNT_TX_YELLOW_PRIO_3,
        TXT_NO_YELLOW_PRIO_4,   TXT_NO_YELLOW_PRIO_4,   CNT_RX_YELLOW_PRIO_4,   CNT_TX_YELLOW_PRIO_4,
        TXT_NO_YELLOW_PRIO_5,   TXT_NO_YELLOW_PRIO_5,   CNT_RX_YELLOW_PRIO_5,   CNT_TX_YELLOW_PRIO_5,
        TXT_NO_YELLOW_PRIO_6,   TXT_NO_YELLOW_PRIO_6,   CNT_RX_YELLOW_PRIO_6,   CNT_TX_YELLOW_PRIO_6,
        TXT_NO_YELLOW_PRIO_7,   TXT_NO_YELLOW_PRIO_7,   CNT_RX_YELLOW_PRIO_7,   CNT_TX_YELLOW_PRIO_7,

        TXT_NO_GREEN_PRIO_0,    TXT_NO_GREEN_PRIO_0,    CNT_RX_GREEN_PRIO_0,    CNT_TX_GREEN_PRIO_0,
        TXT_NO_GREEN_PRIO_1,    TXT_NO_GREEN_PRIO_1,    CNT_RX_GREEN_PRIO_1,    CNT_TX_GREEN_PRIO_1,
        TXT_NO_GREEN_PRIO_2,    TXT_NO_GREEN_PRIO_2,    CNT_RX_GREEN_PRIO_2,    CNT_TX_GREEN_PRIO_2,
        TXT_NO_GREEN_PRIO_3,    TXT_NO_GREEN_PRIO_3,    CNT_RX_GREEN_PRIO_3,    CNT_TX_GREEN_PRIO_3,
        TXT_NO_GREEN_PRIO_4,    TXT_NO_GREEN_PRIO_4,    CNT_RX_GREEN_PRIO_4,    CNT_TX_GREEN_PRIO_4,
        TXT_NO_GREEN_PRIO_5,    TXT_NO_GREEN_PRIO_5,    CNT_RX_GREEN_PRIO_5,    CNT_TX_GREEN_PRIO_5,
        TXT_NO_GREEN_PRIO_6,    TXT_NO_GREEN_PRIO_6,    CNT_RX_GREEN_PRIO_6,    CNT_TX_GREEN_PRIO_6,
        TXT_NO_GREEN_PRIO_7,    TXT_NO_GREEN_PRIO_7,    CNT_RX_GREEN_PRIO_7,    CNT_TX_GREEN_PRIO_7,
#endif // UNMANAGED_PORT_STATISTICS_QOS
    };
    uchar j;
    uchar c;
    uchar display_header;
    uchar indentation;
    std_txt_t rx_txt_no;
    uchar spaces;
    std_txt_t tx_txt_no;
    ulong reg_addr;
    ulong reg_val;

    for (j = 0; j < NO_OF_LINES; j++) {

        /*
        ** Possibly display counter block header
        */
        display_header = FALSE;
        if (j == 0) {
            /* Total counters */
            display_header = TRUE;
            indentation = 10;
            rx_txt_no   = TXT_NO_RX_TOTAL;
            spaces      = 25;
            tx_txt_no   = TXT_NO_TX_TOTAL;
        } else if (j == SIZE_COUNTERS_START) {
            /* Size counters */
            display_header = TRUE;
            indentation = 6;
            rx_txt_no   = TXT_NO_RX_SIZE;
            spaces      = 17;
            tx_txt_no   = TXT_NO_TX_SIZE;
        } else if (j == ERROR_COUNTERS_START) {
            /* Error counters */
            display_header = TRUE;
            indentation = 6;
            rx_txt_no   = TXT_NO_RX_ERROR;
            spaces      = 17;
            tx_txt_no   = TXT_NO_TX_ERROR;
#if UNMANAGED_PORT_STATISTICS_QOS
        } else if (j == QOS_COUNTERS_START) {
            /* QoS counters */
            display_header = TRUE;
            indentation = 6;
            rx_txt_no   = TXT_NO_RX_QOS;
            spaces      = 19;
            tx_txt_no   = TXT_NO_TX_QOS;
#endif // UNMANAGED_PORT_STATISTICS_QOS
        }

        if (display_header) {
            print_cr_lf();
            print_spaces(indentation);
            print_txt(rx_txt_no);
            print_spaces(spaces);
            print_txt(tx_txt_no);
            print_cr_lf();

            print_spaces(indentation);
            print_line(txt_len(rx_txt_no));
            print_spaces(spaces);
            print_line(txt_len(tx_txt_no));
            print_cr_lf();
        }

        /*
        ** Display 2 columns of counters
        */
        for (c = 0; c < 2; c++) {
            reg_addr = display_tab_1[j].counter_id[c];
            print_txt(prefix_txt[c & 0x01]);
            print_txt_left(display_tab_1[j].txt_no[c], 23);
            if (reg_addr != COUNTER_NONE) {
                reg_val = h2_stats_counter_get(chip_port, reg_addr);
                print_dec_right(reg_val);
            } else {
                print_str("         -");
            }
            print_spaces(2);
        }
        print_cr_lf();

        if (GPARM_break_show_statistic_flag) {
            GPARM_break_show_statistic_flag = FALSE;
            break;
        }
    }
}
#endif // UNMANAGED_PORT_STATISTICS_IF
