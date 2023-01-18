//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT




/* ===========================================================================
 *
 * Normally, all features should be disabled by default.
 * Refer to the project configured file under directory src/config/proj_opt/
 * It lists the featues which you want to enable, i.e. proj_opt_ferret_release.h
 *
 * ==========================================================================/
 

/****************************************************************************
 * Debug
 ****************************************************************************/
/* Enable debug in h2port.c */
// #define HWPORT_DEBUG_ENABLE

/* Enable debug in phymap.c */
// #define PHYMAP_DEBUG_ENABLE

/* Enable debug in phydrv.c */
// #define PHYDRV_DEBUG_ENABLE

/* Enable debug in phytsk.c */
// #define PHYTSK_DEBUG_ENABLE

/* Enable debug in ledtsk.c */
// #define LEDTSK_DEBUG_ENABLE

/* Enable debug in h2pcs1g.c */
// #define H2_PCS1G_DEBUG_ENABLE

/* Enable debug in h2sdcfg.c */
// #define H2_DEBUG_ENABLE

/* Enable debug in h2sdcfg.c */
// #define H2_SD6G_DEBUG_ENABLE

/* Enable debug in h2txtx.c */
// #define H2TXRX_DEBUG_ENABLE

/* Define NO_DEBUG_IF to disable the UIs debug message */
// TODO, cannot define it
// #define NO_DEBUG_IF

/* Define NDEBUG to disable asserts and trace */
// TODO, cannot undefine it
#define NDEBUG

// TODO, cannot define 1
#ifndef UNMANAGED_PORT_MAPPINGS
#define UNMANAGED_PORT_MAPPINGS                 1   /* Enable print APIs in print.c */
#endif

#ifndef UNMANAGED_LLDP_DEBUG_IF
#define UNMANAGED_LLDP_DEBUG_IF                 0
#endif

#ifndef UNMANAGED_EEE_DEBUG_IF
#define UNMANAGED_EEE_DEBUG_IF                  0
#endif

#ifndef UNMANAGED_FAN_DEBUG_IF
#define UNMANAGED_FAN_DEBUG_IF                  0
#endif

#ifndef UNMANAGED_TRANSIT_VERIPHY_DEBUG_IF
#define UNMANAGED_TRANSIT_VERIPHY_DEBUG_IF      0
#endif

#ifndef UNMANAGED_PORT_STATISTICS_IF
#define UNMANAGED_PORT_STATISTICS_IF            0
#endif

#ifndef UNMANAGED_PORT_STATISTICS_QOS
#define UNMANAGED_PORT_STATISTICS_QOS           0
#endif

#if defined(UNMANAGED_ENHANCEMENT) && !defined(UNMANAGED_ENHANCEMENT_DEBUG)
#define UNMANAGED_ENHANCEMENT_DEBUG             1
#endif

/****************************************************************************
 * MAC configuration
 ****************************************************************************/
/* System MAC configuration */
#ifndef TRANSIT_UNMANAGED_SYS_MAC_CONF
#define TRANSIT_UNMANAGED_SYS_MAC_CONF          0
#endif

/* MAC address entry operations: GET/GETNEXT */
#ifndef TRANSIT_UNMANAGED_MAC_OPER_GET
#define TRANSIT_UNMANAGED_MAC_OPER_GET          0
#endif

/* MAC address entry operations: ADD/DELETE */
#ifndef TRANSIT_UNMANAGED_MAC_OPER_SET
#define TRANSIT_UNMANAGED_MAC_OPER_SET          0
#endif


/****************************************************************************
 * LACP
 ****************************************************************************/
#ifndef TRANSIT_LAG
#define TRANSIT_LAG                             0
#endif

#ifndef TRANSIT_LACP
#define TRANSIT_LACP                            0
#endif

// Dependence checking
#if TRANSIT_LACP && !TRANSIT_LAG
#error "Dependence checking error! - MUST define MUST TRANSIT_LAG=1 when LACP feature is enabled"
#endif


/****************************************************************************
 * Enable/Disable Loop Detection / Protection
 ****************************************************************************/
#ifndef TRANSIT_LOOPDETECT
#define TRANSIT_LOOPDETECT                      0
#endif


/****************************************************************************
 * ActiPHY
 ****************************************************************************/
#ifndef TRANSIT_ACTIPHY
#define TRANSIT_ACTIPHY                         0   /* Not implemented yet */
#endif


/****************************************************************************
 * End to End transparent clock
 ****************************************************************************/
#ifndef TRANSIT_E2ETC
#define TRANSIT_E2ETC                           0
#endif

#ifndef TRANSIT_TCAM_IS2
#define TRANSIT_TCAM_IS2                        0
#endif

#ifndef UNMANAGED_TCAM_DEBUG_IF 
#define UNMANAGED_TCAM_DEBUG_IF                 0 
#endif

/****************************************************************************
 * MAILBOX communication - Configure the switch operation by the external CPU
 *                         interface through mailbox register
 ****************************************************************************/
#ifndef TRANSIT_MAILBOX_COMM
#define TRANSIT_MAILBOX_COMM                    0
#endif


/****************************************************************************
 * LLDP - IEEE802.1AB
 ****************************************************************************/
#ifndef TRANSIT_LLDP
#define TRANSIT_LLDP                            0
#endif

/****************************************************************************
 * EEE
 ****************************************************************************/
#ifndef TRANSIT_EEE
#define TRANSIT_EEE                             0
#endif

#ifndef TRANSIT_EEE_LLDP
#define TRANSIT_EEE_LLDP                        0
#endif


/****************************************************************************
 * VeriPHY
 ****************************************************************************/
#ifndef TRANSIT_VERIPHY
#define TRANSIT_VERIPHY                         0   /* Not implemented yet */
#endif


/****************************************************************************
 * Perfect Reach
 ****************************************************************************/
#ifndef PERFECT_REACH_LNK_UP
#define PERFECT_REACH_LNK_UP                    0   /* Not implemented yet */
#endif

#ifndef PERFECT_REACH_LNK_DN
#define PERFECT_REACH_LNK_DN                    0   /* Not implemented yet */
#endif


/****************************************************************************
 * Software upgrade
 ****************************************************************************/
#ifndef TRANSIT_UNMANAGED_SWUP
#define TRANSIT_UNMANAGED_SWUP                  0   /* Not implemented yet */
#endif


/****************************************************************************
 * SPI Flash
 ****************************************************************************/
#ifndef TRANSIT_SPI_FLASH
#define TRANSIT_SPI_FLASH                       0   /* Not implemented yet */
#endif


/****************************************************************************
 * FTIME
 ****************************************************************************/
/**
 * Set TRANSIT_FTIME to 1 to enable system uptime tracking and provide
 * an API ftime() to get the uptime of the system.
 *
 * @note ftime is a 4.2BSD, POSIX.1-2001 API.
 */
#ifndef TRANSIT_FTIME
#define TRANSIT_FTIME                           0   /* Not implemented yet */
#endif


/****************************************************************************
 * FAN S/W Control
 ****************************************************************************/
/* FAN specifications is defined in fan_custom_api.h */
#ifndef TRANSIT_FAN_CONTROL
#define TRANSIT_FAN_CONTROL                     0   /* Not implemented yet */
#endif


/****************************************************************************
 * Thermal Control - This is de-speced from VSC7420, VSC7421, VSC7422
 ****************************************************************************/
#ifndef TRANSIT_THERMAL
#define TRANSIT_THERMAL                         0   /* Not implemented yet */
#endif


/****************************************************************************
 * PoE
 ****************************************************************************/
#ifndef TRANSIT_POE
#define TRANSIT_POE                             0   /* Not implemented yet */
#endif

#ifndef TRANSIT_POE_LLDP
#define TRANSIT_POE_LLDP                        0   /* Not implemented yet */
#endif


/****************************************************************************
 * Allow/Disable BPDU pass through the switch.
 ****************************************************************************/
#ifndef TRANSIT_BPDU_PASS_THROUGH
#define TRANSIT_BPDU_PASS_THROUGH               0   /* Not implemented yet */
#endif


/****************************************************************************
 * realated compling flags for LACP/trunk driver. Consider to remove later. !!!
 ****************************************************************************/
/* Set if Static trunk should load balancing when RSTP enabled */
#ifndef TRANSIT_RSTP_TRUNK_COWORK
#define TRANSIT_RSTP_TRUNK_COWORK               0   /* Not implemented yet */
#endif


/****************************************************************************
 * IEEE 802.1w protocol
 ****************************************************************************/
#ifndef TRANSIT_RSTP
#define TRANSIT_RSTP                            0   /* Not implemented yet */
#endif


/****************************************************************************
 * DOT1X
 ****************************************************************************/
#ifndef TRANSIT_DOT1X
#define TRANSIT_DOT1X                           0   /* Not implemented yet */
#endif


/****************************************************************************
 * SNMP
 ****************************************************************************/
#ifndef TRANSIT_SNMP
#define TRANSIT_SNMP                            0   /* Not implemented yet */
#endif

/****************************************************************************
 * Enable the switch initial procedure based on verification team's source code
 ****************************************************************************/
#if defined(VTSS_ARCH_OCELOT)
// Execute the initial procedure that provided by verification team
// Undefined FERRET_MAIN_ENABLE now since we done the task now.
// #define FERRET_MAIN_ENABLE
#endif // VTSS_ARCH_OCELOT


/****************************************************************************
 *
 * Do the cross check last
 *
 ****************************************************************************/
// LACP
#if TRANSIT_LACP
    #if TRANSIT_UNMANAGED_SYS_MAC_CONF == 0
    #error "Require TRANSIT_UNMANAGED_SYS_MAC_CONF"
    #endif
#endif // TRANSIT_LACP

// EEE
#if TRANSIT_EEE_LLDP
    #if TRANSIT_EEE == 0
    #error "Require TRANSIT_EEE"
    #endif

    #if TRANSIT_LLDP == 0
    #error "Require TRANSIT_LLDP"
    #endif
#endif // TRANSIT_EEE_LLDP

// Mailbox communication
#if TRANSIT_MAILBOX_COMM
    #if TRANSIT_UNMANAGED_SYS_MAC_CONF == 0
    #error "Require TRANSIT_UNMANAGED_MAC_OPER_SET"
    #endif

    #if TRANSIT_UNMANAGED_MAC_OPER_SET == 0
    #error "Require TRANSIT_UNMANAGED_MAC_OPER_SET"
    #endif
#endif // TRANSIT_MAILBOX_COMM
