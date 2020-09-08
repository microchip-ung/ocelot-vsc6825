//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#include "common.h"     /* Always include common.h at the first place of user-defined herder files */
#include "vtss_api_base_regs.h"
#include "h2io.h"
#include "timer.h"
#if defined(VTSS_ARCH_OCELOT)
#include "misc2.h"
#endif // VTSS_ARCH_OCELOT

#pragma NOAREGS

/*****************************************************************************
 *
 *
 * Public data
 *
 *
 *
 ****************************************************************************/

data ushort tick_count = 0;
bit     ms_10_timeout_flag  = 0;
bit     ms_100_timeout_flag = 0;
bit     sec_1_timeout_flag  = 0;

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


/*****************************************************************************
 *
 *
 * Local data
 *
 *
 *
 ****************************************************************************/

static uchar                data timer_count        = 0;

static bit                  sw_timer_active_flag    = FALSE;
static bit                  ms_1_timeout_flag       = FALSE;

static ulong xdata          time_since_boot_t       = 0;

#if TRANSIT_FTIME
static struct timeb xdata   t_now;
#endif /* TRANSIT_FTIME */

/*****************************************************************************
 *
 *
 * Local functions
 *
 *
 *
 ****************************************************************************/

#if TRANSIT_FTIME
static void _time_carry()
{
    struct timeb t;

    t = t_now;

    t.millitm++;

    if (t.millitm >= 1000) {
        t.time    ++;
        t.millitm -= 1000;
    }

    t_now = t;
}
#endif /* TRANSIT_FTIME */

/*****************************************************************************
 *
 *
 * Public functions
 *
 *
 *
 ****************************************************************************/

#if defined(VTSS_ARCH_OCELOT)
#define INT_TIMER_1_INDEX   4
#endif // VTSS_ARCH_OCELOT

/**
 * Set timer 1 to generate interrupt every 1 msec.
 */
void timer_1_init (void) small
{
    /*
     * Important !!!
     *
     * Don't access the switch register via APIs H2_READ()/H2_WRITE()/H2_WRITE_MASKED()
     * before the interrupt mechanism is ready
     */

    /* Wrap out every 1ms */
    h2_write(VTSS_ICPU_CFG_TIMERS_TIMER_RELOAD_VALUE(TIMER_1), 0x9);

#if defined(VTSS_ARCH_OCELOT)

    /* Output to iCPU_IRQ0 */
    h2_write_masked(VTSS_ICPU_CFG_INTR_DST_INTR_MAP(0),
                    bit_mask_32(INT_TIMER_1_INDEX),
                    bit_mask_32(INT_TIMER_1_INDEX));

    /* Enable interrupt source */
    h2_write_masked(VTSS_ICPU_CFG_INTR_INTR_ENA_SET,
                    bit_mask_32(INT_TIMER_1_INDEX),
                    bit_mask_32(INT_TIMER_1_INDEX));
    /* Enable and force reload */
    h2_write(VTSS_ICPU_CFG_TIMERS_TIMER_CTRL(TIMER_1),
             VTSS_F_ICPU_CFG_TIMERS_TIMER_CTRL_TIMER_ENA(1) |
             VTSS_F_ICPU_CFG_TIMERS_TIMER_CTRL_FORCE_RELOAD(1));

#elif defined(VTSS_ARCH_LUTON26)
    /* Output to iCPU_IRQ0 */
    h2_write_masked(VTSS_ICPU_CFG_INTR_TIMER1_INTR_CFG,
                    VTSS_F_ICPU_CFG_INTR_TIMER1_INTR_CFG_TIMER1_INTR_SEL(0),
                    VTSS_M_ICPU_CFG_INTR_TIMER1_INTR_CFG_TIMER1_INTR_SEL);

    /* Enable interrupt source */
    h2_write_masked(VTSS_ICPU_CFG_INTR_INTR_ENA,
                    VTSS_F_ICPU_CFG_INTR_INTR_ENA_TIMER1_INTR_ENA,
                    VTSS_F_ICPU_CFG_INTR_INTR_ENA_TIMER1_INTR_ENA);

    /* Enable and force reload */
    h2_write(VTSS_ICPU_CFG_TIMERS_TIMER_CTRL(TIMER_1),
             VTSS_F_ICPU_CFG_TIMERS_TIMER_CTRL_TIMER_ENA |
             VTSS_F_ICPU_CFG_TIMERS_TIMER_CTRL_FORCE_RELOAD);
#endif // VTSS_ARCH_OCELOT
}


/**
 * Timer interrupt to be activated every 1 msec. Tick SW timers and set flags
 * accordingly.
 */
void timer_1_interrupt (void) small
{
    static uchar data ms_1_count  = 10;
    static uchar data ms_10_count = 10;
    static uchar data ms_100_count = 10;

#if TRANSIT_FTIME
    _time_carry();
#endif /* TRANSIT_FTIME */

    tick_count++;
    ms_1_timeout_flag = TRUE;

    if (--ms_1_count == 0) {
        ms_1_count = 10;

        /* Request 10 msec jobs to be done */
        ms_10_timeout_flag = 1;

        /* If 1 sec elapsed, request 1 sec jobs to be done */
        if (--ms_10_count == 0) {
            ms_10_count = 10;

            /* If 100 ms elapsed, request 100 ms jobs to be done */
            ms_100_timeout_flag = TRUE;

            if (--ms_100_count == 0) {
                ms_100_count = 10;
                sec_1_timeout_flag = TRUE;
            }
        }

        /* If the delay/timeout timer is active, tick it */
        if (sw_timer_active_flag) {
            if (--timer_count == 0) {
                sw_timer_active_flag = FALSE;
            }
        }
    }
}


/**
 * Make a delay by means of timer interrupt function.
 *
 * delay_in_10_msec specifies delay in granularity of 10 msec. here is no
 * synchronization with the HW timer, so the actual delay may be +/- 10 msec
 * (so specifying 1 (= 10 msec) may result in almost no delay at all).
 *
 * @note May only be called when interrupt is enabled. Can not be used
 *       simultaneously with start_timer/timeout mechanism.
 */
#if !defined(BRINGUP)
void delay (uchar delay_in_10_msec) small
{
    /* set timer value to be ticked by interrupt function */
    sw_timer_active_flag = FALSE;
    timer_count = delay_in_10_msec;
    sw_timer_active_flag = TRUE;

    /* Await that time has elapsed */
    while (sw_timer_active_flag) {
        /* do nothing but wait */
    }
}
#endif /* BRINGUP */

/**
 * Make a delay by means of timer interrupt function.
 *
 * delay_in_1_msec specifies delay in granularity of 1 msec. There is no
 * synchronization with the HW timer, so the actual delay may be +/- 1 msec
 * (so specifying 1 (= 1 msec) may result in almost no delay at all).
 *
 * @note May only be called when interrupt is enabled.
 */
void delay_1 (uchar delay_in_1_msec) small
{
    /* set timer value to be ticked by interrupt function */
    ms_1_timeout_flag = FALSE;

    /* Await that time has elapsed */
    while (delay_in_1_msec-- > 0) {
        while (!ms_1_timeout_flag) {
        }
        ms_1_timeout_flag = FALSE;
    }
}


/**
 * Start SW timer running via interrupt function. Status of timer can be read
 * via timeout function.
 *
 * time_in_10_msec specifies time in granularity of 10 msec. There is no
 * synchronization with the HW timer, so the actual time may be +/- 10 msec
 * (so specifying 1 (= 10 msec) may result in almost no time at all).
 *
 * @see timeout function.
 *
 * @example
 *                  start_timer(5);
 *                  while (!timeout()) {
 *                     ... do something
 *                  }
 */
void start_timer (uchar time_in_10_msec) small
{
    sw_timer_active_flag = FALSE;
    timer_count = time_in_10_msec;
    sw_timer_active_flag = TRUE;
}


/**
 * To check if time specified via call to start_timer function HAS expired.
 *
 * @return  Return TRUE, if time has expired, otherwiSe FALSE.
 *
 * @see start_timer
 */
bool timeout (void) small
{
    return (!sw_timer_active_flag);
}


#if TRANSIT_LLDP
void time_since_boot_update (void)
{
    time_since_boot_t += 100; /* (time_since_boot/100) seconds */
}

ulong time_since_boot_ticks (void)
{
    return time_since_boot_t;
}
#endif


#if TRANSIT_FTIME
void ftime(struct timeb *tp)
{
    *tp = t_now;
}
#endif /* TRANSIT_FTIME */

#if TRANSIT_LACP
/* ------------------------------------------------------------------------ --
 * Purpose     : To calculate the diff between two tickvalues taking wrapping
 *               into account.
 * Remarks     :
 * Restrictions:
 * See also    : timer_count
 * Example     :
 * ************************************************************************ */
ushort tdiff(ushort tnew, ushort told)
{
    if (tnew < told) /* Timer wrapped */
        return tnew + (0xFFFF - told);
    return tnew - told;
}
#endif /* TRANSIT_LACP */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
