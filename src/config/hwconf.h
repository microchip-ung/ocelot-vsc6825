//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#ifndef __HWCONF_H__
#define __HWCONF_H__

#ifndef __INCLUDE_CONFIGS__
#error "hwconf.h is for common.h only"
#endif

#include <REG52.H>
#include "phyconf.h"    // for NPI_CHIP_PORT

/*****************************************************************************
 *
 *
 * Internal 8051 registers
 *
 *
 *
 ****************************************************************************/

sfr GPR       = 0x80;
sfr PAGE_SEL  = 0xB0;
sfr EPAGE_SEL = 0xC0;
sfr MMAP      = 0xF2;
sfr RA_CTRL   = 0xF4;
sfr RA_AD0_RD = 0xF6;
sfr RA_AD0_WR = 0xF7;
sfr RA_AD1    = 0xF9;
sfr RA_AD2    = 0xFA;
sfr RA_AD3    = 0xFB;
sfr RA_DA0    = 0xFC;
sfr RA_DA1    = 0xFD;
sfr RA_DA2    = 0xFE;
sfr RA_DA3    = 0xFF;

sbit GPR0     = 0x80;
sbit GPR1     = 0x81;
sbit GPR2     = 0x82;
sbit GPR3     = 0x83;
sbit GPR4     = 0x84;
sbit GPR5     = 0x85;
sbit GPR6     = 0x86;
sbit GPR7     = 0x87;

/*****************************************************************************
 *
 *
 * Select chip interface
 *
 *
 ****************************************************************************/

/*
 * Define which chip interface to use, either 8-bit parallel interface,
 * SI interface or SFR interface.
 * Set USE_PI to 1 to use 8-bit interface, otherwise set it to 0.
 * Set USE_SI to 1 to use SI interface, otherwise set it to 0.
 * Set USE_SFR to 1 to use SFR interface, otherwise set it to 0.
 * Please, keep assembler file hwconf.inc updated accordingly.
 */

#define USE_PI  0
#define USE_SI  0
#define USE_SFR 1

/*****************************************************************************
 *
 *
 * MCU clock frequency
 *
 *
 ****************************************************************************/

#define CLOCK_FREQ          250000000

/*****************************************************************************
 *
 *
 * Serial port baud rate
 *
 *
 ****************************************************************************/

/*
 * Baud rate at RS232 interface.
 * Choose value among 115200, 57600, 38400, 19200 and 9600.
 */
#define BAUD_RATE 115200



/*****************************************************************************
 *
 *
 * GPIOs
 *
 *
 *
 ****************************************************************************/
#if defined(VTSS_ARCH_OCELOT)
#define USE_HW_TWI 1
#define USE_SW_TWI 0
#else
#define USE_HW_TWI 0
#define USE_SW_TWI 1
#endif

/*
 * I2C control signals
 */

#define GPIO_5_BIT          5UL
#define GPIO_6_BIT          6UL

#define GPIO_OE_5_BIT       5UL
#define GPIO_OE_6_BIT       6UL

#define SDA_BIT             GPIO_6_BIT
#define CLK_BIT             GPIO_5_BIT

#define SDA_BIT_OE          GPIO_OE_6_BIT
#define CLK_BIT_OE          GPIO_OE_5_BIT


/*****************************************************************************
 *
 *
 * Hardware testing utilities
 *
 *
 ****************************************************************************/
#if defined(VTSS_ARCH_OCELOT)
// Define HW_TEST_UART to enable UART test.
// #define HW_TEST_UART

// Define HW_TEST_TIMER to enable Timer test.
// #define HW_TEST_TIMER
#endif

/*****************************************************************************
 *
 *
 * Watchdog
 *
 *
 ****************************************************************************/

/*
 * Define whether watchdog is present and whether it should be enabled.
 * Set WATCHDOG_PRESENT to 1, if a watchdog is present and you want to use it,
 *  otherwise set it to 0
 * Set WATCHDOG_ENABLE to 1, if watchdog should be running all the time, otherwise
 * set it to 0 (in which case it will only be used in case of software reboot).
 */
#define WATCHDOG_PRESENT    0
#define WATCHDOG_ENABLE     0

/*
 * Define macros to enable and kick watchdog, see hwport.c.
 */


/*****************************************************************************
 *
 *
 * Control bits for paging
 *
 *
 ****************************************************************************/



/*****************************************************************************
 *
 *
 * Select presence of PHY loopback test
 *
 *
 ****************************************************************************/

/*
 * Define whether PHY loopback test should be supported
 * Set LOOPBACK_TEST to 1, if loopback test should be supported, otherwise set
 * it to 0
 */
#define LOOPBACK_TEST   0


/*
 * Define whether packet tx test should be supported
 * Set LOOPBACK_TEST to 1, if packet tx test  should be supported, otherwise set
 * it to 0
 */

#if defined(VTSS_ARCH_OCELOT)
#if TRANSIT_LLDP || TRANSIT_LACP
#define PKTTX_TEST  1
#else
#define PKTTX_TEST  0
#endif
#endif


/*****************************************************************************
 *
 *
 * Enable/disable switch chip id check
 *
 *
 ****************************************************************************/

/**
 * Define whether switch chip id should be checked as run-time check.
 * Set H2_ID_CHECK to 1 to enable check, or to 0 to disable check.
 */
#define H2_ID_CHECK 0

/*****************************************************************************
 *
 *
 * Configure any alive/debug LED
 *
 *
 *
 ****************************************************************************/

/*
** Define macro for turning on debug/alive LED. If no LED, define
** an empty macro.
*/
#define ALIVE_LED_ON {}

/*
** Define macro for toggling debug/alive LED. If no LED, define
** an empty macro.
*/
#define ALIVE_LED_TOGGLE {}

/*****************************************************************************
 *
 *
 * Configure any front panel LED
 *
 *
 *
 ****************************************************************************/

/*
** Define HW_LED_TOWER_PRESENT when the LED tower is present.
*/
#if defined(HW_LED_TOWER_PRESENT)
    #error "Not defined yet: HW_LED_TOWER_GPIO_PORT"
#endif

#if defined(HW_LED_TOWER_PRESENT)
    #error "Not defined yet: HW_LED_TOWER_GPIO_PORT"
#endif

/*
** Define SFP TX DISABLE GPIO pin when the SFP ports is supported on the board.
** (needs to refer to hardware schematic)
*/
#if defined(FERRET_F11)
    // SFP1
    #define SFP1_CHIP_PORT          8
    #define GPIO_SFP1_TXDISABLE     5
    #define GPIO_SFP1_PRESENT       8
    #define GPIO_SFP1_LOS           10
    #define GPIO_SFP1_RATESEL       18 /* SFP rate selector pin */

    // SFP2
    #define SFP2_CHIP_PORT          10
    #define GPIO_SFP2_TXDISABLE     17
    #define GPIO_SFP2_PRESENT       9
    #define GPIO_SFP2_LOS           11
    #define GPIO_SFP2_RATESEL       19 /* SFP rate selector pin */

#elif defined(FERRET_F10P)
    // SFP1
    #define SFP1_CHIP_PORT          8
    #define GPIO_SFP1_TXDISABLE     5
    #define GPIO_SFP1_PRESENT       8
    #define GPIO_SFP1_LOS           10
    #define GPIO_SFP1_RATESEL       18 /* SFP rate selector pin */

    // SFP2
    #define SFP2_CHIP_PORT          9
    #define GPIO_SFP2_TXDISABLE     17
    #define GPIO_SFP2_PRESENT       9
    #define GPIO_SFP2_LOS           11
    #define GPIO_SFP2_RATESEL       19 /* SFP rate selector pin */

#elif defined(FERRET_F5)
    // SFP2
    #define SFP2_CHIP_PORT          10
    #define GPIO_SFP2_TXDISABLE     17
    #define GPIO_SFP2_PRESENT       9
    #define GPIO_SFP2_LOS           11
    #define GPIO_SFP2_RATESEL       19 /* SFP rate selector pin */
#endif

/*
** Define whether a red/green front LED is present.
** Set FRONT_LED_PRESENT to 1, if present, otherwise set it to 0
*/
#if !defined(BRINGUP) && (defined(FERRET_F11) || defined(FERRET_F10P) || defined(FERRET_F5) || defined(FERRET_F4P))
#define FRONT_LED_PRESENT  1
#else
#define FRONT_LED_PRESENT  0
#endif

/*
** Define LED mode.
*/
#define LED_MODE_OFF 0
#define LED_MODE_ON  1
#define LED_LINK_ACTIVE_MODE 6


/*****************************************************************************
 *
 *
 * Enable/disable jumbo frame support
 *
 *
 *
 ****************************************************************************/

/*
** Define JUMBO when jumbo frames feature is supported.
*/
#define JUMBO

/*
** Define size of jumbo frames, provided JUMBO is set to 1.
*/
/* A selection of max frame lengths */
#define VTSS_MAX_FRAME_LENGTH_STANDARD  1518  /**< IEEE 802.3 standard */
#define VTSS_MAX_FRAME_LENGTH_MAX       10240

#if defined(JUMBO)
#define MAX_FRAME_SIZE 9600
#else
#define MAX_FRAME_SIZE VTSS_MAX_FRAME_LENGTH_STANDARD
#endif // JUMBO

/****************************************************************************
 *
 *
 * SFP Ports Support Functionality Configuration.
 *
 *
 ****************************************************************************/

/* Define if on some ports, MAC to SerDes directly */
#if defined(VTSS_ARCH_OCELOT)
    #define MAC_TO_MEDIA                        1
    #define TRANSIT_SFP_DETECT                  1
#else
    #define MAC_TO_MEDIA                        0
    #define TRANSIT_SFP_DETECT                  0
#endif


#if defined(VTSS_ARCH_OCELOT) && defined(SGMII_SERDES_FORCE_1G_DEBUG_ENABLE)
    #undef TRANSIT_SFP_DETECT
    #define TRANSIT_SFP_DETECT                  0 //because of ANEG disabled
#endif
/*****************************************************************************
 *
 *
 * Port mapping
 *
 *
 ****************************************************************************/
#if defined(FERRET_F11)
/*
** Define the mapping from chip port (zero-based) to user port (one-based).
   -------------------------------------------
   | 2(D5) | 4(D7) | 6(D1) | 8(D3) | 10(D10) |
   -------------------------------------------
   | 1(D4) | 3(D6) | 5(D0) | 7(D2) |  9( D8) |    NPI: 11(D9)
   -------------------------------------------
** Port number: The array size need to refer the definition NO_OF_CHIP_PORTS in common.h
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10
  -------------------------------------------------------------------------- */
#if defined(NPI_CHIP_PORT)
    #define CPORT_MAPTO_UPORT   { 5, 6, 7, 8, 1, 2, 3, 4, 9, 11, 10 }
#else // None NPI
    #define CPORT_MAPTO_UPORT   { 5, 6, 7, 8, 1, 2, 3, 4, 9,  0, 10 }
#endif // NPI_CHIP_PORT


/*
** Define the mapping from user port (one-based) --> chip port (zero-based)
** Port number: The array size need to refer the definition NO_OF_BOARD_PORTS in common.h
  1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11
  ---------------------------------------------- */
#if defined(NPI_CHIP_PORT)
#define UPORT_MAPTO_CPORT       { 4, 5, 6, 7, 0, 1, 2, 3, 8, 10, 9 }
#else // None NPI
#define UPORT_MAPTO_CPORT       { 4, 5, 6, 7, 0, 1, 2, 3, 8, 10 }
#endif // NPI_CHIP_PORT

#elif defined(FERRET_F10P)
/*
** Define the mapping from chip port (zero-based) to user port (one-based).
   ------------------------------------------
   | 2(D5) | 4(D7) | 6(D1) | 8(D3) | 10(D9) |
   ------------------------------------------
   | 1(D4) | 3(D6) | 5(D0) | 7(D2) |  9(D8) |   PCIe: (D10)
   ------------------------------------------
** port number: The array size need to refer the definition NO_OF_CHIP_PORTS in common.h
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10
  -------------------------------------------------------------------------- */
#define CPORT_MAPTO_UPORT       { 5, 6, 7, 8, 1, 2, 3, 4, 9, 10, 0 }

/*
** Define the mapping from user port (one-based) --> chip port (zero-based)
** Port number: The array size need to refer the definition NO_OF_BOARD_PORTS in common.h
  1, 2, 3, 4, 5, 6, 7, 8, 9, 10
  ---------------------------------------------- */
#define UPORT_MAPTO_CPORT       { 4, 5, 6, 7, 0, 1, 2, 3, 8, 9 }

#elif defined(FERRET_F5)
/*
** Define the mapping from chip port (zero-based) to user port (one-based).
   --------------------------------------
   | x | x | 2(D1) | 4(D3) | NPI: 5(D10)|
   --------------------------------------
   | x | x | 1(D0) | 3(D2) |   x        |
   --------------------------------------
** Port number: The array size need to refer the definition NO_OF_CHIP_PORTS in common.h
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10
  -------------------------------------------------------------------------- */
#if defined(NPI_CHIP_PORT)
#define CPORT_MAPTO_UPORT       { 1, 2, 3, 4, 0, 0, 0, 0, 0, 0, 5 }
#else // None NPI
#define CPORT_MAPTO_UPORT       { 1, 2, 3, 4, 0, 0, 0, 0, 0, 0, 0 }
#endif // NPI_CHIP_PORT

/*
** Define the mapping from user port (one-based) --> chip port (zero-based)
** Port number: The array size need to refer the definition NO_OF_BOARD_PORTS in common.h
  1, 2, 3, 4, 5
  ---------------------------------------------- */
#if defined(NPI_CHIP_PORT)
#define UPORT_MAPTO_CPORT       { 0, 1, 2, 3, 10 }
#else // None NPI
#define UPORT_MAPTO_CPORT       { 0, 1, 2, 3 }
#endif // NPI_CHIP_PORT

#elif defined(FERRET_F4P)
/*
** Define the mapping from chip port (zero-based) to user port (one-based).
   -------------------------
   | x | x | 2(D1) | 4(D3) |
   -------------------------
   | x | x | 1(D0) | 3(D2) |    PCIe: (D10)
   -------------------------
** Port number: The array size need to refer the definition NO_OF_CHIP_PORTS in common.h
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10
  -------------------------------------------------------------------------- */
#define CPORT_MAPTO_UPORT       { 1, 2, 3, 4, 0, 0, 0, 0, 0, 0, 0 }

/*
** Define the mapping from user port (one-based) --> chip port (zero-based)
** Port number: The array size need to refer the definition NO_OF_BOARD_PORTS in common.h
  1, 2, 3, 4
  ---------------------------------------------- */
#define UPORT_MAPTO_CPORT       { 0, 1, 2, 3 }

#else
#error "Not defined yet: port mapping"
#endif


#endif /* __HWCONF_H__ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
