//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



/****************************************************************************
 * Debug
 ****************************************************************************/
/* Enable debug in phydrv.c */
// #define PHYDRV_DEBUG_ENABLE

/* Enable debug in phytsk.c */
// #define PHYTSK_DEBUG_ENABLE

/* Enable debug in h2txtx.c */
// #define H2TXRX_DEBUG_ENABLE

/* Enable debug in h2mailc.c */
// #define H2_MAILC_DEBUG_ENABLE

/* Enable debug for buffer resource (watermark) */
// #define BUFFER_RESOURCE_DEBUG_ENABLE

/* Enable debug for port statistics */
#define UNMANAGED_PORT_STATISTICS_IF            1
#if UNMANAGED_PORT_STATISTICS_IF
#define UNMANAGED_PORT_STATISTICS_QOS           0
#endif // UNMANAGED_PORT_STATISTICS_IF

/* Enable debug command for flow control */
#define UNMANAGED_FLOW_CTRL_IF

/* Enable debug for TCAM */
#define UNMANAGED_TCAM_DEBUG_IF                 0

/* Enable debug for ANEG disabled and forced 1Gfdx speed for SGMII/SERDES interface
 * MUST set value =0x2 or 0x9 for CPORT_MAPTO_MIIMBUS accoringly
 */
//#define SGMII_SERDES_FORCE_1G_DEBUG_ENABLE
/****************************************************************************
 * MAC configuration
 ****************************************************************************/
/* System MAC configuration */
#define TRANSIT_UNMANAGED_SYS_MAC_CONF          1

/* MAC address entry operations: GET/GETNEXT */
#define TRANSIT_UNMANAGED_MAC_OPER_GET          1

/* MAC address entry operations: ADD/DELETE */
#define TRANSIT_UNMANAGED_MAC_OPER_SET          1


/****************************************************************************
 * Flow control default setting
 ****************************************************************************/
#define TRANSIT_FLOW_CTRL_DEFAULT               0 /* Disable */

/****************************************************************************
 * Enable flash driver for accessing MAC address stored in SPI flash
 ****************************************************************************/
#define TRANSIT_UNMANAGED_SWUP                  1
#define TRANSIT_SPI_FLASH                       1


/****************************************************************************
 * LACP
 ****************************************************************************/
#define TRANSIT_LAG                             0
#define TRANSIT_LACP                            0

#if TRANSIT_LACP
#if defined(TRANSIT_FLOW_CTRL_DEFAULT)
#undef TRANSIT_FLOW_CTRL_DEFAULT
#define TRANSIT_FLOW_CTRL_DEFAULT               0 /* Disable */
#endif // TRANSIT_FLOW_CTRL_DEFAULT
/* Disalbe FC on LACP-enabled ports */
//#define TRANSIT_LACP_FC_OPT
#endif //TRANSIT_LACP


/****************************************************************************
 * Enable/Disable Loop Detection / Protection
 ****************************************************************************/
#define TRANSIT_LOOPDETECT                      1


/****************************************************************************
 * ActiPHY
 ****************************************************************************/
#define TRANSIT_ACTIPHY                         1


/****************************************************************************
 * One-step End to End Transparent clock
 ****************************************************************************/
#define TRANSIT_E2ETC                           1
#define TRANSIT_TCAM_IS2                        1


/****************************************************************************
 * LLDP - IEEE802.1AB
 ****************************************************************************/
#define TRANSIT_LLDP                            1


/****************************************************************************
 * EEE
 ****************************************************************************/
#define TRANSIT_EEE                             0
#define TRANSIT_EEE_LLDP                        0


/****************************************************************************
 * MAILBOX communication - Configure the switch operation by the external CPU
 *                         interface through mailbox register
 *
 * The 32-bit register value is split into three parts:
 * <conf_oper>( 2-bit) - The operation between the embedded 8051 iCPU and external CPU
 * <conf_type>( 6-bit) - Configure type.
 * <conf_data>(24-bit) - Configure data, it depends on which configured type is defined.
 *
 * Only two features are supported now.
 *
 * 1. Set system MAC address.
 *    Possible value of <conf_type>( 6-bit) - H2MAILC_TYPE_SYS_MAC_SET_LOW(0), H2MAILC_TYPE_SYS_MAC_SET_HIGH(1)
 *    Format of <conf_data>(24-bit) - <sys_mac_higher_3_bytes>(24-bit) when <conf_type> = 0
 *                                  - <sys_mac_lower_3_bytes>(24-bit)  when <conf_type> = 1
 *    For local testing, type the following commands to set the system MAC address
      to 00-01-02-03-04-05
 *      1.1 w 0x71000000 0x0 0x20 0x80000102 --> Set the higher 3 bytes of the MAC address
 *      1.2 w 0x71000000 0x0 0x20 0x81030405 --> Set the lower 3 bytes of the MAC address
 *      1.3 w 0x71000000 0x0 0x20 0x82000000 --> Apply the new configuration
 *    After that, use command 'CONFIG' to check if the new configuration is applied.
 *
 * 2. Set LACP configuration.
 *    Possible value of <conf_type>( 6-bit) - H2MAILC_TYPE_LACP_ENABLE(3), H2MAILC_TYPE_LACP_DISABLE(4)
 *    Format of <conf_data>(24-bit) - <uport>(8-bit), <key>(16-bit)
 *    For local testing, type the following command to enable/disable LACP on uport 1.
 *      2.1 w 0x71000000 0x0 0x20 0x83010064 --> Enable with key 100(ox64) LACP on uport 1.
 *      2.2 w 0x71000000 0x0 0x20 0x84010064 --> Disable LACP on uport 1.
 *    After that, use command 'F' to check if the new configuration is applied.
 ****************************************************************************/
#define TRANSIT_MAILBOX_COMM                    0

