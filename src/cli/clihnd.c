//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#include <ctype.h>
#include <string.h>

#include "common.h"     /* Always include common.h at the first place of user-defined herder files */
#include "sysutil.h"
#include "hwport.h"
#if TRANSIT_EEE
#include "eee_api.h"
#include "eee_base_api.h"
#endif

#include "txt.h"
#include "h2io.h"
#include "uartdrv.h"
#include "main.h"
#include "print.h"
#include "h2.h"
#include "timer.h"
#include "phydrv.h"
#include "phymap.h"
#include "phytsk.h"
#include "phy_family.h"
#include "phy_base.h"
#include "txrxtst.h"
#include "misc1.h"
#include "clihnd.h"
#include "version.h"
#include "spiflash.h"
#include "vtss_api_base_regs.h"

#include "i2c_h.h"
#include "ledtsk.h"
#include "i2c.h"
#include "misc2.h"
#include "vtss_common_os.h"
#include "h2pcs1g.h"
#include "h2sdcfg.h"

#if TRANSIT_EEE
#include "eee_api.h"
#include "eee_base_api.h"
#endif

#if TRANSIT_FAN_CONTROL
#include "fan_api.h"
#endif

#if TRANSIT_LLDP
#include "lldp_remote.h"
#include "lldp_tlv.h"
#include "h2txrx.h"
#endif

#if TRANSIT_VERIPHY
#include "veriphy.h"
#endif

#if UNMANAGED_PORT_STATISTICS_IF
#include "h2stats.h"
#endif

#if TRANSIT_LAG
#include "h2aggr.h"
#endif

#if TRANSIT_LACP
#include "vtss_lacp.h"
#endif

#if TRANSIT_UNMANAGED_MAC_OPER_GET || TRANSIT_UNMANAGED_MAC_OPER_SET
#include "h2mactab.h"
#endif // TRANSIT_UNMANAGED_MAC_OPER_GET || TRANSIT_UNMANAGED_MAC_OPER_SET

#if UNMANAGED_TCAM_DEBUG_IF
#include "h2tcam.h"
#endif
#ifndef NO_DEBUG_IF

/*****************************************************************************
 *
 *
 * Defines
 *
 *
 *
 ****************************************************************************/

#define CLI_MAX_NO_OF_PARMS     4
#define CLI_MAX_CMD_LEN         48

#define FORMAT_OK    0
#define FORMAT_ERROR 1

#if TRANSIT_EEE
#define EEE_WAKEUP_TIME_MAX 1000
#define EEE_WAKEUP_TIME_MIN 0
#endif

/*****************************************************************************
 *
 *
 * Typedefs and enums
 *
 *
 *
 ****************************************************************************/

typedef struct {
    uchar *str;
    uchar len;
} str_parm_t;

/*****************************************************************************
 *
 *
 * Prototypes for local functions
 *
 *
 *
 ****************************************************************************/

static uchar handle_command (void);
static void  cli_show_sw_ver(void);
static uchar retrieve_parms (void);
#if !defined(BRINGUP)
static void cli_show_port_info(void);
#endif
#if TRANSIT_UNMANAGED_SYS_MAC_CONF
static uchar retrieve_str_parms (void);
#endif
static void  skip_spaces (void);
static uchar cmd_cmp (char *s1, char *s2) small;
#if TRANSIT_UNMANAGED_SYS_MAC_CONF
static uchar handle_sys_config (void);
#endif
#ifndef VTSS_COMMON_NDEBUG
static void update_debug_lvl(uchar lvl);
#endif
#if TRANSIT_FAN_CONTROL
#if UNMANAGED_FAN_DEBUG_IF
static void handle_fan_control (void);
#endif
#endif
#if TRANSIT_LLDP
#if UNMANAGED_LLDP_DEBUG_IF
static void cmd_print_lldp_remoteinfo (void);
#endif
#endif
#if TRANSIT_VERIPHY
static void cmd_run_veriphy(vtss_iport_no_t iport);
#endif
#if TRANSIT_UNMANAGED_MAC_OPER_GET
static void cli_show_mac_addr(vtss_cport_no_t chip_port);
#endif // TRANSIT_UNMANAGED_MAC_OPER_GET

/*****************************************************************************
 *
 *
 * Local data
 *
 *
 *
 ****************************************************************************/

static ulong idata parms [CLI_MAX_NO_OF_PARMS];
static str_parm_t str_parms [CLI_MAX_NO_OF_PARMS];
static uchar parms_no;

static uchar xdata cmd_buf [CLI_MAX_CMD_LEN];
static uchar xdata *cmd_ptr;
static uchar cmd_len = 0;

/* Common Sequence Mnemonic */
#define CLI_CTLC    0x03
#define CLI_CTLD    0x04
#define CLI_BEL     0x07
#define CLI_CTLH    0x08
#define CLI_BS      CLI_CTLH
#define CLI_LF      0x0a
#define CLI_CR      0x0d
#define CLI_ESC     0x1b
#define CLI_DEL     0x7f

#define CLI_SEQUENCE_INDICATOR     0x5b  // [
#define CLI_CURSOR_UP              0x41  // A
#define CLI_CURSOR_DOWN            0x42  // B
#define CLI_CURSOR_FORWARD         0x43  // C
#define CLI_CURSOR_BACKWARD        0x44  // D

/* Control Sequence Introducer (CSI)
 * An escape sequence that provides supplementary controls and is itself a
 * prefix affecting the interpretation of a limited number of contiguous
 * characters. In the VT100 the CSI is ESC [. */
#define CLI_CSI_1   CLI_ESC /* First character of CSI */
#define CLI_CSI_2   0x5b    /* Second character of CSI */
#define CLI_CUF     0x43    /* Cursor Forward, Sequence: Escape [ C */
#define CLI_CUB     0x44    /* Cursor Backward, Sequence: Escape [ D */
#define CLI_EL      0x4B    /* Erase in Line, Sequence: Escape [ K */


/* ************************************************************************ */
void cli_tsk (void)
/* ------------------------------------------------------------------------ --
 * Purpose     : Handle command line interface.
 * Remarks     :
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    uchar error_status;

    if (cmd_ready()) {
        cmd_ptr = &cmd_buf[0];
        skip_spaces();
#if TRANSIT_UNMANAGED_SWUP
        if (cmd_cmp(cmd_ptr, "ERASE") == 0) {
#ifndef VTSS_COMMON_NDEBUG
            vtss_os_trace_level = 0;
#endif //VTSS_COMMON_NDEBUG

            cmd_ptr += (sizeof("ERASE") - 1);
            if (retrieve_parms() != FORMAT_OK) {
                error_status = FORMAT_ERROR;
            } else {
#if FRONT_LED_PRESENT
                led_update_system(VTSS_LED_MODE_BLINK_GREEN);
#endif //FRONT_LED_PRESENT
                print_cr_lf();
                if (flash_erase_code((ulong) parms[0])) {
                    println_str("Fail");
                } else {
                    println_str("Done");
                }
                error_status = 0;
#if FRONT_LED_PRESENT
                led_update_system(VTSS_LED_MODE_ON_GREEN);
#endif //FRONT_LED_PRESENT
            }
        } else if (cmd_cmp(cmd_ptr, "PROGRAM") == 0) {

            cmd_ptr += (sizeof("PROGRAM") - 1);
            if (retrieve_parms() != FORMAT_OK) {
                error_status = FORMAT_ERROR;
            }
            else if (parms_no != 3) {
                error_status = FORMAT_ERROR;
            } else {
#ifndef VTSS_COMMON_NDEBUG
                vtss_os_trace_level = 0;
#endif //VTSS_COMMON_NDEBUG
#if FRONT_LED_PRESENT
                led_update_system(VTSS_LED_MODE_BLINK_GREEN);
#endif //FRONT_LED_PRESENT
                print_cr_lf();
                if (flash_download_image((ulong) parms[0], (ulong) parms[1], (uchar) parms[2])) {
                    println_str("Fail");
#if FRONT_LED_PRESENT
                    led_update_system(VTSS_LED_MODE_ON_GREEN);
#endif //FRONT_LED_PRESENT
                } else {
                    println_str("Done");
                    h2_reset();
                }
            }
        } else
#endif //TRANSIT_UNMANAGED_SWUP

#if TRANSIT_UNMANAGED_SYS_MAC_CONF
            /*
                config mac
                config mac xx-xx-xx-xx-xx-xx
                config dlevel error, warning, debug, noise
            */
            if(cmp_cmd_txt(CMD_TXT_NO_CONFIG, cmd_ptr)) {
                cmd_ptr += cmd_txt_tab[CMD_TXT_NO_CONFIG].min_match;
                error_status = handle_sys_config();
            } else
#endif

        if (cmd_buf[0] != CLI_CR && /* Null string */
            (error_status = handle_command()) != NULL) {
            /* empty queue */
            while (uart_byte_ready()) {
                (void) uart_get_byte();
            }
            /* Show error message */
            println_str("%% Invalid command");
        }

        cmd_len = 0;
        CLI_PROMPT();
    }
}

static void cli_csi_prefix(uchar ctrl_key) small
{
    uart_put_byte(CLI_CSI_1);
    uart_put_byte(CLI_CSI_2);
    uart_put_byte(ctrl_key);
}

static void cli_backspace(void) small
{
    cli_csi_prefix(CLI_CUB);
    cli_csi_prefix(CLI_EL);
}

/* ************************************************************************ */
bool cmd_ready (void)
/* ------------------------------------------------------------------------ --
 * Purpose     : Collect bytes received by uart driver and check if a command
 *               is ready (i.e. <CR> received).
 * Remarks     : Returns TRUE, if command is ready, otherwise FALSE.
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    uchar ch;

    if (uart_byte_ready()) {
        ch = uart_get_byte();

        if (ch != CLI_LF) { /* discard LF chars */
            if (ch == CLI_CTLH) { /* handle backspace char */
                if (cmd_len > 0) {
                    cmd_len--;
                    cli_backspace();
                }
            } else {
                if (ch == CLI_SEQUENCE_INDICATOR) {
                    delay_1(1); // Delay 1ms to check if cursor control character come along with the sequence indicator
                }
                if (ch == CLI_SEQUENCE_INDICATOR && uart_byte_ready()) {
                    ch = uart_get_byte();
                    if (ch == CLI_CURSOR_UP && cmd_len == 0) { // Only support cursor up while first character input.
                        for (cmd_len = 0; cmd_buf[cmd_len] > 31 && cmd_buf[cmd_len] < 128; cmd_len++) {
                            uart_put_byte(cmd_buf[cmd_len]);
                        }
                    }
                } else if (ch == CLI_CR) {
                    /* error handling: ensure that CR is present in buffer in case of buffer overflow */
                    if (cmd_len == CLI_MAX_CMD_LEN) {
                        cmd_buf[CLI_MAX_CMD_LEN - 1] = CLI_CR;
                    }
                    cmd_buf[cmd_len++] = ch;
                    uart_put_byte(ch); // Echo the input character
                    uart_put_byte(CLI_CR);
                    uart_put_byte(CLI_LF);
                    return TRUE;
                } else if (cmd_len < CLI_MAX_CMD_LEN && (ch > 31 && ch < 128) /* printable characters(32-127) */) {
                    cmd_buf[cmd_len++] = ch;
                    uart_put_byte(ch); // Echo the input character
                }
            }
        }
    }

    return FALSE;
}

#if 0 /* For debug only */
void list_mii_reg(uchar iport)
{
    uchar  cnt;

    println_str("std");

    for (cnt = 0; cnt <= 31; cnt++) {
        print_hex_w(cnt);
        print_str(": ");
        print_hex_w(phy_read(iport, cnt));
        print_cr_lf();
    }

    println_str("ext1");

    phy_page_ext(iport);
    for (cnt = 0x12; cnt <= 0x1e; cnt++) {
        print_hex_w(cnt);
        print_str(": ");
        print_hex_w(phy_read(iport, cnt));
        print_cr_lf();
    }

    println_str("ext2");

    phy_page_ext2(iport);
    for (cnt = 0x10; cnt <= 0x11; cnt++) {
        print_hex_w(cnt);
        print_str(": ");
        print_hex_w(phy_read(iport, cnt));
        print_cr_lf();
    }

    println_str("gp");

    phy_page_gp(iport);
    for (cnt = 0x0e; cnt <= 0x1d; cnt++) {
        print_hex_w(cnt);
        print_str(": ");
        print_hex_w(phy_read(iport, cnt));
        print_cr_lf();
    }

    phy_page_std(iport);

}
#endif

static void cli_show_help_msg(void)
{
    println_str("V : Show version");
    println_str("R <target> <offset> <addr>: Read from chip register");
    println_str("  --> Example: Read Chip ID register DEVCPU_GCB:CHIP_REGS:CHIP_ID");
    println_str("  -->                                0x71070000       0x0     0x0");
    println_str("  --> Command: R 0x71070000 0x0 0x0");

#if 0 //defined(VTSS_ARCH_OCELOT)
    println_str("  --> ANA       : 0x71880000      ES0       : 0x71040000");
    println_str("  --> DEVCPU_GCB: 0x71070000      DEVCPU_ORG: 0x71000000");
    println_str("  --> DEVCPU_PTP: 0x710E0000      DEVCPU_QS : 0x71080000");
    println_str("  --> DEV[ 0]   : 0x711E0000      DEV[ 1]   : 0x711F0000");
    println_str("  --> DEV[ 2]   : 0x71200000      DEV[ 3]   : 0x71210000");
    println_str("  --> DEV[ 4]   : 0x71220000      DEV[ 5]   : 0x71230000");
    println_str("  --> DEV[ 6]   : 0x71240000      DEV[ 7]   : 0x71250000");
    println_str("  --> DEV[ 8]   : 0x71260000      DEV[ 9]   : 0x71270000");
    println_str("  --> DEV[10]   : 0x71280000");
#endif

    println_str("W <target> <offset> <addr> <value>: Write switch register");
    println_str("I <uport> <addr> [<page>]: Read PHY register");
    println_str("  --> Example: Read user port 1 PHY ID from PHY register 2 page 0");
    println_str("  --> Command: I 1 0x2 0");
    println_str("O <uport> <addr> <value> [<page>]: Write PHY register");
    println_str("P : Show Port information");
    println_str("? : Show commands");

#ifndef UNMANAGED_REDUCED_DEBUG_IF
#if defined(VTSS_ARCH_OCELOT)
#if TRANSIT_LLDP || TRANSIT_LACP
    println_str("T <uport>: Send test packet (uport=0xff for all ports)");
#endif
#if TRANSIT_LAG
    println_str("E <0|1|2> <iport_mask>: Trunk configuration (0=Add, 1=Del, 2=Show)");
#endif //TRANSIT_LAG

#if TRANSIT_LACP
    println_str("F <0|1|2> <uport> [<key>]: LACP configuration (0=Enable, 1=Disable, 2=Show)");
#endif //TRANSIT_LACP

#if defined(UNMANAGED_FLOW_CTRL_IF)
    println_str("K <0|1> <uport>: Flow control mode (0=Enable, 1=Disable)");
#endif //UNMANAGED_FLOW_CTRL_IF

#if defined(BUFFER_RESOURCE_DEBUG_ENABLE)
    println_str("B <0|1>: Dump buffer resource configuration or status (0=Config, 1=Status)");
#endif // BUFFER_RESOURCE_DEBUG_ENABLE

#if TRANSIT_UNMANAGED_MAC_OPER_GET
    println_str("M [<uport> [c]: Show/Clear MAC address entries (uport=0 for CPU port)");
#endif // TRANSIT_UNMANAGED_MAC_OPER_GET

#elif defined(VTSS_ARCH_LUTON26)
#if LOOPBACK_TEST
    println_str("T : Loopback test");
#endif //LOOPBACK_TEST
#endif // FERRET


#if TRANSIT_LLDP
#if TRANSIT_EEE_LLDP
    println_str("G [0|1|2|3]: LLDP configuration (0=Disable, 1=Enable, 2=Remove EEE TLV, 3=Add EEE TLV)");
#else
    println_str("G [0|1]: LLDP mode (0=Disable, 1=Enable)");
#endif // TRANSIT_EEE_LLDP
#endif // TRANSIT_LLDP

    println_str("X : Reboot device");
    println_str("Z <0|1>: E-Col-Drop mode, linkup ports only (0=Disable, 1=Enable)");

#if UNMANAGED_FAN_DEBUG_IF
    println_str("F temp_max temp_on: Update temp. max and on");
#endif // UNMANAGED_FAN_DEBUG_IF

#ifndef VTSS_COMMON_NDEBUG
    println_str("B level: Set debug level 1 to 4");
#endif
#if UNMANAGED_TRANSIT_VERIPHY_DEBUG_IF
    println_str("C [<uport>]: Run veriphy");
#endif

#if TRANSIT_EEE
    print_str("A [0|1|2]: 1=EEE Enalbed, 0=EEE Disabled");
#if TRANSIT_EEE_LLDP
    println_str("    LLDP with EEE tlv enabled/disabled automatically ");
#endif // TRANSIT_EEE_LLDP
#endif // TRANSIT_EEE

#if UNMANAGED_PORT_STATISTICS_IF
    println_str("H <uport> [c]: Show/Clear port statistics (uport=0 for CPU port)");
#endif // UNMANAGED_PORT_STATISTICS_IF

#if defined(VTSS_ARCH_LUTON26)
#if USE_HW_TWI
    println_str("C : read I2C data <I2C_addr> <starting addr> <count>");
#endif // USE_HW_TWI
#endif // VTSS_ARCH_LUTON26

    println_str("S <0|1>: Suspend/Resume applications (0=Resume, 1=Suspend)");

#if TRANSIT_UNMANAGED_SWUP
    println_str("D : Dump bytes from SPI flash");
#endif // TRANSIT_UNMANAGED_SWUP

#endif /* UNMANAGED_REDUCED_DEBUG_IF */

#if TRANSIT_UNMANAGED_SYS_MAC_CONF
    println_str("CONFIG                       : Show all configurations");
    println_str("CONFIG MAC xx:xx:xx:xx:xx:xx : Update MAC addresses in RAM");
#if TRANSIT_SPI_FLASH
    println_str("CONFIG SAVE                  : Program configurations at RAM to flash");
#endif // TRANSIT_SPI_FLASH
#endif // TRANSIT_UNMANAGED_SYS_MAC_CONF
}

/* ************************************************************************ */
static uchar handle_command (void)
/* ------------------------------------------------------------------------ --
 * Purpose     : Interpret and handle command (apart from config commands).
 * Remarks     : Module variable cmd_ptr must have been set to point to first
 *               non-space char in command string, when this function is called.
 *               Returns 0, if successful, otherwise <> 0.
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    uchar cmd;
#ifndef UNMANAGED_REDUCED_DEBUG_IF
#if TRANSIT_EEE
    ulong dat;
#endif    
#if defined(VTSS_ARCH_LUTON26)
#if USE_HW_TWI
    ulong count;
    uchar i2c_addr,i2c_data;
    ulong curr_addr_l;
    uchar curr_addr;
#endif //USE_HW_TWI
#endif //defined(VTSS_ARCH_LUTON26)
#if TRANSIT_LAG
    uchar rc;
#endif //TRANSIT_LAG
#if TRANSIT_LACP
    vtss_lacp_port_config_t cfg;
    vtss_common_port_t portno;
    vtss_lacp_portstatus_t pst;
    vtss_lacp_aggregatorstatus_t ast;
    vtss_lacp_agid_t aggrid;
#endif //TRANSIT_LACP

#if UNMANAGED_TCAM_DEBUG_IF
    uchar cntr;
    tcam_data_conf_t conf_r;
#endif
#endif //UNMANAGED_REDUCED_DEBUG_IF

    cmd = *cmd_ptr;
    cmd_ptr++;

    if (retrieve_parms() != FORMAT_OK) {
        return FORMAT_ERROR;
    }

    switch (conv_to_upper_case(cmd)) {
    case 'V': /* Get version info */
        cli_show_sw_ver();
        break;

#if !defined(BRINGUP)
    case 'P': /* Show port information */
        cli_show_port_info();
        break;
#endif // !BRINGUP

    case 'W': /* Write switch register */
        if (parms_no >= 4) {
            H2_WRITE((parms[0] + parms[1] + parms[2]), parms[3]);
        }
        // No need break state for this case (write operation).
        // Read the register value after that.

    case 'R': /* Read switch register */
        /* Target and offset as parameter */
        if (parms_no >= 3) {
            H2_READ((parms[0] + parms[1] + parms[2]), parms[3] /*dat*/);
            print_parse_info_32(parms[3]);
        }
        break;

#if !defined(BRINGUP)
    case 'O': /* Write PHY register */
        /* Front port number and PHY address */
        if (parms_no >= 4) {
            /* Change page */
            phy_write(uport2cport((uchar) parms[0]), 31, parms[3]);
            phy_write(uport2cport((uchar) parms[0]), parms[1], parms[2]);
            phy_write(uport2cport((uchar) parms[0]), 31, 0);
        } else if (parms_no == 3) {
            phy_write(uport2cport((uchar) parms[0]), parms[1], parms[2]); // direct write
        }
        // No need break state for this case (write operation).
        // Read the register value after that.

    case 'I': /* Read PHY register */
        /* Front port number and PHY address */
        print_hex_prefix();
        if (parms_no > 2) {
            // Change page
            phy_write(uport2cport((uchar) parms[0]), 31, parms[2]);
            print_hex_w(phy_read(uport2cport((uchar) parms[0]), parms[1]));
            phy_write(uport2cport((uchar) parms[0]), 31, 0); // Back to std page
#if 0
        } else if (parms_no == 1) {
            /* list all mii register */
            list_mii_reg(uport2cport((uchar) parms[0]));
#endif
        } else if (parms_no == 2) {
            print_hex_w(phy_read(uport2cport((uchar) parms[0]), parms[1]));
        }

        print_cr_lf();
        break;
#endif /* BRINGUP */

#ifndef VTSS_COMMON_NDEBUG
    case 'L': /* test LED */
        led_state(parms[0], parms[1], parms[2]);
        break;
#endif

#if TRANSIT_UNMANAGED_MAC_OPER_GET
    case 'M': /* Show/Clear MAC address entries */
        if (parms_no <= 1) {
            cli_show_mac_addr(parms_no ? (parms[0] == 0 ? CPU_CHIP_PORT : uport2cport(parms[0])) : 0xFF);
        } else {
            h2_mactab_flush_port(uport2cport(parms[0]));
        }
        break;
#endif // TRANSIT_UNMANAGED_MAC_OPER_GET

    case '?': /* Show CLI command help message */
        cli_show_help_msg();
        break;

#ifndef UNMANAGED_REDUCED_DEBUG_IF
#if defined(VTSS_ARCH_OCELOT)
#if TRANSIT_LLDP || TRANSIT_LACP
    case 'T': /* Test packet sending */
        perform_tx_rx_test(parms[0] == 0xFF ? 0xFF : uport2iport(parms[0]), 2, 0);
    break;
#endif // TRANSIT_LLDP || TRANSIT_LACP    
#if TRANSIT_LAG
    case 'E': /* Tests */
       switch ((uchar) parms[0]) {
       case 0:
            rc = h2_aggr_add(parms[1]);
            break;
       case 1:
            rc = h2_aggr_delete(parms[1]);
            break;
       default:
            vtss_show_masks();
            rc = 0;
            break;
        }
        if (rc != 0 ) {
            print_str("Error code: "); print_dec(rc); print_cr_lf();
        }
        break;
#endif //TRANSIT_LAG
#if TRANSIT_LACP
    case 'F':   /* LACP configuration */
        //print_dec(parms[2]);
        portno = OSEXT2LACP(parms[1]);


        vtss_lacp_get_portconfig(portno, &cfg);
        if (parms[2] >= 0 &&  parms[2] <= 65535) {  /* Ignore it if users don't enter this parameter */
          cfg.port_key = parms[2];
        }
        //print_str("cfg.port_key:  ");  print_dec(cfg.port_key); print_cr_lf();

        switch ((uchar) parms[0]) {
        case 0:
            cfg.enable_lacp = TRUE;
            vtss_lacp_set_portconfig(portno, &cfg);
            phy_state_to_setup(uport2cport(parms[1]));
            break;
        case 1:
            cfg.enable_lacp = FALSE;
            vtss_lacp_set_portconfig(portno, &cfg);
            phy_state_to_setup(uport2cport(parms[1]));
            break;
       default:
            println_str("Group-Id | Partner-Id/Priority/key      | Ports");
            print_line(66);
            print_cr_lf();
            //println_str("=================================================================");
            for (aggrid = 1; aggrid <= VTSS_LACP_MAX_AGGR; aggrid++) {
                if (vtss_lacp_get_aggr_status(aggrid, &ast)) {
                    print_dec(aggrid);
                    print_spaces(10);
                    print_mac_addr(ast.partner_oper_system.macaddr);
                    print_ch('/');
                    print_dec(ast.partner_oper_system_priority);
                    print_ch('/');
                    print_dec(ast.partner_oper_key);
                    print_spaces(8);
                    //member_mask = 0;
                    //for (portno = 0; portno < VTSS_LACP_MAX_PORTS; portno++)
                    //    WRITE_PORT_BIT_MASK(LACP2OSINT(portno + 1), ast.port_list[portno], &member_mask);
                    //print_port_list(member_mask);
                    for (portno = 0; portno < VTSS_LACP_MAX_PORTS; portno++) {
                        if (ast.port_list[portno]) {
                            print_dec(iport2uport(portno)); print_spaces(1);
                        }
                    }
                    print_spaces(8);
                    print_dec(ast.secs_since_last_change);
                    print_cr_lf();
                }
            }

            print_cr_lf();
            print_cr_lf();
            println_str("Port  Enabled       Key     Group   Partner-Port/Priority");
            print_line(66);
            print_cr_lf();
            for (portno = 1; portno <= VTSS_LACP_MAX_PORTS; portno++) {
                vtss_lacp_get_port_status(portno, &pst);
                print_dec(portno);
                //print_spaces(6);
                print_dec_right(pst.port_enabled); //state
                print_dec_right(pst.actor_oper_port_key); //key
                print_dec_right(pst.actor_port_aggregator_identifier); //aggid
                //print_spaces(9);
                print_dec_right(pst.partner_oper_port_number); //partner_port
                print_ch('/');
                print_dec(pst.partner_oper_port_priority); // partner_pri
                print_cr_lf();
            }
            break;
        }
        break;
#endif //TRANSIT_LACP

#if defined(UNMANAGED_FLOW_CTRL_IF)
    case 'K':   /* Flow control mode */
       phy_flowcontrol_set(uport2cport(parms[1]), !parms[0]);
       phy_state_to_setup(uport2cport(parms[1]));
     break;           
#endif //UNMANAGED_FLOW_CTRL_IF

#if defined(BUFFER_RESOURCE_DEBUG_ENABLE) || defined(SERDES_DEBUG_ENABLE)
    case 'B':   /* Dump buffer resource configuration or status */
        switch ((uchar) parms[0]) {
#if defined(BUFFER_RESOURCE_DEBUG_ENABLE)
        case 0:    // Dump RES_CFG 
        case 1: {  // Dump RES_STAT
            BOOL dump_status = parms[0] ? TRUE : FALSE;
            ulong i, j, reg_val, wm_value, wm_unit, inuse, maxuse, res_idx;

            if (dump_status) {
                println_str("RES_STAT: Dump when maxuse != 0 and maxuse >= wm");
            } else {
                println_str("RES_CFG: Dump when wm != 0");
            }

            for (i = 0; i < 1024; i++) {
                // Read RES_CFG
                H2_READ(VTSS_QSYS_RES_CTRL_RES_CFG(i), reg_val);
                wm_unit = VTSS_X_QSYS_RES_CTRL_RES_CFG_WM_MULTIPLIER_UNIT(reg_val);
                wm_value = VTSS_X_QSYS_RES_CTRL_RES_CFG_WM_VALE(reg_val) * (wm_unit ? 16 : 1);

                // Read RES_STAT
                if (dump_status) {
                    H2_READ(VTSS_QSYS_RES_CTRL_RES_STAT(i), reg_val);
                    inuse = VTSS_X_QSYS_RES_CTRL_RES_STAT_INUSE(reg_val);
                    maxuse = VTSS_X_QSYS_RES_CTRL_RES_STAT_MAXUSE(reg_val);
                }

                if ((!dump_status && wm_value) || // Only dump when wm_value != 0
                    (dump_status && maxuse && maxuse >= wm_value)     // Only dump when maxuse != 0 and maxuse >= wm_value
                   ) {
                    res_idx = i / 256;
                    print_str("Res[");
                    if (res_idx == 0) {
                        print_str("SrcMem");
                    } else if (res_idx == 1) {
                        print_str("SrcRef");
                    } else if (res_idx == 2) {
                        print_str("DstMem");
                    } else { // res_idx = 3
                        print_str("DstRef");
                    }
                    print_str("] - ");

                    j = i % 256;
                    if (j < 96) {
                        print_str("Port and QoS class : Port");
                        print_dec(j / 8);
                        print_str(", Class");
                        print_dec(j % 8);
                    } else if (j < 216) {
                        //print_str("Unused:");
                        //print_dec(j);
                    } else if (j < 224) {
                        print_str("QoS class sharing  : Class");
                        print_dec(j - 216);
                    } else if (j < 236) {
                        print_str("Port reservation   : Port");
                        print_dec(j - 224);
                    } else if (j == 254) {
                        print_str("Color sharing      : DP=1(Yellow)");
                    } else if (j == 255) {
                        print_str("Color sharing      : DP=0(Green)");
                    } else {
                        //print_str("Unused:");
                        //print_dec(j);
                    }

                    print_str(". WM=");
                    print_dec(wm_value);

                    if (dump_status) {
                        print_str(", Inuse=");
                        print_dec(inuse);
                        print_str(", Maxuse=");
                        print_dec(maxuse);
                    }
                    print_cr_lf();
                }
            }
        }
        break;
#endif // BUFFER_RESOURCE_DEBUG_ENABLE

#if defined(SERDES_DEBUG_ENABLE)
        case 2: {
            ulong idx, i, sd6g_addr, reg_val, dump_reg_cnt = 31;
            ulong reg_addr[31] = {
                VTSS_HSIO_SERDES6G_DIG_CFG_SERDES6G_DIG_CFG            ,
                VTSS_HSIO_SERDES6G_DIG_CFG_SERDES6G_DFT_CFG0           ,
                VTSS_HSIO_SERDES6G_DIG_CFG_SERDES6G_DFT_CFG1           ,
                VTSS_HSIO_SERDES6G_DIG_CFG_SERDES6G_DFT_CFG2           ,
                VTSS_HSIO_SERDES6G_DIG_CFG_SERDES6G_TP_CFG0            ,
                VTSS_HSIO_SERDES6G_DIG_CFG_SERDES6G_TP_CFG1            ,
                VTSS_HSIO_SERDES6G_DIG_CFG_SERDES6G_RC_PLL_BIST_CFG    ,
                VTSS_HSIO_SERDES6G_DIG_CFG_SERDES6G_MISC_CFG           ,
                VTSS_HSIO_SERDES6G_DIG_CFG_SERDES6G_OB_ANEG_CFG        ,
                VTSS_HSIO_SERDES6G_DIG_STATUS_SERDES6G_DFT_STATUS      ,
                VTSS_HSIO_SERDES6G_DIG_STATUS_SERDES6G_ERR_CNT         ,
                VTSS_HSIO_SERDES6G_DIG_STATUS_SERDES6G_MISC_STATUS     ,
                VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_DES_CFG            ,
                VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG             ,
                VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG1            ,
                VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG2            ,
                VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG3            ,
                VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG4            ,
                VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_IB_CFG5            ,
                VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_OB_CFG             ,
                VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_OB_CFG1            ,
                VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_SER_CFG            ,
                VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG         ,
                VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_PLL_CFG            ,
                VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_ACJTAG_CFG         ,
                VTSS_HSIO_SERDES6G_ANA_CFG_SERDES6G_GP_CFG             ,
                VTSS_HSIO_SERDES6G_ANA_STATUS_SERDES6G_IB_STATUS0      ,
                VTSS_HSIO_SERDES6G_ANA_STATUS_SERDES6G_IB_STATUS1      ,
                VTSS_HSIO_SERDES6G_ANA_STATUS_SERDES6G_ACJTAG_STATUS   ,
                VTSS_HSIO_SERDES6G_ANA_STATUS_SERDES6G_PLL_STATUS      ,
                VTSS_HSIO_SERDES6G_ANA_STATUS_SERDES6G_REVID
            };        
            char *reg_name[31] = {
                "SERDES6G_DIG_CFG_SERDES6G_DIG_CFG",
                "SERDES6G_DIG_CFG_SERDES6G_DFT_CFG0",
                "SERDES6G_DIG_CFG_SERDES6G_DFT_CFG1",
                "SERDES6G_DIG_CFG_SERDES6G_DFT_CFG2",
                "SERDES6G_DIG_CFG_SERDES6G_TP_CFG0 ",
                "SERDES6G_DIG_CFG_SERDES6G_TP_CFG1",
                "SERDES6G_DIG_CFG_SERDES6G_RC_PLL_BIST_CFG",
                "SERDES6G_DIG_CFG_SERDES6G_MISC_CFG",
                "SERDES6G_DIG_CFG_SERDES6G_OB_ANEG_CFG",
                "SERDES6G_DIG_STATUS_SERDES6G_DFT_STATUS",
                "SERDES6G_DIG_STATUS_SERDES6G_ERR_CNT",
                "SERDES6G_DIG_STATUS_SERDES6G_MISC_STATUS",
                "SERDES6G_ANA_CFG_SERDES6G_DES_CFG",
                "SERDES6G_ANA_CFG_SERDES6G_IB_CFG",
                "SERDES6G_ANA_CFG_SERDES6G_IB_CFG1",
                "SERDES6G_ANA_CFG_SERDES6G_IB_CFG2",
                "SERDES6G_ANA_CFG_SERDES6G_IB_CFG3",
                "SERDES6G_ANA_CFG_SERDES6G_IB_CFG4",
                "SERDES6G_ANA_CFG_SERDES6G_IB_CFG5",
                "SERDES6G_ANA_CFG_SERDES6G_OB_CFG",
                "SERDES6G_ANA_CFG_SERDES6G_OB_CFG1",
                "SERDES6G_ANA_CFG_SERDES6G_SER_CFG",
                "SERDES6G_ANA_CFG_SERDES6G_COMMON_CFG",
                "SERDES6G_ANA_CFG_SERDES6G_PLL_CFG",
                "SERDES6G_ANA_CFG_SERDES6G_ACJTAG_CFG",
                "SERDES6G_ANA_CFG_SERDES6G_GP_CFG ",
                "SERDES6G_ANA_STATUS_SERDES6G_IB_STATUS0",
                "SERDES6G_ANA_STATUS_SERDES6G_IB_STATUS1",
                "SERDES6G_ANA_STATUS_SERDES6G_ACJTAG_STATUS",
                "SERDES6G_ANA_STATUS_SERDES6G_PLL_STATUS",
                "SERDES6G_ANA_STATUS_SERDES6G_REVID"
            };

            for (idx = 0; idx < 2; idx++) {
                if (idx == 0) {
                    println_str("Dump SerDes6G_1");
                    sd6g_addr = 1 << 1; // SerDes6G_1
                } else {
                    println_str("Dump SerDes6G_2");
                    sd6g_addr = 1 << 2; // SerDes6G_2
                }

                for (i = 0; i < dump_reg_cnt; i++) {
                    h2_sd6g_read(sd6g_addr);
                    H2_READ(reg_addr[i], reg_val);
                    println_str(reg_name[i]);
                    print_parse_info_32(reg_val);
                    print_cr_lf();  
                }
            }
            break;
        }
        case 3: {
            ulong idx, i, sd1g_addr, reg_val, dump_reg_cnt = 15;
            ulong reg_addr[15] = {
                VTSS_HSIO_SERDES1G_ANA_CFG_SERDES1G_DES_CFG        ,
                VTSS_HSIO_SERDES1G_ANA_CFG_SERDES1G_IB_CFG         ,
                VTSS_HSIO_SERDES1G_ANA_CFG_SERDES1G_OB_CFG         ,
                VTSS_HSIO_SERDES1G_ANA_CFG_SERDES1G_SER_CFG        ,
                VTSS_HSIO_SERDES1G_ANA_CFG_SERDES1G_COMMON_CFG     ,
                VTSS_HSIO_SERDES1G_ANA_CFG_SERDES1G_PLL_CFG        ,
                VTSS_HSIO_SERDES1G_ANA_STATUS_SERDES1G_PLL_STATUS  ,
                VTSS_HSIO_SERDES1G_DIG_CFG_SERDES1G_DFT_CFG0       ,
                VTSS_HSIO_SERDES1G_DIG_CFG_SERDES1G_DFT_CFG1       ,
                VTSS_HSIO_SERDES1G_DIG_CFG_SERDES1G_DFT_CFG2       ,
                VTSS_HSIO_SERDES1G_DIG_CFG_SERDES1G_TP_CFG         ,
                VTSS_HSIO_SERDES1G_DIG_CFG_SERDES1G_RC_PLL_BIST_CFG,
                VTSS_HSIO_SERDES1G_DIG_CFG_SERDES1G_MISC_CFG       ,
                VTSS_HSIO_SERDES1G_DIG_STATUS_SERDES1G_DFT_STATUS  ,
                VTSS_HSIO_SERDES1G_DIG_STATUS_SERDES1G_MISC_STATUS
            };        
            char *reg_name[15] = {
                "SERDES1G_ANA_CFG_SERDES1G_DES_CFG",
                "SERDES1G_ANA_CFG_SERDES1G_IB_CFG",
                "SERDES1G_ANA_CFG_SERDES1G_OB_CFG",
                "SERDES1G_ANA_CFG_SERDES1G_SER_CFG",
                "SERDES1G_ANA_CFG_SERDES1G_COMMON_CFG",
                "SERDES1G_ANA_CFG_SERDES1G_PLL_CFG",
                "SERDES1G_ANA_STATUS_SERDES1G_PLL_STATUS",
                "SERDES1G_DIG_CFG_SERDES1G_DFT_CFG0",
                "SERDES1G_DIG_CFG_SERDES1G_DFT_CFG1",
                "SERDES1G_DIG_CFG_SERDES1G_DFT_CFG2",
                "SERDES1G_DIG_CFG_SERDES1G_TP_CFG",
                "SERDES1G_DIG_CFG_SERDES1G_RC_PLL_BIST_CFG",
                "SERDES1G_DIG_CFG_SERDES1G_MISC_CFG",
                "SERDES1G_DIG_STATUS_SERDES1G_DFT_STATUS",
                "SERDES1G_DIG_STATUS_SERDES1G_MISC_STATUS "
            };

            for (idx = 0; idx < 2; idx++) {
                if (idx == 0) {
                    println_str("Dump SerDes1G_4");
                    sd1g_addr = 1 << 4; // SerDes1G_4
                } else {
                    println_str("Dump SerDes1G_5");
                    sd1g_addr = 1 << 5; // SerDes1G_5
                }
                for (i = 0; i < dump_reg_cnt; i++) {
                    h2_sd1g_read(sd1g_addr);
                    H2_READ(reg_addr[i], reg_val);
                    println_str(reg_name[i]);
                    print_parse_info_32(reg_val);
                    print_cr_lf();  
                }
            }
            break;
        }
#endif // SERDES_DEBUG_ENABLE
    }
break;
#endif // BUFFER_RESOURCE_DEBUG_ENABLE || SERDES_DEBUG_ENABLE

#if UNMANAGED_TCAM_DEBUG_IF
    case 'Y': /* TCAM configuration get */
        memset(&conf_r,0,sizeof(tcam_data_conf_t));
        cntr = h2_tcam_count_get(TCAM_TARGET_IS2);
        print_dec(cntr);
        println_str(" is ACL count ");
        if ((uchar)parms[0] < cntr) {
            h2_tcam_e2e_tc_get((uchar)parms[0],&conf_r);
            print_str("TCAM information of entry id ");
            print_dec(parms[0]);
            print_cr_lf();
            print_line(66);
            print_cr_lf();
            for (cntr=0;cntr<TCAM_ENTRY_WIDTH;cntr++) {
                print_cr_lf();
                print_hex_dw(conf_r.tcam_entry[cntr]);
                println_str("<tcam entry");
                print_hex_dw(conf_r.tcam_mask[cntr]);
                println_str("<tcam mask");
            }
            print_hex_dw(conf_r.tcam_action[0]);
            println_str("<action 0");
            print_hex_dw(conf_r.tcam_action[1]);
            println_str("<action 1");
            print_hex_dw(conf_r.tg);
            println_str("<tg");
            print_hex_dw(conf_r.tcam_count);
            println_str("<count");
        } else {
            print_dec((uchar)parms[0]);
            println_str(" is not valid ACL id");
        }
        break;
#endif


#elif defined(VTSS_ARCH_LUTON26)
    case 'T': /* Tests */
        switch ((uchar) parms[0]) {
#if LOOPBACK_TEST
        case 1:
            perform_tx_rx_test(parms[1], parms[2], 0);
            break;
        case 2:
            perform_tx_rx_test(parms[1], parms[2], 1);
            break;
#endif /* LOOPBACK_TEST */

        default:
            return FORMAT_ERROR;
        }
        break;
#endif //#elif defined(VTSS_ARCH_LUTON26)

    case 'Z': { /* Enable/Disalbe Excessive Col drop */
        vtss_iport_no_t iport_idx;
        vtss_cport_no_t chip_port;
        uchar drop_enable = (parms_no == 1 && parms[0] == 1) ? 0 : 1;

        for (iport_idx = MIN_PORT; iport_idx < MAX_PORT; iport_idx++) {
            chip_port = iport2cport(iport_idx);
            if (is_port_link(chip_port)) { // Link up
                // Set E-Col-Drop on link-up ports only
                h2_enable_exc_col_drop(iport2cport(iport_idx), drop_enable);
            }
        }
        break;
    } // case 'Z'

#if USE_HW_TWI
#if defined(VTSS_ARCH_LUTON26)
    case 'C': /* Read switch register */
        print_cr_lf();
        i2c_addr = parms[0];
        if (parms[2] == 0) { /* test i2c_eeprom_read */
            curr_addr_l = parms[1];
            if (i2c_eeprom_read(i2c_addr, &curr_addr_l, &i2c_data) == TRUE) {
                print_hex_b(i2c_data);
                print_spaces(2);
                if (i2c_data > 32 && i2c_data< 127)
                    print_ch(i2c_data);
                print_cr_lf();
            } else {
                println_str("access fail");
            }

        } else { /* sample code of i2c_tx and i2c_rx*/
            curr_addr = parms[1];
            for (count = 0; count < parms[2]; count++) {
#if 0 // Ferret, TODO. Missing first parameter
                i2c_tx(i2c_addr, &curr_addr, 1);
                dat = i2c_rx(i2c_addr, &i2c_data, 1);
#endif // Ferret. TODO.
                if (dat != 0 ) {
                    print_dec(count);
                    print_str(": ");
                    print_hex_b(i2c_data);
                    print_spaces(2);
                    if (i2c_data > 32 && i2c_data< 127)
                        print_ch(i2c_data);
                    print_cr_lf();
                } else
                    break;
                curr_addr++;
            }
        }

        break;
#endif //#if defined(VTSS_ARCH_LUTON26)
#endif //USE_HW_TWI

#if UNMANAGED_PORT_STATISTICS_IF
    case 'H': /* Show/Clear port statistics */
        if (parms_no == 1) {
            print_port_statistics(parms[0] == 0 ? CPU_CHIP_PORT : uport2cport(parms[0]));
        } else if (parms_no == 2 && parms[0] <= NO_OF_BOARD_PORTS) {
            h2_stats_counter_clear(parms[0] == 0 ? CPU_CHIP_PORT : uport2cport(parms[0]));
        } else {
            return FORMAT_ERROR;
        }
        break;
    case 0x18: /* 0x18: Contrl Character ^R, it is used to break the CLI display process */
        GPARM_break_show_statistic_flag = 1;
        break;
#endif

#if TRANSIT_FAN_CONTROL
#if UNMANAGED_FAN_DEBUG_IF
    case 'F': /* Fan control */
        /*
        ** F t_max t_on
        ** F
        */
        if (parms_no >= 1) {
            write_fan_conf_t_max((uchar) parms[0]);
            write_fan_conf_t_on((uchar) parms[1]);
        } else {
            handle_fan_control();
        }
        break;
#endif
#endif

#if TRANSIT_EEE
        //
        // Enable / disable EEE for all ports.
        // cmd:"A 0"  = EEE Disable, disable LLDP
        // cmd:"A 1"  = EEE Enable VGA down, enable LLDP
        // cmd:"A 2"  = EEE Enable VGA up, enable LLDP

    case 'A': /* Enable/Disable EEE */
        // We set all ports to the same.
        for (parms[1] = 0; parms[1] < NO_OF_BOARD_PORTS; parms[1]++)
        {
            vtss_port_no_t  iport = parms[1];
            vtss_port_no_t  fport = iport + 1;
            vtss_port_no_t  sport = uport2cport(fport);

            switch ((uchar) parms[0]) {
            case 0:
                /* Disable EEE */
                write_eee_conf_mode(iport, FALSE);
                eee_port_mode_setup(sport);
#if TRANSIT_EEE_LLDP
                lldp_os_set_admin_status(fport, LLDP_DISABLED);
#endif
                break;
            case 1:
                /* Enable EEE*/
                write_eee_conf_mode(iport, TRUE);
                eee_port_mode_setup(sport);
                /* Enable VGA Down*/
#if VTSS_ATOM12_A
                vga_adc_debug (sport, ATOM12_EN_NONE);
#endif
#if TRANSIT_EEE_LLDP
                lldp_os_set_admin_status(fport, LLDP_ENABLED_RX_TX);
#endif
                break;
            case 2:
                /* Enable EEE*/
                write_eee_conf_mode(iport, TRUE);
                eee_port_mode_setup(sport);
#if VTSS_ATOM12_A
                /* Enable VGA Up*/
                vga_adc_debug (sport, ATOM12_EN_BOTH);
#endif
#if TRANSIT_EEE_LLDP
                lldp_os_set_admin_status(fport, LLDP_ENABLED_RX_TX);
#endif
                break;

            case 3:
                dat = phy_mmd_rd(sport, 7, 61);
                print_str ("7.61 =0x");
                print_hex_w(dat);

                dat = phy_mmd_rd(sport, 7, 60);
                print_str (" 7.60 =0x");
                print_hex_w(dat);

                dat = phy_mmd_rd(sport, 3, 1);
                print_str (" 3.1 =0x");
                print_hex_w(dat);

                print_str (" \r\n");
                break;

            default:
                // print out current state
                print_str("Port ");
                print_dec(fport);

                if (read_eee_conf_mode(iport)) {
                    print_str(": Enabled");
                } else {
                    print_str(": Disabled");
                }
#if TRANSIT_LLDP
                if (lldp_os_get_admin_status(fport)) {
                    print_str(" w LLDP");
#if TRANSIT_EEE_LLDP
                    if(lldp_os_get_optional_tlv_enabled(LLDP_TLV_ORG_EEE_TLV)) {
                        print_str(" EEE tlv");
                    } else {
                        print_str(" No EEE tlv");
                    }
#endif /* TRANSIT_EEE_LLDP */
                } else {
                    print_str(" w/o LLDP");
                }
#endif /* TRANSIT_LLDP */
                print_cr_lf();
                break;
            }
        }
        print_cr_lf();
        break;
#endif /* TRANSIT_EEE */

    case 'X': /* Reboot device */
        h2_reset();
        break;

#if TRANSIT_LLDP
    case 'G': /* LLDP configuration */
        if (parms_no == 1) {
            switch((uchar) parms[0]) {
            case 0: /* set disable */
                for(parms[1] = MIN_PORT; parms[1] < MAX_PORT; parms[1]++) {
                    lldp_os_set_admin_status((uchar)parms[1], LLDP_DISABLED);
                }
#if TRANSIT_BPDU_PASS_THROUGH
                h2_bpdu_t_registration(0x0E, FALSE);
#endif
                break;
            case 1: /* set enable */
                for(parms[1] = MIN_PORT; parms[1] < MAX_PORT; parms[1]++) {
                    lldp_os_set_admin_status((uchar)parms[1], LLDP_ENABLED_RX_TX);
                }
#if TRANSIT_BPDU_PASS_THROUGH
                h2_bpdu_t_registration(0x0E, TRUE);
#endif
                break;
#if TRANSIT_EEE_LLDP
            case 2: /* Remove LLDP EEE tlv */
                lldp_os_set_optional_tlv(LLDP_TLV_ORG_EEE_TLV, 0);
                break;
            case 3: /* Add LLDP EEE tlv */
                lldp_os_set_optional_tlv(LLDP_TLV_ORG_EEE_TLV, 1);
                break;
#endif
            default:
                return FORMAT_ERROR;
            }
            break;
        }
#if UNMANAGED_LLDP_DEBUG_IF
        cmd_print_lldp_remoteinfo();
#endif
        break;
#endif

#ifndef VTSS_COMMON_NDEBUG
    case 'B': /* debug level */
        if(((uchar) parms[0] <= VTSS_COMMON_TRLVL_RACKET) &&
                ((uchar) parms[0] >= VTSS_COMMON_TRLVL_ERROR)) {
            update_debug_lvl((uchar) parms[0]);
        }
        break;
#endif

#if UNMANAGED_TRANSIT_VERIPHY_DEBUG_IF
    case 'C': /* Run veriphy */
        if (parms_no == 0) {
            cmd_run_veriphy(NO_OF_BOARD_PORTS); // Run veriphy on all ports
        } else if (parms_no == 1 && parms[0] < NO_OF_BOARD_PORTS) {
            cmd_run_veriphy(uport2iport(parms[0]));
        } else {
            return FORMAT_ERROR;
        }
        break;
#endif

#if !defined(BRINGUP)
    case 'S': /* Suspend/Resume applications */
        switch ((uchar) parms[0]) {
        case 0:
            /* Resume */
            sysutil_set_suspend(FALSE);
            break;
        case 1:
            /* Suspend */
            sysutil_set_suspend(TRUE);
            break;

        default:
            return FORMAT_ERROR;
        }
        break;
#endif /* BRINGUP */

#if TRANSIT_UNMANAGED_SWUP
    case 'D': /* Dump bytes from SPI flash */
        flash_read_bytes(parms[0], parms[1]);
        break;
#endif

#endif /* UNMANAGED_REDUCED_DEBUG_IF */

    default:
        return FORMAT_ERROR;
    }
    return FORMAT_OK;
}

/* ************************************************************************ */
static uchar retrieve_parms (void)
/* ------------------------------------------------------------------------ --
 * Purpose     : Retrieve parameters from command string.
 * Remarks     : Module variable cmd_ptr must have been set to point to first
 *               char after the command in the command string, when this function
 *               is called.
 *               The module variables parms_no and parms are updated with
 *               actual number of parameters and the actual parameter values.
 *               Returns 0, if successful, otherwise <> 0.
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    uchar ch;
    uchar base;
    uchar j;
    uchar no_of_digits;
    uchar digits [10];
    ulong parm_bin;

    ch = *cmd_ptr;
    if ((ch != ' ') && (ch != CLI_CR)) {
        return FORMAT_ERROR;
    }


    parms_no = 0;
    /* Preset parms to ff's, which may be used as default indication */
    memset(parms, 0xff, sizeof(parms));

    /*
    ** Retrieve parameters one by one.
    */
    for (;;) {

        skip_spaces();
        base = 10; /* default parameter is specified in decimal */
        no_of_digits = 0;

        /*
        ** Check if any hex prefix
        */
        if (*cmd_ptr == '0' && (conv_to_upper_case(*(cmd_ptr + 1)) == 'X')) {
            base = 16; /* parameter is specified in hex */
            cmd_ptr += 2;
            if (*cmd_ptr == ' ') {
                return FORMAT_ERROR;
            }
        }

        /*
        ** Retrieve digits until delimiter (space or CR) and then convert
        ** parameter to binary
        */
        for (;;) {

            ch = *cmd_ptr;

            if ((ch == ' ') || (ch == CLI_CR)) {

                if (no_of_digits > 0) {
                    parm_bin = 0;
                    for (j = 0; j < no_of_digits; j++) {
                        parm_bin = (parm_bin * base) + digits[j];
                    }
                    if (parms_no < CLI_MAX_NO_OF_PARMS) {
                        parms[parms_no++] = parm_bin;
                    }
                }
                /* End processing at end of command string */
                if (ch == CLI_CR) {
                    return FORMAT_OK;
                }
                break; /* go get new parameter */
            } else {

                ch = ascii_to_hex_nib(ch);
                if (ch != 0xff) {
                    if (no_of_digits < 10) {
                        digits[no_of_digits++] = ch;
                        if (ch > 9) {
                            base = 16; /* parameter is specified in hex */
                        }
                    }
                } else {
                    return FORMAT_ERROR;
                }
            }
            cmd_ptr++;
        }
    }
}

#if TRANSIT_UNMANAGED_SYS_MAC_CONF
/* ************************************************************************ */
static uchar retrieve_str_parms (void)
/* ------------------------------------------------------------------------ --
 * Purpose     : Retrieve parameters from command string.
 * Remarks     : Module variable cmd_ptr must have been set to point to first
 *               char after the command in the command string, when this function
 *               is called.
 *               The module variables parms_no and parms are updated with
 *               actual number of parameters and the actual parameter values.
 *               Returns 0, if successful, otherwise <> 0.
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    uchar ch;
    uchar j;

    ch = *cmd_ptr;
    if ((ch != ' ') && (ch != CLI_CR)) {
        return FORMAT_ERROR;
    }

    parms_no = 0;
    for(parms_no = 0; parms_no < CLI_MAX_NO_OF_PARMS; parms_no++) {
        skip_spaces();
        str_parms[parms_no].str = cmd_ptr;
        str_parms[parms_no].len = 0;
        while(1) {
            if(*cmd_ptr == CLI_CR) { /* Enter Key */
                *cmd_ptr = '\0';
                for(j = parms_no + 1; j < CLI_MAX_NO_OF_PARMS; j++) {
                    str_parms[j].str = cmd_ptr;
                    str_parms[j].len = 0;
                }
                return FORMAT_OK;
            }
            if(*cmd_ptr == ' ') { /* Space Key */
                *cmd_ptr = '\0'; // string null end sign
                cmd_ptr++;
                break;
            }
            str_parms[parms_no].len++;
            cmd_ptr++;
        }
    }
    return FORMAT_OK;
}
#endif

#ifndef VTSS_COMMON_NDEBUG
static void update_debug_lvl(uchar lvl)
{
    vtss_os_trace_level = lvl;
}
#endif

#if TRANSIT_UNMANAGED_SYS_MAC_CONF
static uchar cmd_retrieve_mac_addr (uchar *mac_addr_str, uchar * mac_addr)
/* ------------------------------------------------------------------------ --
 * Purpose     : Retrieve MAC value from command string.
 * Remarks     : Module variable cmd_ptr must have been set to point to first
 *               non-space char after the "config mac" command in the command string,
 *               when this function is called.
 *               The retrieved MAC address is returned in module variable cmd_mac_addr
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    uchar j;
    uchar k;
    uchar ch;
    uchar *ptr = mac_addr_str;

    for (j = 0; j < 6; j++) {
        for (k = 0; k < 2; k++) {
            ch = *ptr;
            ptr++;

            ch = ascii_to_hex_nib(ch);
            if (ch == 0xff) {
                return FORMAT_ERROR;
            }

            mac_addr[j] = (mac_addr[j] << 4) | ch;
        }

        if (j < 5) {
            ch = *ptr;
            if (ascii_to_hex_nib(ch) == 0xff) {
                ptr++;
                if ((ch != '-') && (ch != ':') && (ch != '.')) {
                    return FORMAT_ERROR;
                }
            }
        }
    }
    if(*ptr != '\0') {
        return FORMAT_ERROR;
    }

    return FORMAT_OK;
}
#endif

/* ************************************************************************ */
static void skip_spaces (void)
/* ------------------------------------------------------------------------ --
 * Purpose     : Adjust cmd_ptr to point to next char different from space.
 * Remarks     :
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    while (*cmd_ptr == ' ') {
        cmd_ptr++;
    }
}

#if UNMANAGED_TRANSIT_VERIPHY_DEBUG_IF
/* ************************************************************************ */
static void print_veriphy_status (uchar status)
/* ------------------------------------------------------------------------ --
 * Purpose     :
 * Remarks     :
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    code std_txt_t status_txt_tab [16] = {
        TXT_NO_VERIPHY_OK,       /*  0, Correctly terminated pair */
        TXT_NO_VERIPHY_OPEN,     /*  1, Open pair */
        TXT_NO_VERIPHY_SHORT,    /*  2, Shorted pair */
        TXT_NO_VERIPHY_FAULT,    /*  3, not used */
        TXT_NO_VERIPHY_ABNORMAL, /*  4, Abnormal termination */
        TXT_NO_VERIPHY_FAULT,    /*  5, not used */
        TXT_NO_VERIPHY_FAULT,    /*  6, not used */
        TXT_NO_VERIPHY_FAULT,    /*  7, not used */
        TXT_NO_VERIPHY_XA,       /*  8, Cross-pair short to pair A */
        TXT_NO_VERIPHY_XB,       /*  9, Cross-pair short to pair B */
        TXT_NO_VERIPHY_XC,       /* 10, Cross-pair short to pair C */
        TXT_NO_VERIPHY_XD,       /* 11, Cross-pair short to pair D */
        TXT_NO_VERIPHY_XCPLA,    /* 12, Abnormal cross-pair coupling with pair A */
        TXT_NO_VERIPHY_XCPLB,    /* 13, Abnormal cross-pair coupling with pair B */
        TXT_NO_VERIPHY_XCPLC,    /* 14, Abnormal cross-pair coupling with pair C */
        TXT_NO_VERIPHY_XCPLD,    /* 15, Abnormal cross-pair coupling with pair D */
    };
    print_txt(status_txt_tab[status]);

}

static void cmd_run_veriphy(vtss_iport_no_t iport)
{
    uchar port_no;
    uchar j;
    uchar errors = 0;
    port_bit_mask_t iport_mask;
    BOOL    done = FALSE;
    vtss_uport_no_t uport_idx;
    vtss_iport_no_t iport_idx;
    vtss_cport_no_t chip_port;
    /* veriphy_parms_t xdata veriphy_parms [NO_OF_CHIP_PORTS]; */
    veriphy_parms_t xdata *veriphy_parms = (veriphy_parms_t *) rx_packet;

    // Fill iport_mask
    if (iport == NO_OF_BOARD_PORTS) {
        for (iport_idx = MIN_PORT; iport_idx < MAX_PORT; iport_idx++) {
            WRITE_PORT_BIT_MASK(iport, 1, &iport_mask);
        }
    } else {
        WRITE_PORT_BIT_MASK(iport, 1, &iport_mask);
    }

    // Start VeriPhy
    for (iport_idx = MIN_PORT; iport_idx < MAX_PORT; iport_idx++) {
        chip_port = iport2cport(iport_idx);
        if (TEST_PORT_BIT_MASK(iport_idx, &iport_mask) && phy_map(chip_port)) {
            /* Read PHY id to determine action */
            veriphy_start(chip_port); // Starting veriphy for selected port(s)
        }
    }

    // Pulling Verphy until Veriphy is done
    for (iport_idx = MIN_PORT; iport_idx < MAX_PORT; iport_idx++) {
        chip_port = iport2cport(iport_idx);
        phy_page_std(chip_port);//read phy status register
        if (TEST_PORT_BIT_MASK(iport_idx, &iport_mask) && phy_map(chip_port)) {
            done = FALSE;
            while (!done) {
                veriphy_run(chip_port, (veriphy_parms + chip_port), &done);
            }
        } else { //For Non-Test port, set link status as "OPEN"
            veriphy_parms[chip_port].loc[0] = 0;
            veriphy_parms[chip_port].loc[1] = 0;
            veriphy_parms[chip_port].loc[2] = 0;
            veriphy_parms[chip_port].loc[3] = 0;
            veriphy_parms[chip_port].stat[0] = 1;
            veriphy_parms[chip_port].stat[1] = 1;
            veriphy_parms[chip_port].stat[2] = 1;
            veriphy_parms[chip_port].stat[3] = 1;
            veriphy_parms[chip_port].flags = 0;
        }
    }

    /* Print header, order by uport
     */
    print_txt(TXT_NO_VERIPHY_STAT_HDR);
    for (uport_idx = 1; uport_idx < NO_OF_BOARD_PORTS; uport_idx++) {
        chip_port = uport2cport(uport_idx);
        iport_idx = uport2iport(uport_idx);
        if (TEST_PORT_BIT_MASK(iport_idx, &iport_mask) && phy_map(chip_port) ) {
            print_dec_16_right(uport_idx, 2);
            print_ch(':');
            print_spaces(3);
            /* Valid */
            if ((veriphy_parms + chip_port)->flags) {
                print_str("yes");
            } else {
                print_str("no ");
            }
            print_spaces(2);

            /* Length or distance to fault for each pair */
            for (j = 0; j < 4; j++) {
                print_spaces(3);

                if ((veriphy_parms + chip_port)->loc[j] != 0xff) {
                    print_dec_16_right(veriphy_parms[chip_port].loc[j], 3);
                } else {
                    print_str("  -");
                }
            }

            /* Status for each pair */

            for (j = 0; j < 4; j++) {
                print_spaces(2);
                print_veriphy_status((veriphy_parms + chip_port)->stat[j]);
#if FRONT_LED_PRESENT
                if ((veriphy_parms + chip_port)->stat[j]) {
                    errors = 1;
                    /* Set Error Leds */
                    led_port_event_set(iport_idx, VTSS_LED_EVENT_VERIPHY_ERR, VTSS_LED_MODE_BLINK_YELLOW);
                }
#endif // FRONT_LED_PRESENT
            }
            print_cr_lf();
        }
    }

#if FRONT_LED_PRESENT
    if (errors) {
        delay_1(3000); // Show error state for 3 seconds
        for (iport_idx = MIN_PORT; iport_idx < MAX_PORT; iport_idx++) {
            chip_port = iport2cport(port_no);

            if (TEST_PORT_BIT_MASK(iport_idx, &iport_mask) && phy_map(chip_port)) {
                led_port_event_set(iport_idx, VTSS_LED_EVENT_VERIPHY_ERR, VTSS_LED_MODE_NORMAL);
            }
        }
    }
#endif // FRONT_LED_PRESENT
}
#endif // UNMANAGED_TRANSIT_VERIPHY_DEBUG_IF

#if TRANSIT_LLDP
#if UNMANAGED_LLDP_DEBUG_IF
static void report_remote_entry_val (std_txt_t txt_no, lldp_tlv_t lldp_field, lldp_remote_entry_t xdata * entry)
{

    print_txt_left(txt_no, 15);
    if(txt_len(txt_no) >= 15) {
        print_spaces(1);
    }

    lldp_remote_tlv_to_string(entry, lldp_field, rx_packet);
    println_str(rx_packet);
    rx_packet[0] = 0;
}

static void cmd_print_lldp_remoteinfo (void)
{
    uchar i, max;
    lldp_remote_entry_t xdata * entry;
    uchar ext_port;
    uchar found = FALSE;


    for(ext_port = 1; ext_port < NO_OF_BOARD_PORTS; ext_port++) {
        max = lldp_remote_get_max_entries();
        for(i = 0; i < max; i++) {
            entry = lldp_get_remote_entry(i);
            if((entry->in_use) && (entry->receive_port == ext_port)) {
                found = TRUE;
                print_txt(TXT_NO_LLDP_LOCAL_PORT);
                print_ch(':');
                print_spaces(4);
                print_dec(ext_port);
                print_cr_lf();

                print_txt_left(TXT_NO_LLDP_CHASSIS_TYPE, 15);
                lldp_chassis_type_to_string(entry, rx_packet);
                println_str(rx_packet);

                report_remote_entry_val(TXT_NO_LLDP_CHASSIS_ID, LLDP_TLV_BASIC_MGMT_CHASSIS_ID, entry);

                print_txt_left(TXT_NO_LLDP_PORT_TYPE, 15);
                lldp_port_type_to_string(entry, rx_packet);
                println_str(rx_packet);

                report_remote_entry_val(TXT_NO_LLDP_PORT_ID, LLDP_TLV_BASIC_MGMT_PORT_ID, entry);
                report_remote_entry_val(TXT_NO_LLDP_SYSTEM_NAME, LLDP_TLV_BASIC_MGMT_SYSTEM_NAME, entry);
                report_remote_entry_val(TXT_NO_LLDP_SYSTEM_DESCR, LLDP_TLV_BASIC_MGMT_SYSTEM_DESCR, entry);
                report_remote_entry_val(TXT_NO_LLDP_PORT_DESCR, LLDP_TLV_BASIC_MGMT_PORT_DESCR, entry);
                report_remote_entry_val(TXT_NO_LLDP_SYSTEM_CAPA, LLDP_TLV_BASIC_MGMT_SYSTEM_CAPA, entry);
                report_remote_entry_val(TXT_NO_LLDP_MGMT_ADDR, LLDP_TLV_BASIC_MGMT_MGMT_ADDR, entry);
#if UNMANAGED_EEE_DEBUG_IF
                report_remote_entry_val(TXT_NO_EEE_STAT, LLDP_TLV_ORG_EEE_TLV, entry);
#endif
#ifndef NDEBUG
                print_str("TTL: ");
                print_dec(entry->rx_info_ttl);
                print_cr_lf();
#endif

                print_cr_lf();
            }
        }
    }

    if(!found) {
        println_str(TXT_NO_LLDP_NO_ENTRIES);
    }
}
#endif
#endif

#if TRANSIT_FAN_CONTROL
#if UNMANAGED_FAN_DEBUG_IF
static void handle_fan_control (void)
{
    // Get the chip temperature
    fan_local_status_t status;

    print_str("Temp Max.");
    print_dec_16_right(read_fan_conf_t_max(), 5);
    print_str("C, ");
    print_str("Temp On  ");
    print_dec_16_right(read_fan_conf_t_on (), 5);
    println_str("C.");

    fan_get_local_status(&status);
    print_str("Chip Temp.  ");
    print_dec_16_right(status.chip_temp, 5);
    print_str("C, ");
    print_str("Fan Speed set to ");
    print_dec_16_right(status.fan_speed_setting_pct, 5);
    print_str("%, ");
    print_str("Fan Speed  ");
    print_dec_16_right(status.fan_speed, 5);
    println_str("RPM.");

}
#endif
#endif

#if TRANSIT_THERMAL
static void print_chips_temp (void)
{
    print_txt(TXT_NO_TEMPERATURE);
    print_str(": ");
    parms[0] = phy_read_temp_reg(0);
    parms[1] = (71*parms[0]) / 100;
    if(parms[1] > 135) {
        print_str("Reg: ");
        print_hex_prefix();
        print_hex_w((ushort) parms[0]);
    } else {
        parms[0] = 135 - parms[1];
        print_dec(parms[0]);
        print_str(" C");
    }
#if defined(LUTON26_L25)
    print_str(", ");
    parms[0] = phy_read_temp_reg(12);
    parms[1] = (71*parms[0]) / 100;
    if(parms[1] > 135) {
        print_str("Reg: ");
        print_hex_prefix();
        print_hex_w((ushort) parms[0]);
    } else {
        parms[0] = 135 - parms[1];
        print_dec(parms[0]);
        print_str(" C");
    }
#endif

    print_cr_lf();
}
#endif /* TRANSIT_THERMAL */

#if TRANSIT_UNMANAGED_SYS_MAC_CONF
static uchar handle_sys_config (void)
{
    uchar ret;
    mac_addr_t mac_addr;

#if TRANSIT_LACP   
    vtss_lacp_system_config_t system_config;
#endif
    ret = retrieve_str_parms();
    if(ret != FORMAT_OK) {
        return ret;
    }

    if(!str_parms[0].len) {
        /* Dump all configurations */
        flash_read_mac_addr(&mac_addr);
        print_mac_addr(mac_addr);
        print_cr_lf();
    } else {
        if(cmp_cmd_txt(CMD_TXT_NO_MAC, str_parms[0].str))  {
            /* Update MAC addresses in RAM */
            ret = cmd_retrieve_mac_addr(str_parms[1].str, (uchar *)mac_addr);
            if(ret != FORMAT_OK) {
                return ret;
            }
            if(mac_cmp(mac_addr, mac_addr_0) == 0 ||
                    mac_addr[0] & 0x01)
                return FORMAT_ERROR;

            flash_write_mac_addr(&mac_addr);

#if TRANSIT_LLDP
            lldp_something_changed_local();
#endif

#if TRANSIT_LACP
            vtss_lacp_get_config(&system_config);
            mac_copy(system_config.system_id.macaddr, mac_addr);
            vtss_lacp_set_config(&system_config);
#endif

#if TRANSIT_SPI_FLASH
        } else if(cmp_cmd_txt(CMD_TXT_NO_SAVE, str_parms[0].str)) {
            /* Program configurations at RAM to flash */
            flash_read_mac_addr(&mac_addr);
            if(mac_cmp(mac_addr, mac_addr_0) == 0 || mac_cmp(mac_addr, spiflash_mac_addr) == 0 ||
                    mac_addr[0] & 0x01)
                return FORMAT_ERROR;
            flash_program_config();
#endif // TRANSIT_SPI_FLASH
        }
    }
    return FORMAT_OK;
}
#endif

#if TRANSIT_UNMANAGED_SWUP
/* ************************************************************************ */
static uchar cmd_cmp (char *s1, char *s2) small
/* ------------------------------------------------------------------------ --
 * Purpose     : Compare a string in RAM with a 0-terminated string in flash memory.
 * Remarks     : s1 points to string in RAM, s2 points to string in flash.
 *               Returns 0 if RAM string is equal to the flash string up till, but
 *               not including the 0-terminator. Otherwise 1 is returned.
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    uchar ch1;
    uchar ch2;

    for (;;) {
        ch2 = *s2;
        if (ch2 == 0) {
            return 0;
        }
        s2++;

        ch1 = conv_to_upper_case(*s1++);
        if (ch1 != ch2) {
            return 1;
        }
    }
}
#endif

#endif /* NO_DEBUG_IF */
#if !defined(BRINGUP)
static void print_phy_model_2_txt(unsigned char model)
{
    switch (model) {
        case PHY_MODEL_VTSS_8211:
        case PHY_MODEL_VTSS_8221:
            print_n_str("COBRA", 12);
        break;
        case PHY_MODEL_VTSS_8224:
        case PHY_MODEL_VTSS_8234:
        case PHY_MODEL_VTSS_8244:
            print_n_str("QUATTRO", 12);
        break;
        case PHY_MODEL_VTSS_8538:
        case PHY_MODEL_VTSS_8558:
        case PHY_MODEL_VTSS_8658:
            print_n_str("SPYDER", 12);
        break;
        case PHY_MODEL_VTSS_8664:
            print_n_str("ENZO", 12);
        break;
        case PHY_MODEL_VTSS_8512:
            print_n_str("ATOM", 12);
        break;
        case PHY_MODEL_VTSS_8504:
        case PHY_MODEL_VTSS_8552:
            print_n_str("TESLA", 12);
        break;
        case PHY_MODEL_VTSS_8514:
             print_n_str("ELISE_8514", 12);
        break;
        case PHY_MODEL_VTSS_7512:
            print_n_str("FERRET_7512", 12);
        break;
        case PHY_MODEL_NONE:
            print_n_str("Not PHY", 12);
        case PHY_MODEL_UNKNOWN:
        default:
        print_n_str("Unknown PHY", 12);
            break;
    }
}
static void port_info_format_output(uchar *fdata)
{
    uchar link_mode = fdata[6], speed;

    print_spaces(3);
    print_dec_8_right_2(fdata[0]);
    print_spaces(4);
    print_dec_8_right_2(fdata[1]);
    print_spaces(4);
    print_dec_8_right_2(fdata[2]);
    print_spaces(7);
    print_dec_8_right_2(fdata[3]);
    print_spaces(6);
    print_hex_prefix();
    print_hex_b(fdata[4]);
    print_spaces(1);
    if (phy_map(fdata[2])) {
#if defined(VTSS_ARCH_OCELOT)
        // PHY/Serdes model
        print_phy_model_2_txt(fdata[5]);
        print_spaces(1);

        // CRC uPatch
        if (fdata[5] == PHY_MODEL_VTSS_8514 || fdata[5] == PHY_MODEL_VTSS_7512 || fdata[5] == PHY_MODEL_VTSS_8221 || fdata[5] == PHY_MODEL_VTSS_8211) {
            print_n_str("No uPatch", 11);
        }
#endif
    } else {
        // PHY/Serdes model
        if (fdata[3] == MAC_IF_SFP_MSA_DETECTED) {
            print_n_str("Auto SFP", 12);
        } else if (fdata[3] == MAC_IF_100FX) {
            print_n_str("100M SFP", 12);
        } else if (fdata[3] == MAC_IF_SERDES_1G) {
            print_n_str("1G SFP", 12);
        } else if (fdata[3] == MAC_IF_SERDES_2_5G) {
            print_n_str("2.5G SFP", 12);
        } else {
            print_n_str("-", 12);
        }
        print_spaces(1);
        
        // CRC uPatch
        print_n_str("Not PHY", 11);
    }
    print_spaces(1);

    // Link mode
    if (link_mode == LINK_MODE_DOWN) {
        print_str("Down");
    } else {
        print_str("Up - ");
        speed = link_mode & LINK_MODE_SPEED_MASK;
        if (speed == LINK_MODE_SPEED_10) {
            print_str("10M");
        } else if (speed == LINK_MODE_SPEED_100) {
            print_str("100M");
        } else if (speed == LINK_MODE_SPEED_1000) {
            print_str("1G");
        } else if (speed == LINK_MODE_SPEED_2500) {
            print_str("2.5G");
        } else {
            print_str("Unknown");
        }
        if (link_mode & LINK_MODE_FDX_MASK) {
            print_str("FDX");
        } else {
            print_str("HDX");
        }

        // Flow control status
        print_spaces(1);
        print_str("FC(");
        print_str(fdata[7] ? "E)" : "D)");
    }
    print_cr_lf();
}

static void cli_show_port_info(void)
{
    phy_id_t        phy_id;
    vtss_uport_no_t uport_idx = 0;
    vtss_iport_no_t iport = 0;
    vtss_cport_no_t chip_port = 0;
    uchar           fdata[8];
    ulong           reg_val;

    print_str("uPort");
    print_spaces(1);
    print_str("iPort");
    print_spaces(1);
    print_str("cPort");
    print_spaces(1);
    print_str("MIIM Bus");
    print_spaces(1);
    print_str("MIIM Addr");
    print_spaces(3);
    print_str("PHY/Serdes");
    print_spaces(2);
#if defined(VTSS_ARCH_OCELOT)
    print_str("CRC uPatch");
    print_spaces(1);
#endif
    println_str("Link Status");
    print_line(78);
    print_cr_lf();

    for (uport_idx = 1; uport_idx <= NO_OF_BOARD_PORTS; uport_idx++) {
        iport = uport2iport(uport_idx);
        chip_port = iport2cport(iport);
        phy_read_id(chip_port, &phy_id);
        fdata[0] = uport_idx;
        fdata[1] = iport;
        fdata[2] = chip_port;
        fdata[3] = phy_map_miim_no(chip_port);
        fdata[4] = phy_map_phy_no(chip_port);
        fdata[5] = phy_id.model;
        fdata[6] = port_link_mode_get(chip_port);
        H2_READ(VTSS_SYS_PAUSE_CFG_MAC_FC_CFG(chip_port), reg_val);
        fdata[7] = VTSS_X_SYS_PAUSE_CFG_MAC_FC_CFG_TX_FC_ENA(reg_val) ? 1 : 0;
        port_info_format_output(fdata);
    }
}
#endif

#ifndef NO_DEBUG_IF

#if TRANSIT_UNMANAGED_MAC_OPER_GET
/* Show MAC address entries, chip_port=0xFF for all ports */
static void cli_show_mac_addr(vtss_cport_no_t chip_port)
{
    mac_tab_t mac_tab_entry;
    BOOL is_ipmc_entry;
    ulong status, total_cnt = 0;

    mac_tab_entry.vid = 0;
    mac_tab_entry.mac_addr[0] = mac_tab_entry.mac_addr[1] = mac_tab_entry.mac_addr[2] = mac_tab_entry.mac_addr[3] = mac_tab_entry.mac_addr[4] = mac_tab_entry.mac_addr[5] = 0;

    while(1) {
        status = h2_mactab_get_next(&mac_tab_entry, &is_ipmc_entry, TRUE);
        if (status == 0xFFFFFFFF) {
            break;
        }

        if (chip_port != 0xFF && !TEST_PORT_BIT_MASK(chip_port, &mac_tab_entry.port_mask)) {
            continue;
        }

        total_cnt++;

        // Show MAC address
        print_mac_addr(mac_tab_entry.mac_addr);
        print_spaces(2);

        // Show MAC address entry mode (static/dynamic)
        print_str(VTSS_X_ANA_ANA_TABLES_MACACCESS_ENTRY_TYPE(status) ? "S" : "D");
        print_spaces(2);

        // Show IPMC
        print_str(is_ipmc_entry ? "IPMC" : "    ");
        print_spaces(2);

        // Show CPU
        //print_str(VTSS_X_ANA_ANA_AGENCTRL_LEARN_SRC_KILL(status) ? "CPU" : "   ");
        //print_spaces(2);

        // Show portmask
        print_port_list(mac_tab_entry.port_mask);
        print_cr_lf();
    };

    // Show total count
    print_cr_lf();
    print_str("Total count = ");
    print_dec(total_cnt);
    print_cr_lf();
}
#endif // TRANSIT_UNMANAGED_MAC_OPER_GET

/* ************************************************************************ */
static void cli_show_sw_ver(void)
/* ------------------------------------------------------------------------ --
 * Purpose     :
 * Remarks     :
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
#if defined(BRINGUP)
    return;
#else
#ifndef UNMANAGED_REDUCED_DEBUG_IF
    vtss_cport_no_t chip_port;
    uchar port_ext;
    //ushort dat;
#endif

    /* Chip Family */
    sysutil_show_chip_id();

    /* Software version */
    sysutil_show_sw_ver();

    /* Image build time */
    sysutil_show_compile_date();

    /* HW Revision */
    sysutil_show_hw_ver();

#if defined(JUMBO)
    print_str("MaxFrame Size   :");
    print_spaces(1);
    print_dec(MAX_FRAME_SIZE);
    print_cr_lf();
#endif

#if TRANSIT_THERMAL
    /* chip temperature */
    print_chips_temp();
#endif

#ifndef UNMANAGED_REDUCED_DEBUG_IF
    /* Info about ports */
    print_cr_lf();
    println_str("uPort SMAC              MIIM PHY");
    println_str("----- ----------------- ---- ---");
    for (port_ext = 1; port_ext <= NO_OF_BOARD_PORTS; port_ext++) {
        chip_port = uport2cport(port_ext);
        if(!phy_map(chip_port)
#if MAC_TO_MEDIA
                && !phy_map_serdes(chip_port)
#endif
          ) {
            continue;
        }
        print_spaces(3);
        print_dec_8_right_2(port_ext);
        print_spaces(1);

        /* mac address, to be modified */
        print_port_mac_addr(chip_port);
        print_spaces(2);

        /* miim and phy number */
        print_spaces(1);
        print_dec_8_right_2(phy_map_miim_no(chip_port));
        print_spaces(1);
        print_dec_8_right_2(phy_map_phy_no(chip_port));
        print_cr_lf();
    }
#endif
#endif /* BRINGUP */
}
#endif

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
