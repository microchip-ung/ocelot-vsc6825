//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#include "common.h"     /* Always include common.h at the first place of user-defined herder files */
#include "vtss_api_base_regs.h"
#include "h2io.h"
#include "timer.h"
#include "uartdrv.h"
#include "misc2.h"

/*****************************************************************************
 *
 *
 * Public data
 *
 *
 *
 ****************************************************************************/


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

/* ************************************************************************ */
void ext_interrupt_init (void) small
/* ------------------------------------------------------------------------ --
 * Purpose     : Enable external interrupts. Ext0 with low priority
 *               and Ext1 with high priority
 * Remarks     :
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
#if defined(VTSS_ARCH_LUTON26)
    /* Enable interrupt output to 8051 */
    H2_WRITE_MASKED(VTSS_ICPU_CFG_INTR_ICPU_IRQ1_ENA,
    VTSS_F_ICPU_CFG_INTR_ICPU_IRQ1_ENA_ICPU_IRQ1_ENA,
    VTSS_F_ICPU_CFG_INTR_ICPU_IRQ1_ENA_ICPU_IRQ1_ENA);

    H2_WRITE_MASKED(VTSS_ICPU_CFG_INTR_ICPU_IRQ0_ENA,
    VTSS_F_ICPU_CFG_INTR_ICPU_IRQ0_ENA_ICPU_IRQ0_ENA,
    VTSS_F_ICPU_CFG_INTR_ICPU_IRQ0_ENA_ICPU_IRQ0_ENA);
#endif // VTSS_ARCH_LUTON26

    /* Enable 8051 interrupt */
    PX1 = 1;  /* Set high priority for ext 1 */
    EX1 = 1;  /* Enable ext 1 interrupt */

    PX0 = 0;  /* Set low priority for ext 0 */
    EX0 = 1;  /* Enable ext 0 interrupt */
}

/* ************************************************************************ */
void ext_1_interrupt (void) small interrupt 2 using 2
/* ------------------------------------------------------------------------ --
 * Purpose     : Handle UART interrupts with high priority
 * Remarks     : ISR could only call H2_READ, H2_WRITE, H2_WRITE_MASKED, no
 *               other function is shared with main
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
#if defined(VTSS_ARCH_OCELOT)
#define INT_UART_INDEX   6

    ulong ident;

    H2_READ(VTSS_ICPU_CFG_INTR_DST_INTR_IDENT(1), ident);
#ifndef NO_DEBUG_IF
    if(test_bit_32(INT_UART_INDEX, &ident)) {
        // UART interrupt
        uart_interrupt();
        H2_WRITE_MASKED(VTSS_ICPU_CFG_INTR_INTR_STICKY,
                        bit_mask_32(INT_UART_INDEX),
                        bit_mask_32(INT_UART_INDEX));
    }
#endif // NO_DEBUG_IF

#elif defined(VTSS_ARCH_LUTON26)
    ulong ident;
    H2_READ(VTSS_ICPU_CFG_INTR_ICPU_IRQ1_IDENT, ident);
#ifndef NO_DEBUG_IF
    if(test_bit_32(6, &ident)) {
        // UART interrupt
        uart_interrupt();
        H2_WRITE(VTSS_ICPU_CFG_INTR_INTR, VTSS_F_ICPU_CFG_INTR_INTR_UART_INTR);
    }
#endif // NO_DEBUG_IF
#endif // VTSS_ARCH_OCELOT
}

/* ************************************************************************ */
void ext_0_interrupt (void) small interrupt 0 using 1
/* ------------------------------------------------------------------------ --
 * Purpose     : Handle timer and exteral interrupts with low priority
 * Remarks     : ISR could only call H2_READ, H2_WRITE, H2_WRITE_MASKED, no
 *               other function is shared with main
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
#if defined(VTSS_ARCH_OCELOT)
#define INT_TIMER_1_INDEX   4

    ulong ident;
    H2_READ(VTSS_ICPU_CFG_INTR_DST_INTR_IDENT(0), ident);

    if(test_bit_32(INT_TIMER_1_INDEX, &ident)) {
        // Timer 1 interrupt
        timer_1_interrupt();
        H2_WRITE_MASKED(VTSS_ICPU_CFG_INTR_INTR_STICKY,
                         bit_mask_32(INT_TIMER_1_INDEX),
                         bit_mask_32(INT_TIMER_1_INDEX));
    }

#elif defined(VTSS_ARCH_LUTON26)
    ulong ident;
    H2_READ(VTSS_ICPU_CFG_INTR_ICPU_IRQ0_IDENT, ident);
    if(test_bit_32(8, &ident)) {
        // Timer 1 interrupt
        timer_1_interrupt();
        H2_WRITE(VTSS_ICPU_CFG_INTR_INTR, VTSS_F_ICPU_CFG_INTR_INTR_TIMER1_INTR);
    }
#endif // VTSS_ARCH_OCELOT
}

