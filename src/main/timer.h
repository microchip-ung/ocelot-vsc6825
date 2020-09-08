//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#ifndef __TIMER_H__
#define __TIMER_H__

/*****************************************************************************
 *
 *
 * Defines
 *
 *
 *
 ****************************************************************************/

#define TIMER_0 	0
#define TIMER_1	 	1
#define TIMER_2		2

/* Some frequently used timer values in granularity of 10 msec */
#define MSEC_20     2
#define MSEC_30     3
#define MSEC_40     4
#define MSEC_50     5
#define MSEC_100   10
#define MSEC_400   40
#define MSEC_500   50
#define MSEC_750   75
#define MSEC_1000 100
#define MSEC_1500 150
#define MSEC_2000 200
#define MSEC_2500 250

/****************************************************************************
 *
 *
 * Typedefs and enums
 *
 *
 ****************************************************************************/

typedef unsigned long time_t;

/****************************************************************************
 *
 *
 * Structures
 *
 *
 ****************************************************************************/

struct timeb {
    time_t          time;
    unsigned short  millitm;
};

/****************************************************************************
 *
 *
 * Directly accessible data (todo: hide data from external access)
 *
 *
 ****************************************************************************/
extern data ushort tick_count;

extern bit      ms_10_timeout_flag;
extern bit      ms_100_timeout_flag;
extern bit      sec_1_timeout_flag;

/****************************************************************************
 *
 *
 * Functions
 *
 *
 ****************************************************************************/

void    timer_1_init            (void) small;
void    timer_1_interrupt       (void) small;
void    delay                   (uchar delay_in_10_msec) small;
void    delay_1                 (uchar delay_in_1_msec) small;
void    start_timer             (uchar time_in_10_msec) small;
bool    timeout                 (void) small;

#if TRANSIT_LLDP
void    time_since_boot_update  (void);
ulong   time_since_boot_ticks   (void);
#endif /* TRANSIT_LLDP */

#if TRANSIT_FTIME
void    ftime                   (struct timeb *tp);
#endif /* TRANSIT_FTIME */

#if TRANSIT_LACP
ushort tdiff(ushort tnew, ushort told);
#endif /* TRANSIT_LACP */

#endif

/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
