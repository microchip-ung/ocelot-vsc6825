//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



/**
 * This file contains the macros for the all protocol modules
 * that must be adapted for the operating environment.
 *
 * This is the version for the 8051 environment.
 */

#ifndef _VTSS_COMMON_OS_H_
#define _VTSS_COMMON_OS_H_ 1

/* Standard include files */
#include <stdarg.h>
#include <string.h>
#include <absacc.h>

#include "common.h"     /* Always include common.h at the first place of user-defined herder files */
#include "hwport.h"
#include "spiflash.h"
#include "misc3.h"

typedef unsigned short vtss_common_port_t; /* Port numbers counted from 1 to VTSS_XXX_MAX_PORTS */

typedef unsigned short vtss_common_vlanid_t; /* VLAN numbers counted from 1 to VTSS_XXX_MAX_VLANS */

typedef unsigned char vtss_common_octet_t;

typedef unsigned long vtss_common_counter_t; /* Statistical counters */

typedef unsigned int vtss_common_framelen_t;

typedef unsigned char vtss_common_bool_t;
#define VTSS_COMMON_BOOL_FALSE          ((vtss_common_bool_t)0)
#define VTSS_COMMON_BOOL_TRUE           ((vtss_common_bool_t)1)

typedef unsigned long vtss_common_linkspeed_t; /* Link speed in Mbps */

typedef unsigned char vtss_common_linkstate_t; /* Port link state - down or up */
#define VTSS_COMMON_LINKSTATE_DOWN      ((vtss_common_linkstate_t)VTSS_COMMON_BOOL_FALSE)
#define VTSS_COMMON_LINKSTATE_UP        ((vtss_common_linkstate_t)VTSS_COMMON_BOOL_TRUE)

typedef unsigned char vtss_common_duplex_t;
#define VTSS_COMMON_LINKDUPLEX_HALF     ((vtss_common_duplex_t)VTSS_COMMON_BOOL_FALSE)
#define VTSS_COMMON_LINKDUPLEX_FULL     ((vtss_common_duplex_t)VTSS_COMMON_BOOL_TRUE)

typedef unsigned char vtss_common_pwr_mngr_t;
#define VTSS_COMMON_LINK_PWR_MNGR_DISABLE ((vtss_common_pwr_mngr_t)VTSS_COMMON_BOOL_FALSE)
#define VTSS_COMMON_LINK_PWR_MNGR_ENABLE  ((vtss_common_pwr_mngr_t)VTSS_COMMON_BOOL_TRUE)

/* Basically, the STP state is controlling both egrees and ingress frames on a port */
typedef unsigned char vtss_common_stpstate_t; /* Port Spanning Tree state */
#define VTSS_COMMON_STPSTATE_DISABLED   ((vtss_common_stpstate_t)0)
#define VTSS_COMMON_STPSTATE_BLOCKING   ((vtss_common_stpstate_t)1)
#define VTSS_COMMON_STPSTATE_LEARNING   ((vtss_common_stpstate_t)2)
#define VTSS_COMMON_STPSTATE_FORWARDING ((vtss_common_stpstate_t)3)
#define VTSS_COMMON_STPSTATE_ENABLED    ((vtss_common_stpstate_t)4)
#define VTSS_COMMON_STPSTATE_MAPLRN      /* In which states do we learn addresses */ \
                                                                                                (( 1 << VTSS_COMMON_STPSTATE_LEARNING) | \
                                                                                                 ( 1 << VTSS_COMMON_STPSTATE_FORWARDING) | \
                                                                                                 ( 1 << VTSS_COMMON_STPSTATE_ENABLED))
#define VTSS_COMMON_STPSTATE_MAPFWD     /* In which states do we forward frames */ \
                                                                                                (( 1 << VTSS_COMMON_STPSTATE_FORWARDING) | \
                                                                                                 ( 1 << VTSS_COMMON_STPSTATE_ENABLED))

/* And so is  FWD state (for LACP) */
typedef unsigned char vtss_common_fwdstate_t; /* Port forwarding state */
#define VTSS_COMMON_FWDSTATE_DISABLED   ((vtss_common_fwdstate_t)VTSS_COMMON_BOOL_FALSE)
#define VTSS_COMMON_FWDSTATE_ENABLED    ((vtss_common_fwdstate_t)VTSS_COMMON_BOOL_TRUE)

#define VTSS_COMMON_MACADDR_SIZE MAC_ADDR_LEN
typedef struct {
    vtss_common_octet_t macaddr[VTSS_COMMON_MACADDR_SIZE];
} vtss_common_macaddr_t;

typedef unsigned short vtss_common_ethtype_t;

typedef struct {
    vtss_common_macaddr_t dst_mac;
    vtss_common_macaddr_t src_mac;
    vtss_common_ethtype_t eth_type;
} vtss_common_macheader_t;

#define VTSS_COMMON_MACADDR_CMP(M1, M2)         mac_cmp(M1, M2)
#define VTSS_COMMON_MACADDR_ASSIGN(MDST, MSRC)  mac_copy(MDST, MSRC)
#define HOST2NETS(S)			(S)
#define NET2HOSTS(S)			(S)
#define HOST2NETL(L)			(L)
#define NET2HOSTL(L)			(L)

#define VTSS_COMMON_UNALIGNED_PUT_2B(DP, V)  *((unsigned short *)(DP)) = (V)
#define VTSS_COMMON_UNALIGNED_GET_2B(DP)     *((const unsigned short *)(DP))
#define VTSS_COMMON_UNALIGNED_PUT_4B(DP, V)  *((unsigned long *)(DP)) = (V)
#define VTSS_COMMON_UNALIGNED_GET_4B(DP)     *((const unsigned long *)(DP))

#define UNAL_NET2HOSTS(SP) NET2HOSTS(VTSS_COMMON_UNALIGNED_GET_2B(SP))
#define UNAL_HOST2NETS(SP) HOST2NETS(VTSS_COMMON_UNALIGNED_GET_2B(SP))
#define UNAL_NET2HOSTL(SP) NET2HOSTL(VTSS_COMMON_UNALIGNED_GET_4B(SP))
#define UNAL_HOST2NETL(SP) HOST2NETL(VTSS_COMMON_UNALIGNED_GET_4B(SP))

#define VTSS_COMMON_BUFMEM_ATTRIB       xdata /* Attribute for network buffer memmory */
#define VTSS_COMMON_DATA_ATTRIB         xdata /* Attribute for general data */
#define VTSS_COMMON_PTR_ATTRIB          xdata /* Attribute for misc pointers */
#define VTSS_COMMON_CODE_ATTRIB         code  /* Attribute for function pointers */

typedef uchar VTSS_COMMON_BUFMEM_ATTRIB *vtss_common_bufref_t;

typedef unsigned char vtss_common_trlevel_t;
#define VTSS_COMMON_TRLVL_ERROR         ((vtss_common_trlevel_t)1)
#define VTSS_COMMON_TRLVL_WARNING       ((vtss_common_trlevel_t)2)
#define VTSS_COMMON_TRLVL_DEBUG         ((vtss_common_trlevel_t)3)
#define VTSS_COMMON_TRLVL_NOISE         ((vtss_common_trlevel_t)4)
#define VTSS_COMMON_TRLVL_RACKET        VTSS_COMMON_TRLVL_NOISE

extern vtss_common_trlevel_t vtss_os_trace_level;

#if 0
#define VTSS_COMMON_RETURN_ADDRESS(LVL) MK_USHORT((unsigned short)DBYTE[SP], DBYTE[SP - 1])
#else
#define VTSS_COMMON_RETURN_ADDRESS(LVL) 0
#endif

#ifdef NDEBUG
#define VTSS_COMMON_NDEBUG 1
#define VTSS_COMMON_TRACE(LVL, ARGS)    /* Go away */
#define VTSS_COMMON_ASSERT(EXPR)        /* Go away */

#else
extern void vtss_os_trace(const char *fmt, ...);
extern void vtss_os_assert(unsigned line_number);

extern VTSS_COMMON_DATA_ATTRIB char _common_file[80];
extern ushort _common_line;
extern ushort _common_retaddr;
extern vtss_common_trlevel_t _common_lvl;

/* Very dirty macro trick, but what can you do with a C-preprocessor without varargs? */
#define VTSS_COMMON_TRACE(LVL, ARGS) \
do { \
    if ((LVL) <= vtss_os_trace_level) { \
        _common_retaddr = VTSS_COMMON_RETURN_ADDRESS(0); \
        strncpy(_common_file, __FILE__, sizeof(_common_file) - 1); \
        _common_line = __LINE__; \
        _common_lvl = (LVL); \
        vtss_os_trace ARGS; \
    } \
} while (0);

#define VTSS_COMMON_ASSERT(EXPR) \
do { \
    if (EXPR) \
        ; \
    else { \
        _common_retaddr = VTSS_COMMON_RETURN_ADDRESS(0); \
        strncpy(_common_file, __FILE__, sizeof(_common_file) - 1); \
        vtss_os_assert(__LINE__); \
    } \
} while (0)

extern void vtss_common_dump_frame(const vtss_common_octet_t VTSS_COMMON_PTR_ATTRIB *frame, vtss_common_framelen_t len);
#endif /* NDEBUG */

extern const char *vtss_common_str_linkstate(vtss_common_linkstate_t state);
extern const char *vtss_common_str_linkduplex(vtss_common_duplex_t duplex);
extern const char *vtss_common_str_stpstate(vtss_common_stpstate_t stpstate);

extern const char *vtss_common_str_macaddr(const vtss_common_macaddr_t VTSS_COMMON_PTR_ATTRIB *mac);
extern const char *vtss_common_str_ipaddr(const uchar VTSS_COMMON_PTR_ATTRIB *ip_addr);

/* Convert PROTO portnumber (1 - MAX) to 8051 extern portnumber */
#define PROTO2OSEXT(PORTNO)             (PORTNO)

/* Convert PROTO portnumber (1 - MAX) to 8051 internal (GMII) portnumber (MIN_PORT - MAX_PORT) */
#define PROTO2OSINT(PORTNO)             uport2cport(PROTO2OSEXT(PORTNO))

/* Convert OS extern portnumber to PROTO portnumber (1 - MAX_PORT) */
#define OSEXT2PROTO(PORTNO)             (PORTNO)

/* Convert OS internal (GMII) portnumber (MIN_PORT - MAX_PORT to PROTO portnumber (1 - MAX_PORT) */
#define OSINT2PROTO(PORTNO)             OSEXT2PROTO(cport2uport(PORTNO))

#define VTSS_COMMON_ZEROMAC             (mac_addr_0)

/**
 * vtss_os_get_linkspeed - Deliver the current link speed (in Kbps) of a specific port.
 */
extern vtss_common_linkspeed_t vtss_os_get_linkspeed(vtss_uport_no_t vtss_cport_no_t);

/**
 * vtss_os_get_linkstate - Deliver the current link state of a specific port.
 */
extern vtss_common_linkstate_t vtss_os_get_linkstate(vtss_common_port_t portno);

/**
 * vtss_os_get_linkduplex - Deliver the current link duplex mode of a specific port.
 */
extern vtss_common_duplex_t vtss_os_get_linkduplex(vtss_common_port_t portno);

/**
 * vtss_os_get_link_pwr_mngr_mode - Deliver the current link power management mode of a specific port.
 */
extern vtss_common_pwr_mngr_t vtss_os_get_link_pwr_mngr_mode(vtss_uport_no_t portno);

/**
 * vtss_os_get_systemmac - Deliver the MAC address for the switch.
 */
extern void vtss_os_get_systemmac(vtss_common_macaddr_t *system_macaddr);

/**
 * vtss_os_get_portmac - Deliver the MAC address for the a specific port.
 */
extern void vtss_os_get_portmac(vtss_common_port_t portno, vtss_common_macaddr_t *port_macaddr);

/**
 * vtss_os_set_stpstate - Set the Spanning Tree state of a specific port.
 */
extern void vtss_os_set_stpstate(vtss_common_port_t portno, vtss_common_stpstate_t new_state);

/**
 * vtss_os_get_stpstate - Get the Spanning Tree state of a specific port.
 */
extern vtss_common_stpstate_t vtss_os_get_stpstate(vtss_common_port_t portno);

/**
 * vtss_os_set_fwdstate - Set the forwarding state of a specific port.
 */
extern void vtss_os_set_fwdstate(vtss_common_port_t portno, vtss_common_fwdstate_t new_state);

/**
 * vtss_os_get_fwdstate - Get the forwarding state of a specific port.
 */
extern vtss_common_fwdstate_t vtss_os_get_fwdstate(vtss_common_port_t portno);

/**
 * vtss_os_flush_fdb - Clear the filtering database for a specific port.
 */
extern void vtss_os_flush_fdb(vtss_common_bool_t flushport, vtss_common_port_t portno,
                              vtss_common_bool_t flushvlan, vtss_common_vlanid_t vlanid);

/**
 * vtss_os_alloc_xmit - Allocate a buffer to be used for transmitting a frame.
 */
extern void VTSS_COMMON_BUFMEM_ATTRIB *vtss_os_alloc_xmit(vtss_common_port_t portno, vtss_common_framelen_t len, vtss_common_bufref_t *pbufref);

/**
 * vtss_os_xmit - Rransmit a frame on a specific port.
 */
extern int vtss_os_xmit(vtss_common_port_t portno, void VTSS_COMMON_BUFMEM_ATTRIB *frame, vtss_common_framelen_t len, vtss_common_bufref_t bufref);

/* Return codes from the vtss_os_xmit() function */
#define VTSS_COMMON_CC_OK         0
#define VTSS_COMMON_CC_GENERR     1 /* General error */

extern void vtss_os_init(void);

extern void vtss_os_deinit(void);

#endif /* _VTSS_COMMON_OS_H__ */
