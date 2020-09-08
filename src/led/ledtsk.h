//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#ifndef __LEDTSK_H__
#define __LEDTSK_H__

#include "common.h"     /* Always include common.h at the first place of user-defined herder files */
#include "h2gpios.h"

#define LED_MODE_DEFAULT_TIME   30

typedef enum {
    VTSS_LED_MODE_NORMAL,               /* Normal mode (control by led tower mode) */
    VTSS_LED_MODE_OFF,                  /* OFF mode */
    VTSS_LED_MODE_ON_GREEN,             /* ON (green) mode */
    VTSS_LED_MODE_BLINK_GREEN,          /* BLINK (green) mode */
    VTSS_LED_MODE_LINK_ACTIVITY_GREEN,  /* Link activity (green) mode */
    VTSS_LED_MODE_ON_YELLOW,            /* ON (yellow) mode */
    VTSS_LED_MODE_BLINK_YELLOW,         /* BLINK (yellow) mode */
    VTSS_LED_MODE_LINK_ACTIVITY_YELLOW, /* Link activity (yellow) mode */
    VTSS_LED_MODE_ON_RED,               /* ON (red) mode */
    VTSS_LED_MODE_BLINK_RED,            /* BLINK (red) mode */
    VTSS_LED_MODE_TYPE_END
} vtss_led_mode_type_t;

typedef enum {
    VTSS_LED_EVENT_LED,             /* LED itself */
    VTSS_LED_EVENT_PORT_LOOP,       /* Port loop */
    VTSS_LED_EVENT_PHY_OVERHEAT,    /* PHY over heat */
    VTSS_LED_EVENT_VERIPHY_ERR,     /* Alias: Cable diagnostic */

    /* Add new events before here */
    VTSS_LED_EVENT_END
} vtss_led_event_type_t;

#if FRONT_LED_PRESENT
extern uchar led_mode_timer;
#endif

void led_init(void);
void led_tsk(void);
void led_update_system(vtss_led_mode_type_t mode);
void led_1s_timer(void);
void led_refresh(void);

#if (TRANSIT_VERIPHY || TRANSIT_LOOPDETECT)
/* The API is used to upper layer application */
void led_port_event_set(vtss_iport_no_t iport, vtss_led_event_type_t event, vtss_led_mode_type_t mode);
#endif // TRANSIT_VERIPHY || TRANSIT_LOOPDETECT

#endif /* __LEDTSK_H__ */
