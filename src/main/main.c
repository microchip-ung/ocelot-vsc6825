//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#include "common.h"     /* Always include common.h at the first place of user-defined herder files */
#include "sysutil.h"
#include "event.h"

#include "interrupt.h"
#include "hwport.h"
#include "spiflash.h"
#include "timer.h"
#include "uartdrv.h"
#include "phytsk.h"
#include "print.h"
#include "clihnd.h"
#include "phydrv.h"
#include "phymap.h"
#include "phy_family.h"
#include "phy_base.h"
#include "h2.h"
#include "h2txrx.h"
#include "h2gpios.h"

#include "misc2.h"
#include "i2c_h.h"
#include "taskdef.h"
#include "h2sdcfg.h"

#if TRANSIT_FAN_CONTROL
#include "fan_api.h"
#endif

#if TRANSIT_SNMP
#include "snmp.h"
#include "traps.h"
#endif /* TRANSIT_SNMP */

#if TRANSIT_LLDP
#include "lldp.h"
#endif /* TRANSIT_LLDP */

#if TRANSIT_LACP
#include "vtss_lacp.h"
#endif /* TRANSIT_LACP */

#if TRANSIT_EEE
#include "eee_api.h"
#include "eee_base_api.h"
#endif

#if TRANSIT_LAG
#include "h2aggr.h"
#endif /* TRANSIT_LAG */

#if TRANSIT_LOOPDETECT
#include "loopdet.h"
#endif

#if TRANSIT_POE
#include "poetsk.h"
#endif /* TRANSIT_POE */

#if TRANSIT_POE_LLDP
#include "poe_os.h"
#endif

#if FRONT_LED_PRESENT
#include "ledtsk.h"
#endif

#include "main.h"
#include "misc2.h"  
#include "h2io.h"   

#if TRANSIT_E2ETC
#include "h2e2etc.h"
#if TRANSIT_TCAM_IS2
#include "h2tcam.h"
#endif
#endif

#if TRANSIT_MAILBOX_COMM
#include "h2mailc.h"
#endif // TRANSIT_MAILBOX_COMM

/*
 * Debug enable/disable on local file
 */
#define MAIN_DEBUG_ENABLE
#if defined(MAIN_DEBUG_ENABLE)
#include "version.h"
#endif

/*****************************************************************************
 *
 *
 * Public data
 *
 *
 ****************************************************************************/

#if defined(UNMANAGED_ENHANCEMENT) && defined(VTSS_ARCH_OCELOT)
/* variable used in main/l51_bank.a51 */
data uchar  rom_page_mask;
#endif

/*****************************************************************************
 *
 *
 * Defines
 *
 *
 ****************************************************************************/


/*****************************************************************************
 *
 *
 * Typedefs and enums
 *
 *
 ****************************************************************************/


/*****************************************************************************
 *
 *
 * Local data
 *
 *
 ****************************************************************************/
static ulong up_time = 0; // the unit is second


/*****************************************************************************
 *
 *
 * Local Functions
 *
 *
 ****************************************************************************/


#if !defined(BRINGUP)
static void error_check (void)
{
#if H2_ID_CHECK
    if (!h2_chip_family_support()) {
        sysutil_assert_event_set(SYS_ASSERT_EVENT_CHIP_FAMILY);
    }
#endif // H2_ID_CHECK

    if (phy_check_all()) {
        sysutil_assert_event_set(SYS_ASSERT_EVENT_PHY);
    }

}
#endif /* !BRINGUP */

#if defined(HW_TEST_UART)
void sleep_ms(u32 ms)
{
  /* Set the timer value (the default timer ticks every 100 us) */
  H2_WRITE(VTSS_ICPU_CFG_TIMERS_TIMER_VALUE(0), ms * 10);

  /* Enable timer 0 for one-shot */
  H2_WRITE(VTSS_ICPU_CFG_TIMERS_TIMER_CTRL(0),
         VTSS_F_ICPU_CFG_TIMERS_TIMER_CTRL_ONE_SHOT_ENA(1) |
         VTSS_F_ICPU_CFG_TIMERS_TIMER_CTRL_TIMER_ENA(1));

  /* Wait for timer 0 to reach 0 */
  while (H2_READ(VTSS_ICPU_CFG_TIMERS_TIMER_VALUE(0)));
}
#endif /* HW_TEST_UART */

#if 0  //uncall functions
char getchar() {
  /* Wait for Data Ready */
  while (!VTSS_X_UART_UART_LSR_DR(H2_READ(VTSS_UART_UART_LSR(VTSS_TO_UART))));

  /* Read Rx data */
  return H2_READ(VTSS_UART_UART_RBR_THR(VTSS_TO_UART));
}
#endif

#if !defined(NO_DEBUG_IF) && defined(HW_TEST_TIMTER)
void debug_test_timer(void)
{
    //timer
    if (ms_10_timeout_flag) {
        ms_10_timeout_flag = 0;
        println_str("ms_10_timeout_flag*");
    }
    if (ms_100_timeout_flag) {
        ms_100_timeout_flag = 0;
        println_str("ms_100_timeout_flag**");
    }
    if (sec_1_timeout_flag) {
        sec_1_timeout_flag = 0;
        println_str("sec_1_timeout_flag***");
    }
}
#endif /* HW_TEST_TIMTER */

#if !defined(NO_DEBUG_IF) && defined(HW_TEST_UART)
void debug_test_uart(void)
{
    static char toggle_cnt = 0;

    println_str("*********************************");
    print_str("VTSS_DEVCPU_GCB_GPIO_GPIO_ALT(0):0x");
    print_hex_dw(H2_READ(VTSS_DEVCPU_GCB_GPIO_GPIO_ALT(0)));
    print_cr_lf();
    print_str("VTSS_DEVCPU_GCB_GPIO_GPIO_ALT(1):0x");
    print_hex_dw(H2_READ(VTSS_DEVCPU_GCB_GPIO_GPIO_ALT(1)));
    print_cr_lf();

    //THR
    print_str("VTSS_UART_UART_RBR_THR(VTSS_TO_UART):0x");
    print_hex_dw(H2_READ(VTSS_UART_UART_RBR_THR(VTSS_TO_UART)));
    print_spaces(4);
    print_ch(H2_READ(VTSS_UART_UART_RBR_THR(VTSS_TO_UART)));
    print_cr_lf();

    //IER
    print_str("VTSS_UART_UART_IER(VTSS_TO_UART):0x");
    print_hex_dw(H2_READ(VTSS_UART_UART_IER(VTSS_TO_UART)));
    print_cr_lf();

    //FCR
    print_str("VTSS_UART_UART_IIR_FCR(VTSS_TO_UART):0x");
    print_hex_dw(H2_READ(VTSS_UART_UART_IIR_FCR(VTSS_TO_UART)));
    print_cr_lf();

    //LCR
    print_str("VTSS_UART_UART_LCR(VTSS_TO_UART):0x");
    print_hex_dw(H2_READ(VTSS_UART_UART_LCR(VTSS_TO_UART)));
    print_cr_lf();

    //MCR
    print_str("VTSS_UART_UART_MCR(VTSS_TO_UART):0x");
    print_hex_dw(H2_READ(VTSS_UART_UART_MCR(VTSS_TO_UART)));
    print_cr_lf();

    //LSR
    print_str("VTSS_UART_UART_LSR(VTSS_TO_UART):0x");
    print_hex_dw(H2_READ(VTSS_UART_UART_LSR(VTSS_TO_UART)));
    print_cr_lf();

    //MSR
    print_str("VTSS_UART_UART_MSR(VTSS_TO_UART):0x");
    print_hex_dw(H2_READ(VTSS_UART_UART_MSR(VTSS_TO_UART)));
    print_cr_lf();

    //SCR
    print_str("VTSS_UART_UART_SCR(VTSS_TO_UART):0x");
    print_hex_dw(H2_READ(VTSS_UART_UART_SCR(VTSS_TO_UART)));
    print_cr_lf();

    //RESERVED1
    print_str("VTSS_UART_UART_RESERVED1(VTSS_TO_UART,0):0x");
    print_hex_dw(H2_READ(VTSS_UART_UART_RESERVED1(VTSS_TO_UART,0)));
    print_cr_lf();

    //USR
    print_str("VTSS_UART_UART_USR(VTSS_TO_UART):0x");
    print_hex_dw(H2_READ(VTSS_UART_UART_USR(VTSS_TO_UART)));
    print_cr_lf();

    //VTSS_UART_UART_RESERVED2
    print_str("VTSS_UART_UART_RESERVED2(VTSS_TO_UART, 0):0x");
    print_hex_dw(H2_READ(VTSS_UART_UART_RESERVED2(VTSS_TO_UART, 0)));
    print_cr_lf();


    //VTSS_UART_UART_RESERVED2
    print_str("VTSS_UART_UART_HTX(VTSS_TO_UART):0x");
    print_hex_dw(H2_READ(VTSS_UART_UART_HTX(VTSS_TO_UART)));
    print_cr_lf();
    print_cr_lf();
    print_cr_lf();

    sleep_ms(5000);
    if (++toggle_cnt % 2) {
        H2_WRITE_MASKED(VTSS_UART_UART_MCR(VTSS_TO_UART), VTSS_F_UART_UART_MCR_LB(1), VTSS_M_UART_UART_MCR_LB);
    }   else {
        H2_WRITE_MASKED(VTSS_UART_UART_MCR(VTSS_TO_UART), 0, VTSS_M_UART_UART_MCR_LB);
    }
}
#endif /* HW_TEST_UART */

static void handle_timeout_event_10ms(void)
{
    if (!ms_10_timeout_flag) {
        return;
    }

#if TRANSIT_MAILBOX_COMM
    TASK(TASK_ID_MAILBOX_COMM, h2mailc_tsk());
#endif // TRANSIT_MAILBOX_COMM

    TASK(TASK_ID_PHY_TIMER, phy_timer_10());
    TASK(TASK_ID_PHY, phy_tsk());
    
#if TRANSIT_LACP
            TASK(TASK_ID_LACP_TIMER, lacp_timer_check());
#endif /* TRANSIT_LACP */

#if (WATCHDOG_PRESENT && WATCHDOG_ENABLE)
    TASK(TASK_ID_WATCHDOG, kick_watchdog());
#endif

    // Clear event flag
    ms_10_timeout_flag = 0;
}

static void handle_timeout_event_100ms(void)
{
    if (!ms_100_timeout_flag) {
        return;
    }

#if FRONT_LED_PRESENT
    led_tsk();
#endif

#if TRANSIT_LOOPDETECT
     TASK(TASK_ID_LOOPBACK_CHECK, ldettsk());
#endif

#if TRANSIT_EEE
    TASK(TASK_ID_EEE, eee_mgmt());
#endif

    // Clear event flag
    ms_100_timeout_flag = 0;
}

static void handle_timeout_event_1sec(void)
{
    if (!sec_1_timeout_flag) {
        return;
    }

#if FRONT_LED_PRESENT
    led_1s_timer();
#endif

#if TRANSIT_THERMAL
    phy_handle_temperature_protect();
    phy_temperature_timer_1sec ();
#endif

#if TRANSIT_LLDP
    TASK(TASK_ID_LLDP_TIMER, lldp_1sec_timer_tick());
    TASK(TASK_ID_TIMER_SINCE_BOOT, time_since_boot_update());
#endif /* TRANSIT_LLDP */

#if !defined(BRINGUP)
    /*
     * Check H2 and PHYs
     */
    TASK(TASK_ID_ERROR_CHECK, error_check());
#endif /* !BRINGUP */

#if TRANSIT_EEE
    callback_delayed_eee_lpi();
#endif /* TRANSIT_EEE */

#if TRANSIT_FAN_CONTROL
    TASK(TASK_ID_FAN_CONTROL, fan_control());
#endif

#if TRANSIT_LOOPDETECT
     ldet_aging_1s();
#endif

    /* toggle any alive LED */
    ALIVE_LED_TOGGLE;

    // Clear event flag
    sec_1_timeout_flag = 0;
    up_time++;
}

/*****************************************************************************
 *
 *
 * Public Functions
 *
 *
 ****************************************************************************/
#if 0 // Uncalled function
ulong get_system_uptime()
{
    return up_time; // the unit is second
}    
#endif // Uncalled function


#if defined(FERRET_F11) || defined(FERRET_F10P) || defined(FERRET_F4P)
static void _ferret_serdes1g_4_patch(void)
{
    /* Bugzilla#20911 - Hardware pins S4_RXN#3 and S4_RXP#3 are swapped on SERDES1G_4 interface.
     *
     *  On current (rev.1) boards there is no external pull on Ferret's MII_MDC output
     * (GPIO_14), hence when used as a GPIO (e.g. before using it as MII_MDC / alternate mode 01)
     * it will read as high due to the internal pullup resistor on the pin.
     *
     * On future (e.g. rev.2) boards I will add a 1K pulldown (R92 in the attached schematics)
     * to GPIO_14, so that it will read as low (until the pin is configured as MII_MDC output,
     * of course).
     *
     * Notice the patch only be executed when SERDES1G_4 is in used and GPIO_14 = high.
     */
    ulong addr = 1 << 4;
    ulong dat;

    // The same as h2_sd1g_read()
    h2_write(VTSS_HSIO_MCB_SERDES1G_CFG_MCB_SERDES1G_ADDR_CFG,
             VTSS_F_HSIO_MCB_SERDES1G_CFG_MCB_SERDES1G_ADDR_CFG_SERDES1G_ADDR(addr) |
             VTSS_F_HSIO_MCB_SERDES1G_CFG_MCB_SERDES1G_ADDR_CFG_SERDES1G_RD_ONE_SHOT(1));

    /* Wait until write operation is completed  */
    do {
        dat = h2_read(VTSS_HSIO_MCB_SERDES1G_CFG_MCB_SERDES1G_ADDR_CFG);
    } while(VTSS_X_HSIO_MCB_SERDES1G_CFG_MCB_SERDES1G_ADDR_CFG_SERDES1G_RD_ONE_SHOT(dat));

    h2_write_masked(VTSS_HSIO_SERDES1G_DIG_CFG_SERDES1G_MISC_CFG,
                    VTSS_F_HSIO_SERDES1G_DIG_CFG_SERDES1G_MISC_CFG_RX_DATA_INV_ENA(1),
                    VTSS_M_HSIO_SERDES1G_DIG_CFG_SERDES1G_MISC_CFG_RX_DATA_INV_ENA);

    // The same as h2_sd1g_write()
    h2_write(VTSS_HSIO_MCB_SERDES1G_CFG_MCB_SERDES1G_ADDR_CFG,
             VTSS_F_HSIO_MCB_SERDES1G_CFG_MCB_SERDES1G_ADDR_CFG_SERDES1G_ADDR(addr) |
             VTSS_F_HSIO_MCB_SERDES1G_CFG_MCB_SERDES1G_ADDR_CFG_SERDES1G_WR_ONE_SHOT(1));

    /* Wait until write operation is completed  */
    do {
        dat = h2_read(VTSS_HSIO_MCB_SERDES1G_CFG_MCB_SERDES1G_ADDR_CFG);
    } while(VTSS_X_HSIO_MCB_SERDES1G_CFG_MCB_SERDES1G_ADDR_CFG_SERDES1G_WR_ONE_SHOT(dat));
}
#endif // FERRET_F11 || FERRET_F10P || FERRET_F4P

/**
 * Control initialization sequence and control round-robin loop.
 */
#if defined(FERRET_MAIN_ENABLE)
extern void ferret_main(void);
#endif // FERRET_MAIN_ENABLE

void main (void)
{
    char hw_ver;

    /*
     * Important !!!
     *
     * Don't access the switch register via APIs H2_READ()/H2_WRITE()/H2_WRITE_MASKED()
     * before the interrupt mechanism is ready
     */

#if defined(VTSS_ARCH_OCELOT) && !defined(BRINGUP) && defined(FERRET_MAIN_ENABLE)
    ferret_main();
#endif

    /* Determine hardware version */
    // Add something here to determin rev.A/rev. B board BZ#20911
    hw_ver = get_hw_version();

#if defined(FERRET_F11) || defined(FERRET_F10P) || defined(FERRET_F4P)
    // Before the alternate function enabling,
    // read GPIO_14 value to decide if SERDES1G_4 S/W patch need to execute or not
    if (hw_ver == '1') {
        _ferret_serdes1g_4_patch();
    }
#endif // FERRET_F11 || FERRET_F10P || FERRET_F4P

#if defined(FERRET_F11) || defined(FERRET_F10P)
    /* We need to enable alternate functions before uart_init()
     * (needs to refer to hardware schematic)
     * Chose GPIO Overlaid Funciton 1: 0,1,4,6,7,14,15,16
     * GPIO_0  - SG0_CLK
     * GPIO_1  - SG0_DO
     * GPIO_4  - IRQ0_IN
     * GPIO_6  - UART_RXD
     * GPIO_7  - UART_TXD
     * GPIO_14 - MIIM1_MDC
     * GPIO_15 - MIIM1_MDIO
     * GPIO_16 - TWI_SDA
     */
    h2_write(VTSS_DEVCPU_GCB_GPIO_GPIO_ALT(0), 0x01C0D3);

#elif defined(FERRET_F5) || defined(FERRET_F4P)
    // Without MIIM1 GPIO_14/GPIO_15 (for external PHY chip)
    // 31     24 23     16 15      8 7       0
    // --------- --------- --------- ---------
    //                0001.0000.0000.1101.0011
    h2_write(VTSS_DEVCPU_GCB_GPIO_GPIO_ALT(0), 0x0100D3);
#endif

    /* Set up timer 0 for system tick */
    timer_1_init();

    /* Initialize drivers */
#if !defined(NO_DEBUG_IF)
    uart_init();
#endif

    /* Setup interrupts */
    ext_interrupt_init();

    /*
     * Enable global interrupt
     *
     * The interrupt mechanism is ready now.
     * Use APIs H2_READ()/H2_WRITE()/H2_WRITE_MASKED() to access switch register.
     */
    EA = 1;

    /* Wait 20 msec before accessing chip and PHYs */
    delay_1(20);

#if defined(MAIN_DEBUG_ENABLE)
    print_cr_lf();
    print_cr_lf();
    sysutil_show_chip_id();
    sysutil_show_sw_ver();
    sysutil_show_compile_date();
    sysutil_show_hw_ver();
#endif /* MAIN_DEBUG_ENABLE */

#if defined(BRINGUP)
    while(1);

#else

    /* Initialize memory and enable the switch core */
    h2_post_reset();

    /* GPIO/SGPIO initialization */
    gpio_init();

    /* Read configuration data into RAM */
#if TRANSIT_UNMANAGED_SWUP
    flash_init();
#endif

    flash_load_config();
#ifdef VTSS_COMMENTS
#endif
    /*
     * Do some health check of chip
     */
    if (sysutil_assert_event_get()
#if H2_ID_CHECK
        || !h2_chip_family_support()
#endif // H2_ID_CHECK
        ) {
        sysutil_hang();
    }

    /*
     * Initialize hardware: PHY reset
     */
    phy_hw_init();  // Initializes the internal PHY by releasing resets.

    // Initialize hardware L2 port features
    h2_init_ports();

    /*
     * Initialize and check PHYs, hang the system if chek not passed.
     *
     * @note    Phy Init must come after the h2_init_port, because the port
     *          clocks must be enabled.
     */
    if (phy_tsk_init()) {
        sysutil_hang();
    }

#ifdef VTSS_COMMENTS
#endif
    /* Turn on green front LED when power up done */
#if FRONT_LED_PRESENT
    led_init();
#endif

#if defined(VTSS_ARCH_OCELOT)
#if TRANSIT_LLDP || TRANSIT_LACP
    h2_rx_init();
#endif //#if TRANSIT_LLDP || TRANSIT_LACP    
#elif defined(VTSS_ARCH_LUTON26)
#if TRANSIT_LLDP || LOOPBACK_TEST
    h2_rx_init();
#endif
#endif

#if TRANSIT_LLDP
    lldp_init();
#endif /* TRANSIT_LLDP */

#if TRANSIT_LACP || TRANSIT_RSTP
    vtss_os_init();
#endif /* TRANSIT_LACP || TRANSIT_RSTP */


#if TRANSIT_LAG
    h2_aggr_init();
#endif /* TRANSIT_LAG */    

#if TRANSIT_E2ETC
#if TRANSIT_TCAM_IS2
    h2_tcam_init();
#endif
    h2_e2etc_init();
#endif

#if (WATCHDOG_PRESENT && WATCHDOG_ENABLE)
    enable_watchdog();
#endif

#if TRANSIT_VERIPHY
    phy_tsk(); // Activate state machine. We have seen that dual media doesn't pass VeriPhy if state machine is not activated.
    phy_veriphy_all();
#endif

#if TRANSIT_ACTIPHY
    phy_actiphy_set(TRUE);
#endif

#if TRANSIT_EEE
    eee_mgmt_int();
#endif

#if TRANSIT_FAN_CONTROL
    fan_init();
#endif

#if USE_HW_TWI
    /* Initial I2C */
    i2c_init();
#endif

#if !defined(NO_DEBUG_IF)
    print_cr_lf();
    println_str("Enter ? to get the CLI help command");
    CLI_PROMPT();
#endif

    /************************************************************************
     *
     *
     * Loop forever
     *
     *
     *
     ************************************************************************/

    while (TRUE) {

#if defined(BRINGUP) // For board bringup stage
{
    #if !defined(NO_DEBUG_IF) && defined(HW_TEST_TIMTER)
            debug_test_timer();
    #endif /* HW_TEST_TIMTER */

    #if !defined(NO_DEBUG_IF) && defined(HW_TEST_UART)
            debug_test_uart();
    #endif /* HW_TEST_UART */

    cli_tsk();
}

#else // !defined(BRINGUP)
{
        /* For profiling/debug purposes */
        MAIN_LOOP_ENTER();

        /* If fatal error happens, reboot */
        if (sysutil_assert_event_get())
            sysutil_reboot();

        /* Handle any commands received on RS232 interface */
#if !defined(NO_DEBUG_IF)
        TASK(TASK_ID_CLI, cli_tsk());
#endif

        /* Suspended. Skip all tasks besides CLI */
        if (sysutil_get_suspend())
            continue;

#if defined(VTSS_ARCH_OCELOT)
#if TRANSIT_LLDP || TRANSIT_LACP
        TASK(TASK_ID_RX_PACKET, rx_packet_tsk());
#endif //#if TRANSIT_LLDP || TRANSIT_LACP        
#elif defined(VTSS_ARCH_LUTON26)
#if TRANSIT_LLDP || LOOPBACK_TEST
        /* Handle any packets received */
        TASK(TASK_ID_RX_PACKET, rx_packet_tsk());
#endif
#endif
#if TRANSIT_LACP
        TASK(TASK_ID_LACP, vtss_lacp_more_work());
#endif /* TRANSIT_LACP */

        // Handle 10ms timeout event
        handle_timeout_event_10ms();

        // Handle 100ms timeout event
        handle_timeout_event_100ms();

        // Handle 1 second timeout event
        handle_timeout_event_1sec();

        /* For profiling/debug purposes */
        MAIN_LOOP_EXIT();

        /*
         * Sleep until next interrupt
         * Make sure to keep it as the last command
         */
        PCON = 0x1;
}
#endif /* BRINGUP */
    } // End of while (TRUE)
    
    
#endif // BRINGUP
}

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/

