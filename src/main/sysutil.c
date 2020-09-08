//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#include "common.h"     /* Always include common.h at the first place of user-defined herder files */
#include "h2io.h"
#include "vtss_common_os.h"
#include "vtss_api_base_regs.h"
#include "sysutil.h"
#include "timer.h"
#include "h2.h"

#if !defined(NO_DEBUG_IF)
#include "print.h"
#include "version.h"
#endif // !NO_DEBUG_IF

/*****************************************************************************
 *
 *
 * Macros
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

static BOOL g_suspend_enable = FALSE;
static sys_assert_event_t sys_assert_event = SYS_ASSERT_EVENT_OK;

/*****************************************************************************
 *
 *
 * Local Functions
 *
 *
 ****************************************************************************/


/*****************************************************************************
 *
 *
 * Public Functions
 *
 *
 ****************************************************************************/


/**
 * Reboot by forcing watchdog reset or utilizing any other reset feature.
 */
void sysutil_reboot(void)
{
    MMAP = 0xAF;
    h2_chip_reset();
    while (1) ;
}

/**
 * Hang forever with front LED blinking red.
 */
void sysutil_hang (void)
{
    u8 sys_assert_event_t = sysutil_assert_event_get();

    while (1) {
        /* Hang function */
        delay_1(1000);  // 1 second

#if !defined(NO_DEBUG_IF)
        print_str("%% System hanging (0x");
        print_hex_b(sys_assert_event_t);

        // Dump S/W version
        sysutil_show_sw_ver();

        // Dump Image build time
        sysutil_show_compile_date();
#endif  // !NO_DEBUG_IF
    }
}

/**
 * Indicate error and initiate error recovery.
 */
void sysutil_assert_event_set(sys_assert_event_t err_id)
{
    /* Error code handler */
    sys_assert_event = err_id;
}

sys_assert_event_t sysutil_assert_event_get(void)
{
    return sys_assert_event;
}

void sysutil_set_suspend(BOOL enable)
{
    g_suspend_enable = enable;
}

BOOL sysutil_get_suspend(void)
{
    return g_suspend_enable;
}

void sysutil_show_compile_date(void)
{
    print_str("Build Date      : ");
    println_str(SW_COMPILE_DATE);
}

void sysutil_show_sw_ver(void)
{
    // S/W version
    print_str("Software Version: ");
    println_str(SW_VERSION);

    // Code revision
    print_str("Code Revision   : ");
    println_str(CODE_REVISION);
}

void sysutil_show_hw_ver(void)
{
    char hw_ver;

    print_str("HW Revision     : ");
    hw_ver = get_hw_version();
    print_ch(hw_ver);
    print_cr_lf();
}

void sysutil_show_chip_id(void)
{
    u32 chip_id;
    u8  rev;

    H2_READ(VTSS_DEVCPU_GCB_CHIP_REGS_CHIP_ID, chip_id);
    rev = 'A'+ (char)VTSS_X_DEVCPU_GCB_CHIP_REGS_CHIP_ID_REV_ID(chip_id);

    // Chip Family
    print_str("Chip Family     : ");
	if ((VTSS_X_DEVCPU_GCB_CHIP_REGS_CHIP_ID_PART_ID(chip_id)&0x7510) == 0x7510) {
        print_hex_w(VTSS_X_DEVCPU_GCB_CHIP_REGS_CHIP_ID_PART_ID(chip_id));
        // Chip Revision
        print_str(" Rev.");
        print_ch(rev);
        print_cr_lf();
	}
	else {
        print_str(" Unknown.");
        print_cr_lf();
    }
}

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
