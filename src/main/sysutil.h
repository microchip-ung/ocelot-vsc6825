//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#ifndef __SYSUTIL_H__
#define __SYSUTIL_H__

#include "common.h"

/*
 * System assert event
 */
typedef enum {
    SYS_ASSERT_EVENT_OK = 0, // Don't change the value

    SYS_ASSERT_EVENT_OS,            // Operation system
    SYS_ASSERT_EVENT_CHIP_FAMILY,   // Wrong chip family
    SYS_ASSERT_EVENT_H2_INIT,       // Hardware L2 feature initialization failed
    SYS_ASSERT_EVENT_H2_POST_RESET, // Hardware L2 feature post reset process failed
    SYS_ASSERT_EVENT_PHY,           // PHY error

    SYS_ASSERT_EVENT_CNT
} sys_assert_event_t;

void    sysutil_reboot                          (void);
void    sysutil_hang                            (void);

void sysutil_assert_event_set(sys_assert_event_t err_id);
sys_assert_event_t sysutil_assert_event_get(void);

// Backward compatible
#define sysutil_set_fatal_error(x) sysutil_assert_event_set(x)
#define sysutil_get_fatal_error() sysutil_assert_event_get()

/**
 * Set suspend flag.
 *
 * @see sysutil_get_suspend().
 */
void    sysutil_set_suspend                     (BOOL enable);

/**
 * Return suspend flag.
 *
 * @see sysutil_set_suspend().
 */
BOOL    sysutil_get_suspend                     (void);

void sysutil_show_compile_date(void);
void sysutil_show_chip_id(void);
void sysutil_show_sw_ver(void);
void sysutil_show_hw_ver(void);

#endif /* __SYSUTIL_H__ */

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
