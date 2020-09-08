//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#include "common.h"     /* Always include common.h at the first place of user-defined herder files */
#include "vtss_common_os.h"
#include "sysutil.h"

#include "main.h"
#include "phytsk.h"
#include "hwport.h"
#include "h2.h"
#include "print.h"
#include "timer.h"
#include "h2txrx.h"
#include "uartdrv.h"
#if TRANSIT_LLDP
#include "lldp.h"
#endif /* TRANSIT_LLDP */

#if TRANSIT_EEE_LLDP
#include "eee_api.h"
#endif /* TRANSIT_EEE_LLDP */

#if TRANSIT_LACP
#define VTSS_LACP_PROTOCOL 1
#include "vtss_lacp.h"
#include "vtss_lacp_private.h"
#endif /* TRANSIT_LACP */

vtss_common_trlevel_t vtss_os_trace_level = VTSS_COMMON_TRLVL_DEBUG;

#ifndef VTSS_COMMON_NDEBUG
VTSS_COMMON_DATA_ATTRIB char _common_file[80] = __FILE__;
ushort _common_line = 0;
ushort _common_retaddr = 0;
vtss_common_trlevel_t _common_lvl = VTSS_COMMON_TRLVL_NOISE;

static const char *basenm(const char *fname)
{
    const char *cp;

    cp = strrchr(fname, '\\');
    if (cp == NULL)
        cp = strrchr(fname, '/');
    return (cp == NULL) ? fname : cp + 1;
}

void vtss_os_trace(const char *fmt, ...)
{
    va_list ap;

    if (_common_lvl > vtss_os_trace_level)
        return;

    vtss_printf("%s(%u) %c: ", basenm(_common_file),
                (unsigned)_common_line,
                "0EWDN"[_common_lvl]);
    va_start(ap, fmt);
    vtss_vprintf(fmt, ap);
    va_end(ap);
    if (fmt[strlen(fmt) - 1] != '\n')
        print_cr_lf();
}

void vtss_os_assert(unsigned line_number)
{
    uchar ch;

    vtss_printf("\nAssert file %s line %u called from 0x%x failed.\nType c - continue, r - reboot h - hang: ",
                (const char *)basenm(_common_file), (unsigned)line_number,
                (unsigned)_common_retaddr);
    do {
        while (!uart_byte_ready())
            /* Wait for it */;
        ch = uart_get_byte();
        switch (ch) {
        case 'c' :
        case 'C' :
            vtss_printf("\n Continuing execution\n");
            return;
        case 'r' :
        case 'R' :
            sysutil_reboot();
            break;
        }
    } while (ch != 'h' && ch != 'H');
    vtss_printf("\n hanging.\n");
    sysutil_assert_event_set(SYS_ASSERT_EVENT_OS);
    sysutil_hang();
}

void vtss_common_dump_frame(const vtss_common_octet_t VTSS_COMMON_PTR_ATTRIB *frame, vtss_common_framelen_t len)
{
#define MBUF    ((const vtss_common_macheader_t VTSS_COMMON_PTR_ATTRIB *)frame)
    vtss_common_framelen_t i;

    vtss_printf("Frame len %u dst %s",
                (unsigned)len, vtss_common_str_macaddr(&MBUF->dst_mac));
    vtss_printf(" src %s type 0x%x",
                vtss_common_str_macaddr(&MBUF->src_mac), (unsigned)MBUF->eth_type);
    for (i = 0; i < len; i++) {
        if ((i & 0xF) == 0) {
            print_cr_lf();
            print_dec_16_right(i, 5);
            print_ch(':');
            print_spaces(1);
        }
        print_hex_b(frame[i]);
        print_spaces(1);
    }
    print_cr_lf();
#undef MBUF
}
#endif /* !VTSS_COMMON_NDEBUG */

#ifndef NDEBUG
const char *vtss_common_str_macaddr(const vtss_common_macaddr_t VTSS_COMMON_PTR_ATTRIB *mac)
{
    static char VTSS_COMMON_DATA_ATTRIB buf[24];

    uart_redirect(buf);
    print_mac_addr(mac->macaddr);
    uart_redirect(NULL);
    return buf;
}
#endif

/**
 * vtss_os_get_linkspeed - Return link speed (in Mbps) for a given physical port
 * If link state is "down" the speed is returned as 0.
 * @portno: Port number (1 - VTSS_LACP_MAX_PORTS)
 *
 */
#if TRANSIT_EEE_LLDP 
vtss_common_linkspeed_t vtss_os_get_linkspeed(vtss_uport_no_t portno)
{
    static const ushort speed[] = { 10, 100, 1000 };
    uchar lm;

    VTSS_COMMON_ASSERT(portno > 0 && portno <= NO_OF_BOARD_PORTS);
    lm = port_link_mode_get(PROTO2OSINT(portno));
    if (lm == LINK_MODE_DOWN)
        return 0;
    return (vtss_common_linkspeed_t)speed[lm & LINK_MODE_SPEED_MASK];
}
#endif


#if TRANSIT_EEE
/**
 * vtss_os_get_link_pwr_mngr_mode - Return layer 2 power management mode for a
 * given physical port.
 * If phy register shows L2 capability,
 * returned as VTSS_COMMON_LINK_PWR_MNGR_ENABLE.
 * @portno: Port number (1 - VTSS_LACP_MAX_PORTS)
 *
 */
vtss_common_pwr_mngr_t vtss_os_get_link_pwr_mngr_mode(vtss_uport_no_t portno)
{
    uchar lm = VTSS_COMMON_LINK_PWR_MNGR_DISABLE;

    VTSS_COMMON_ASSERT(portno > 0 && portno <= NO_OF_BOARD_PORTS);
    lm = port_link_mode_get(PROTO2OSINT(portno));
    if (lm != LINK_MODE_DOWN) {
        return ((lm & LINK_MODE_POWER_MASK) >> 6);
    }
    return VTSS_COMMON_LINK_PWR_MNGR_DISABLE;
}
#endif


/**
 * vtss_os_get_systemmac - Return MAC address associated with the system.
 * @system_macaddr: Return the value.
 *
 */
#if TRANSIT_LLDP || TRANSIT_LACP
void vtss_os_get_systemmac(vtss_common_macaddr_t *system_macaddr)
{
    get_mac_addr(SYSTEM_MAC_ADDR, system_macaddr->macaddr);
}

/**
 * vtss_os_get_portmac - Return MAC address associated a specific physical port.
 * @portno: Port number (1 - VTSS_LACP_MAX_PORTS)
 * @port_macaddr: Return the value.
 *
 */
void vtss_os_get_portmac(vtss_common_port_t portno, vtss_common_macaddr_t *port_macaddr)
{
    VTSS_COMMON_ASSERT(portno > 0 && portno <= NO_OF_PORTS);
    get_mac_addr(PROTO2OSINT(portno), port_macaddr->macaddr);
}

#endif

#if TRANSIT_LACP
/**
 * vtss_os_make_key - Return key for a given physical port (based on link speed)
 * @portno: Port number (1 - VTSS_LACP_MAX_PORTS)
 *
 */
vtss_lacp_key_t vtss_os_make_key(vtss_common_port_t portno, vtss_lacp_key_t new_key)
{
    uchar lm;

#ifdef NON_STRICT_IEEE8023AD
    if (new_key == VTSS_LACP_AUTOKEY) {
        portno = LACP2OSINT(portno);
        lm = port_link_mode_get(portno);

        /* We don't really care what the value is when the link is down */
        return (vtss_lacp_key_t)((eeprom_read_pvid(portno) << 4) | ((lm & LINK_MODE_SPEED_MASK) + 1));
    }
    return new_key;
#else
    if (new_key == VTSS_LACP_AUTOKEY) {
        lm = port_link_mode_get(LACP2OSINT(portno));
        if (lm == LINK_MODE_DOWN) { //Ferret comes here
        	return (VTSS_LACP_AUTOKEY);
        } else {		
        	return (new_key << 8) | ((lm & LINK_MODE_SPEED_MASK) + 1);
        }	
    }
    return new_key;
#endif /* NON_STRICT_IEEE8023AD */
}
#endif /* TRANSIT_LACP */

#if TRANSIT_LACP
#define BUFSZ_LACP      VTSS_LACP_MAX_FRAME_SIZE
#else
#define BUFSZ_LACP      0
#endif /* TRANSIT_LACP */

#if TRANSIT_LACP

#define BUFSZ 128 /* MAX(BUFSZ_LACP, BUFSZ_RSTP) */

void VTSS_COMMON_BUFMEM_ATTRIB *vtss_os_alloc_xmit(vtss_common_port_t portno, vtss_common_framelen_t len, vtss_common_bufref_t *pbufref)
{
    static uchar VTSS_COMMON_BUFMEM_ATTRIB buffer[BUFSZ + 4]; /* +4 for the CRC */
    
    VTSS_COMMON_ASSERT(len <= BUFSZ);
    portno = portno;
    len = len;
    *pbufref = buffer;
    return (void VTSS_COMMON_BUFMEM_ATTRIB *)buffer;
}

/**
 * vtss_os_xmit - Transmit a MAC frame on a specific physical port.
 * @portno: Port number (1 - VTSS_LACP_MAX_PORTS)
 * @frame: A full MAC frame.
 * @len: The length of the MAC frame.
 *
 * Return: VTSS_LACP_CC_OK if frame was successfully queued for transmission.
 *         VTSS_LACP_CC_GENERR in case of error.
 */
int vtss_os_xmit(vtss_common_port_t portno, void VTSS_COMMON_BUFMEM_ATTRIB *frame, vtss_common_framelen_t len, vtss_common_bufref_t bufref)
{
    VTSS_COMMON_ASSERT(frame == bufref);
    VTSS_COMMON_ASSERT(portno > 0 && portno <= NO_OF_PORTS);
    frame = frame;
    h2_send_frame(PROTO2OSINT(portno), (uchar VTSS_COMMON_BUFMEM_ATTRIB *)bufref, len + 4); /* +4 for CRC */
    return VTSS_COMMON_CC_OK;
}

/**
 * vtss_os_get_linkstate - Return link state for a given physical port
 * @portno: Port number (1 - VTSS_LACP_MAX_PORTS)
 *
 */
vtss_common_linkstate_t vtss_os_get_linkstate(vtss_common_port_t portno)
{
    return port_link_mode_get(PROTO2OSINT(portno)) == LINK_MODE_DOWN ? VTSS_COMMON_LINKSTATE_DOWN : VTSS_COMMON_LINKSTATE_UP;
}

/**
 * vtss_os_get_linkduplex - Return link duplex mode for a given physical port
 * If link state is "down" the duplexmode is returned as VTSS_LACP_LINKDUPLEX_HALF.
 * @portno: Port number (1 - VTSS_LACP_MAX_PORTS)
 *
 */
vtss_common_duplex_t vtss_os_get_linkduplex(vtss_common_port_t portno)
{
    uchar lm;

    VTSS_COMMON_ASSERT(portno > 0 && portno <= NO_OF_PORTS);
    lm = port_link_mode_get(PROTO2OSINT(portno));
    if (lm == LINK_MODE_DOWN)
        return VTSS_COMMON_LINKDUPLEX_HALF;
    return (vtss_common_duplex_t)((lm & LINK_MODE_FDX_MASK) >> 4);
}


static ushort lacp_last_lbolt;

/*
 * lacp_timer_check() is called as often as possible.
 * The tick_count however is counting 1 msec.
 * Since we only need to call vtss_lacp_tick()
 * every 100 msec (= 1/10 sec) we will need
 * to see 100 ticks before we call vtss_lacp_tick().
 */
void lacp_timer_check(void)
{
#define	LACP_TICK_FACTOR	(1000 / VTSS_LACP_TICKS_PER_SEC)
    ushort nticks;

    nticks = tdiff(tick_count, lacp_last_lbolt);
#ifndef VTSS_LACP_NDEBUG
    if (nticks >= 2 * LACP_TICK_FACTOR) {
        static ushort last_report = 0;
        static ushort latecnt = 0;

        if (latecnt < 0xFFFF)
            latecnt++;
        
        if (tdiff(tick_count, last_report) >= 30 * LACP_TICK_FACTOR) { /* Only report every 30 secs */
            VTSS_LACP_TRACE(VTSS_LACP_TRLVL_WARNING, ("LACP Timer dropped %u time(s) in the last %u secs\n",
                                                      (unsigned)latecnt, (unsigned)((tick_count - last_report) / (10 * LACP_TICK_FACTOR))));
            last_report = tick_count;
            latecnt = 0;
        }
    }
#endif /* !VTSS_LACP_NDEBUG */
 
    if (nticks > 20 * LACP_TICK_FACTOR) {
        VTSS_LACP_TRACE(VTSS_LACP_TRLVL_WARNING, ("LACP Timer jumped more than 20 ticks: reset\n"));
        vtss_lacp_tick();
        lacp_last_lbolt = tick_count;
    }   
    else while (nticks >= LACP_TICK_FACTOR) {
        vtss_lacp_tick();
        lacp_last_lbolt += LACP_TICK_FACTOR;
        nticks -= LACP_TICK_FACTOR;
    }
#undef LACP_TICK_FACTOR
}

static void lacp_setup(void)
{
    vtss_common_port_t portno;
    vtss_lacp_port_config_t cfg;
    uchar pno;

#ifndef VTSS_LACP_NDEBUG
    vtss_printf("LACP from %s %s - all ports from eeprom\n", (char *)__DATE__, (char *)__TIME__);
#endif /* !VTSS_LACP_NDEBUG */
    cfg.port_prio = VTSS_LACP_DEFAULT_PORTPRIO;
    cfg.xmit_mode = VTSS_LACP_DEFAULT_FSMODE;
    cfg.active_or_passive = VTSS_LACP_DEFAULT_ACTIVITY_MODE;
    for (portno = 1; portno <= VTSS_LACP_MAX_PORTS; portno++) {
        pno = LACP2OSINT(portno);

#if defined(VTSS_ARCH_OCELOT)        
        cfg.enable_lacp = FALSE;  // Default value
        cfg.port_key = VTSS_LACP_AUTOKEY; // Default value
#else                
        cfg.enable_lacp = eeprom_read_lacp_enable(pno);
        cfg.port_key = eeprom_read_lacp_port_key(pno);
#endif        
        vtss_lacp_set_portconfig(portno, &cfg);
    }
    vtss_lacp_init();
    lacp_last_lbolt = tick_count;
}
#endif /* TRANSIT_LACP */

#if TRANSIT_LACP || TRANSIT_RSTP
void vtss_os_init(void)
{
#if TRANSIT_LACP
    lacp_setup();
#endif /* TRANSIT_LACP */
#if TRANSIT_RSTP
    rstp_setup();
#endif /* TRANSIT_RSTP */
}
#endif //TRANSIT_LACP || TRANSIT_RSTP


#if 0   // Uncalled function
void vtss_os_deinit(void)
{
#if TRANSIT_LACP
    vtss_lacp_deinit();
#endif /* TRANSIT_LACP */
#if TRANSIT_RSTP
    vtss_rstp_deinit();
#endif /* TRANSIT_RSTP */
}
#endif  // Uncalled function


/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
