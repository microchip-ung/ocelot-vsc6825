//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#include "common.h"     /* Always include common.h at the first place of user-defined herder files */
#include "vtss_api_base_regs.h"
#include "h2gpios.h"
#include "h2io.h"
#include "hwport.h"
#include "phytsk.h"
#include "main.h"
#include "phydrv.h"
#include "h2stats.h"
#include "ledtsk.h"
#include "string.h"

#if defined(LEDTSK_DEBUG_ENABLE)
#include "print.h"
#endif /* LEDTSK_DEBUG_ENABLE */


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
typedef enum {
    VTSS_LED_TOWER_MODE_LINK_SPEED,     /**< Green: 1G link/activity; Orange: 10/100 link/activity */
    VTSS_LED_TOWER_MODE_DUPLEX,         /**< Green: FDX; Orange: HDX + Collisions */
    VTSS_LED_TOWER_MODE_LINK_STATUS,    /**< Green: Link/activity; Orange: port disabled/errors */
    VTSS_LED_TOWER_MODE_POWER_SAVE,     /**< Disabled to save power */
    VTSS_LED_TOWER_MODE_END
} vtss_led_tower_mode_t;

/*****************************************************************************
 *
 *
 * Prototypes for local functions
 *
 *
 *
 ****************************************************************************/
static void _led_update_port_by_tower_mode();

/*****************************************************************************
 *
 *
 * Local data
 *
 *
 *
 ****************************************************************************/

#if FRONT_LED_PRESENT

static uchar led_mode_timer;
static vtss_led_tower_mode_t curr_tower_mode = VTSS_LED_TOWER_MODE_LINK_SPEED; /* Default twoer mode */
static BOOL is_refresh_led = FALSE;
static char led_port_event[NO_OF_BOARD_PORTS + 1][VTSS_LED_EVENT_END];
static char curr_led_port_event[NO_OF_BOARD_PORTS + 1];

#if UNMANAGED_PORT_STATISTICS_IF && defined(HW_LED_TOWER_PRESENT)
static ulong col_cnt[NO_OF_BOARD_PORTS + 1]; /* plus 1 for system LED */
#endif // UNMANAGED_PORT_STATISTICS_IF && HW_LED_TOWER_PRESENT


// The port_LED SGPIO mapping is different with system_LED mapping on some specific board. i.e. Ferret reference boards
// So we have 'is_front_port' parameter to distinguish it.
static void _led_mode_set(uchar sgpio_no, vtss_led_mode_type_t mode)
{
    vtss_sgpio_mode_t sgpio_bit_0_mode, sgpio_bit_1_mode;
    bool valid_mode = TRUE;

    switch(mode) {
#if defined(FERRET_F11) || defined(FERRET_F10P) || defined(FERRET_F5) || defined(FERRET_F4P)
    // Light system LED (needs to refer to hardware schematic)
    // bit[1:0] 01 => green, 10 => red, 11 => off
    case VTSS_LED_MODE_OFF:
        sgpio_bit_1_mode = VTSS_SGPIO_MODE_ON;
        sgpio_bit_0_mode = VTSS_SGPIO_MODE_ON;
        break;
    case VTSS_LED_MODE_ON_RED:
        sgpio_bit_1_mode = VTSS_SGPIO_MODE_ON;
        sgpio_bit_0_mode = VTSS_SGPIO_MODE_OFF;
        break;
    case VTSS_LED_MODE_BLINK_RED:
        sgpio_bit_1_mode = VTSS_SGPIO_MODE_ON;
        sgpio_bit_0_mode = VTSS_SGPIO_MODE_BL_0;
        break;
    case VTSS_LED_MODE_ON_GREEN:
        sgpio_bit_1_mode = VTSS_SGPIO_MODE_OFF;
        sgpio_bit_0_mode = VTSS_SGPIO_MODE_ON;
        break;
    case VTSS_LED_MODE_BLINK_GREEN:
        sgpio_bit_1_mode = VTSS_SGPIO_MODE_BL_0;
        sgpio_bit_0_mode = VTSS_SGPIO_MODE_ON;
        break;
    case VTSS_LED_MODE_LINK_ACTIVITY_GREEN:
        sgpio_bit_1_mode = VTSS_SGPIO_MODE_LACT_0;
        sgpio_bit_0_mode = VTSS_SGPIO_MODE_ON;
        break;
    case VTSS_LED_MODE_ON_YELLOW:
        sgpio_bit_1_mode = VTSS_SGPIO_MODE_OFF;
        sgpio_bit_0_mode = VTSS_SGPIO_MODE_OFF;
        break;
    case VTSS_LED_MODE_BLINK_YELLOW:
        sgpio_bit_1_mode = sgpio_bit_0_mode = VTSS_SGPIO_MODE_BL_0;
        break;
    case VTSS_LED_MODE_LINK_ACTIVITY_YELLOW:
        sgpio_bit_1_mode = sgpio_bit_0_mode = VTSS_SGPIO_MODE_LACT_0;
        break;

#elif defined(VTSS_ARCH_LUTON26)
    case VTSS_LED_MODE_OFF:
        sgpio_bit_1_mode = sgpio_bit_0_mode = VTSS_SGPIO_MODE_OFF;
        break;
    case VTSS_LED_MODE_ON_RED:
        sgpio_bit_1_mode = VTSS_SGPIO_MODE_OFF;
        sgpio_bit_0_mode = VTSS_SGPIO_MODE_ON;
        break;
    case VTSS_LED_MODE_BLINK_RED:
        sgpio_bit_1_mode = VTSS_SGPIO_MODE_OFF;
        sgpio_bit_0_mode = VTSS_SGPIO_MODE_BL_1;
        break;
    case VTSS_LED_MODE_ON_GREEN:
        sgpio_bit_1_mode = VTSS_SGPIO_MODE_ON;
        sgpio_bit_0_mode = VTSS_SGPIO_MODE_OFF;
        break;
    case VTSS_LED_MODE_BLINK_GREEN:
        sgpio_bit_1_mode = VTSS_SGPIO_MODE_BL_1;
        sgpio_bit_0_mode = VTSS_SGPIO_MODE_OFF;
        break;
    case VTSS_LED_MODE_LINK_ACTIVITY_GREEN:
        sgpio_bit_1_mode = VTSS_SGPIO_MODE_LACT_1;
        sgpio_bit_0_mode = VTSS_SGPIO_MODE_OFF;
        break;
    case VTSS_LED_MODE_ON_YELLOW:
        sgpio_bit_1_mode = sgpio_bit_0_mode = VTSS_SGPIO_MODE_ON;
        break;
    case VTSS_LED_MODE_BLINK_YELLOW:
        sgpio_bit_1_mode = sgpio_bit_0_mode = VTSS_SGPIO_MODE_BL_1;
        break;
    case VTSS_LED_MODE_LINK_ACTIVITY_YELLOW:
        sgpio_bit_1_mode = sgpio_bit_0_mode = VTSS_SGPIO_MODE_LACT_1;
        break;
#endif

    default :
#if defined(LEDTSK_DEBUG_ENABLE)
        print_str("%% Error: Wrong parameter when calling _led_mode_set(FALSE, ), mode=0x");
        print_hex_b(mode);
        print_cr_lf();
#endif /* LEDTSK_DEBUG_ENABLE */
        valid_mode = FALSE;
        break;
    }

    if (valid_mode) {
        h2_sgpio_write(sgpio_no, VTSS_SGPIO_BIT_1, sgpio_bit_1_mode);
        h2_sgpio_write(sgpio_no, VTSS_SGPIO_BIT_0, sgpio_bit_0_mode);
    }
}

#if defined(HW_LED_TOWER_PRESENT)
static void _led_update_tower(uchar mode)
/* ------------------------------------------------------------------------ --
 * Purpose     : Setup mode LED mode
 * Remarks     :
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    switch (mode) {
#if defined(LUTON26_L25)
        /*
         *  LED tower
         *  (top)       o  mode A (link/speed)      sgpio port 26
         *              o  mode B (link/duplex)     sgpio port 27
         *              o  mode C (link/status)     sgpio port 28
         *  (button)    o  PWR save                 sgpio port 29
         */
    case VTSS_LED_TOWER_MODE_LINK_SPEED:
        _led_mode_set(28, VTSS_LED_MODE_OFF); /* Off */
        _led_mode_set(29, VTSS_LED_MODE_OFF); /* Off */
        _led_mode_set(27, VTSS_LED_MODE_OFF); /* Off */
        _led_mode_set(26, VTSS_LED_MODE_ON_GREEN);  /* Green */
        break;
    case VTSS_LED_TOWER_MODE_DUPLEX:
        _led_mode_set(27, VTSS_LED_MODE_OFF); /* Off */
        _led_mode_set(29, VTSS_LED_MODE_OFF); /* Off */
        _led_mode_set(26, VTSS_LED_MODE_OFF); /* Off */
        _led_mode_set(27, VTSS_LED_MODE_ON_GREEN);  /* Green */
        break;
    case VTSS_LED_TOWER_MODE_POWER_SAVE:
        _led_mode_set(27, VTSS_LED_MODE_OFF); /* Off */
        _led_mode_set(28, VTSS_LED_MODE_OFF); /* Off */
        _led_mode_set(26, VTSS_LED_MODE_OFF); /* Off */
        _led_mode_set(29, VTSS_LED_MODE_ON_GREEN);  /* Green */
        break;
    case VTSS_LED_TOWER_MODE_LINK_STATUS:
        /* Mode C not supported in the unmanaged solution */
        break;
#elif defined(LUTON26_L10)
        /*
         *  LED tower
         *  (top)       o  mode A (link/speed)      sgpio port 20
         *              o  mode B (link/duplex)     sgpio port 21
         *              o  mode C (link/status)     sgpio port 22
         *  (button)    o  PWR save                 sgpio port 23
         */
    case VTSS_LED_TOWER_MODE_LINK_SPEED:
        _led_mode_set(21, VTSS_LED_MODE_OFF); /* Off */
        _led_mode_set(22, VTSS_LED_MODE_OFF); /* Off */
        _led_mode_set(23, VTSS_LED_MODE_OFF); /* Off */
        _led_mode_set(20, VTSS_LED_MODE_ON_GREEN);  /* Green */

        break;
    case VTSS_LED_TOWER_MODE_DUPLEX:
        _led_mode_set(20, VTSS_LED_MODE_OFF); /* Off */
        _led_mode_set(22, VTSS_LED_MODE_OFF); /* Off */
        _led_mode_set(23, VTSS_LED_MODE_OFF); /* Off */
        _led_mode_set(21, VTSS_LED_MODE_ON_GREEN);  /* Green */
        break;
    case VTSS_LED_TOWER_MODE_POWER_SAVE:
        _led_mode_set(21, VTSS_LED_MODE_OFF); /* Off */
        _led_mode_set(22, VTSS_LED_MODE_OFF); /* Off */
        _led_mode_set(20, VTSS_LED_MODE_OFF); /* Off */
        _led_mode_set(23, VTSS_LED_MODE_ON_GREEN);  /* Green */
        break;
    case VTSS_LED_TOWER_MODE_LINK_STATUS:
        /* Mode C not supported in the unmanaged solution */
#else
        /*
         *  LED tower
         *  (top)       o  mode A (link/speed)      sgpio port 26
         *              o  mode B (link/duplex)     sgpio port 27
         *              o  mode C (link/status)     sgpio port 28
         *  (button)    o  PWR save                 sgpio port 29
         */
    case VTSS_LED_TOWER_MODE_LINK_SPEED:
        _led_mode_set(28, VTSS_LED_MODE_OFF); /* Off */
        _led_mode_set(29, VTSS_LED_MODE_OFF); /* Off */
        _led_mode_set(27, VTSS_LED_MODE_OFF); /* Off */
        _led_mode_set(26, VTSS_LED_MODE_ON_GREEN);  /* Green */
        break;
    case VTSS_LED_TOWER_MODE_DUPLEX:
        _led_mode_set(27, VTSS_LED_MODE_OFF); /* Off */
        _led_mode_set(29, VTSS_LED_MODE_OFF); /* Off */
        _led_mode_set(26, VTSS_LED_MODE_OFF); /* Off */
        _led_mode_set(27, VTSS_LED_MODE_ON_GREEN);  /* Green */
        break;
    case VTSS_LED_TOWER_MODE_POWER_SAVE:
        _led_mode_set(27, VTSS_LED_MODE_OFF); /* Off */
        _led_mode_set(28, VTSS_LED_MODE_OFF); /* Off */
        _led_mode_set(26, VTSS_LED_MODE_OFF); /* Off */
        _led_mode_set(29, VTSS_LED_MODE_ON_GREEN);  /* Green */
        break;
    case VTSS_LED_TOWER_MODE_LINK_STATUS:
        /* Mode C not supported in the unmanaged solution */
        break;
#endif
    default:
        break;
    }
}
#endif /* HW_LED_TOWER_PRESENT */

/* ************************************************************************ */
void led_tsk(void)
/* ------------------------------------------------------------------------ --
 * Purpose     : Detects button and setup port led accordingly
 * Remarks     :
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
#if defined(HW_LED_TOWER_PRESENT)
    /* GPIO bit 16 is for push button */
    if (h2_gpio_read(16)) {
        curr_tower_mode = (++curr_tower_mode) % VTSS_LED_TOWER_MODE_END;

        /* Mode C not supported in the unmanaged solution */
        if (curr_tower_mode == VTSS_LED_TOWER_MODE_LINK_STATUS) {
            curr_tower_mode++;
        }
        led_mode_timer = LED_MODE_DEFAULT_TIME;
    } else if (led_mode_timer == 0) {
        curr_tower_mode = VTSS_LED_TOWER_MODE_POWER_SAVE;

    }

    _led_update_tower(curr_tower_mode);
    _led_update_port_by_tower_mode();
#endif /* HW_LED_TOWER_PRESENT */

    if (is_refresh_led && curr_tower_mode != VTSS_LED_TOWER_MODE_POWER_SAVE) {
        /* Update front LEDs */
        _led_update_port_by_tower_mode();
    }

    is_refresh_led = FALSE;

    /*
    **  Fixme:
    **  1) Setup other led/sgpio outputs, such as LED tower
    **  and sgpio port 26-31 for general purpose in the function
    **  or a new function
    **  2) To implement a callback function to update port led
    **  output for link change (up/down)
    **  3) To implement polling for switch counters to detect
    **  collisions at HDX for VTSS_LED_TOWER_MODE_DUPLEX mode and detect
    **  errors for VTSS_LED_TOWER_MODE_LINK_STATUS mode.
    **  4) Possibily create API for sgpio read. (Note: to read data
    **  in you need to issue two bursts, one for LD and one for
    **  clocking in)
    */
}

#if UNMANAGED_PORT_STATISTICS_IF && defined(HW_LED_TOWER_PRESENT)
/* ************************************************************************ */
static BOOL _port_collision(uchar port_no)
/* ------------------------------------------------------------------------ --
 * Purpose     : Check if collision occured
 * Remarks     :
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    ulong collision_cnt;
    uchar i_port_no;
    i_port_no = uport2cport(port_no + 1);
    collision_cnt = h2_stats_counter_get(i_port_no, CNT_TX_COLLISIONS);

    if (col_cnt[port_no] != collision_cnt) {
        col_cnt[port_no] = collision_cnt;
        return TRUE;
    }

    return FALSE;
}
#endif // UNMANAGED_PORT_STATISTICS_IF && HW_LED_TOWER_PRESENT

/* ************************************************************************ */
static void _led_update_port_by_tower_mode(void)
/* ------------------------------------------------------------------------ --
 * Purpose     : Setup port LED mode
 * Remarks     :
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    vtss_iport_no_t iport_idx;
    vtss_cport_no_t chip_port;
    uchar link_mode;

    for (iport_idx = MIN_PORT; iport_idx < MAX_PORT; iport_idx++) {
        if (curr_led_port_event[iport_idx] != VTSS_LED_MODE_NORMAL) {
            /* at least one of error events occurs, just show the error status */
            continue;
        }

        chip_port = iport2cport(iport_idx);
        link_mode = port_link_mode_get(chip_port);

#if defined(LEDTSK_DEBUG_ENABLE)
    print_str("Calling _led_update_port_by_tower_mode(): iport_idx=0x");
    print_hex_b(iport_idx);

    print_str(", chip_port=0x");
    print_hex_b(chip_port);

    print_str(", link_mode=0x");
    print_hex_b(link_mode);
    print_str(", curr_tower_mode=0x");
    print_hex_b(curr_tower_mode);
    print_cr_lf();
#endif /* LEDTSK_DEBUG_ENABLE */

        switch (curr_tower_mode) {
#if defined(HW_LED_TOWER_PRESENT)
        case VTSS_LED_TOWER_MODE_POWER_SAVE:
            /* Force off no matter link is up or not */
            _led_mode_set(chip_port, VTSS_LED_MODE_OFF);
            break;

        case VTSS_LED_TOWER_MODE_DUPLEX:
            /* Duplex mode; Green for FDX and Yellow for HDX */
            if (link_mode == LINK_MODE_DOWN) {
                /* Link down */
                _led_mode_set(chip_port, VTSS_LED_MODE_OFF);
            } else if (link_mode & LINK_MODE_FDX_MASK) {
                /* Green: FDX */
                _led_mode_set(chip_port, VTSS_LED_MODE_ON_GREEN);
            } else {
#if UNMANAGED_PORT_STATISTICS_IF && defined(HW_LED_TOWER_PRESENT)
                if (_port_collision(iport_idx)) {
                    /* collision, blinking LED - Yellow/Blink: HDX */
                    _led_mode_set(chip_port, VTSS_LED_MODE_BLINK_YELLOW);
                } else
#endif // UNMANAGED_PORT_STATISTICS_IF && HW_LED_TOWER_PRESENT
                {
                    /* no collision, turn on LED - Yellow/On: HDX */
                    _led_mode_set(chip_port, VTSS_LED_MODE_ON_YELLOW);
                }
            }
            break;

        case VTSS_LED_TOWER_MODE_LINK_STATUS:
            /* Green for link/activity; Yellow: Port disabled */
            if (link_mode == LINK_MODE_DOWN) {
                /* Link down */
                _led_mode_set(chip_port, VTSS_LED_MODE_OFF);
            } else if (link_mode & LINK_MODE_SPEED_MASK) {
                /* Green: Link/activity */
                _led_mode_set(chip_port, VTSS_LED_MODE_LINK_ACTIVITY_GREEN);
            } else {
                /* Yellow: Port disabled */
                _led_mode_set(chip_port, VTSS_LED_MODE_ON_YELLOW);
            }
            break;

#endif /* HW_LED_TOWER_PRESENT */

        case VTSS_LED_TOWER_MODE_LINK_SPEED:
            /* Link/activity; Green for 1G and Yellow for 10/100 */
            if (link_mode == LINK_MODE_DOWN) {
                /* Link down */
                _led_mode_set(chip_port, VTSS_LED_MODE_OFF);
            } else if ((link_mode & LINK_MODE_SPEED_MASK) == LINK_MODE_SPEED_10 ||
                       (link_mode & LINK_MODE_SPEED_MASK) == LINK_MODE_SPEED_100) {
                /* Yellow: 100/10 link/activity */
                _led_mode_set(chip_port, VTSS_LED_MODE_LINK_ACTIVITY_YELLOW);
            } else {
                /* Green: 1G/2.5G link/activity */
                _led_mode_set(chip_port, VTSS_LED_MODE_LINK_ACTIVITY_GREEN);
            }
            break;

        default:
            break;
        }
    }
}

void led_refresh(void)
{
    is_refresh_led = TRUE;
}


/* ************************************************************************ */
void led_init(void)
/* ------------------------------------------------------------------------ --
 * Purpose     : SGPIO controller setup based on board
 * Remarks     :
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
#if defined(VTSS_ARCH_LUTON26)
    // Something specfic signature on Luton board ???
    phy_page_std(15);
    phy_write_masked(15, 29, 0x00f0, 0x00f0);
#endif

    /* Given an initial state/value */
    memset(curr_led_port_event, VTSS_LED_MODE_NORMAL, sizeof(curr_led_port_event));
    memset(led_port_event, 0, sizeof(led_port_event));
#if UNMANAGED_PORT_STATISTICS_IF && defined(HW_LED_TOWER_PRESENT)
    memset(col_cnt, 0, sizeof(col_cnt));
#endif // UNMANAGED_PORT_STATISTICS_IF && HW_LED_TOWER_PRESENT

    led_mode_timer = LED_MODE_DEFAULT_TIME;

    /* Light status LED green */
    led_update_system(VTSS_LED_MODE_ON_GREEN);

#if defined(HW_LED_TOWER_PRESENT)
    /* Light tower LED */
    _led_update_tower(VTSS_LED_TOWER_MODE_LINK_SPEED);
#endif /* HW_LED_TOWER_PRESENT */

    /* Light port LED */
    _led_update_port_by_tower_mode();
}

#if (TRANSIT_VERIPHY || TRANSIT_LOOPDETECT)
/* ************************************************************************ */
void led_port_event_set(vtss_iport_no_t iport, vtss_led_event_type_t event, vtss_led_mode_type_t mode)
/* ------------------------------------------------------------------------ --
 * Purpose     : Set the LED mode for error event
 * Remarks     : The status set here overcomes the normal LED display
 *               event
 *                  VTSS_LED_EVENT_PORT_LOOP,
 *                  VTSS_LED_EVENT_PHY_OVERHEAT,
 *                  VTSS_LED_EVENT_VERIPHY_ERR
 *               mode
 *                  VTSS_LED_MODE_NORMAL,    (priority low)
 *                  VTSS_LED_MODE_OFF,
 *                  VTSS_LED_MODE_ON_GREEN,
 *                  VTSS_LED_MODE_ON_YELLOW,
 *                  VTSS_LED_MODE_BLINK_GREEN,   at 10Hz
 *                  VTSS_LED_MODE_BLINK_YELLOW   at 10Hz (priority high)
 *
 *              The highest priority mode will show when any conflict
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    uchar           led_event;
    uchar           temp_led_port_event;
    vtss_cport_no_t chip_port;

    if (iport != SYS_LED_SGPIO_PORT) {
        chip_port = iport2cport(iport);
    } else {
        iport = NO_OF_BOARD_PORTS; // The last array entry is used to system LED
        chip_port = SYS_LED_SGPIO_PORT;
    }

    led_port_event[iport][event] = mode;

    /* Select the highest priority event to show */
    temp_led_port_event = led_port_event[iport][0];
    for (led_event = 1; led_event < VTSS_LED_EVENT_END; led_event++) {
    
        if (temp_led_port_event < led_port_event[iport][led_event])
            temp_led_port_event = led_port_event[iport][led_event];
    }
    curr_led_port_event[iport] =  temp_led_port_event;
#if defined(LEDTSK_DEBUG_ENABLE)    
    if (iport == NO_OF_BOARD_PORTS) {
        print_str("iport: ");
        print_dec(iport);
        print_str("temp_led_port_event: ");
        print_dec(temp_led_port_event); print_cr_lf();
    }
#endif //LEDTSK_DEBUG_ENABLE    
    /* Set port LED mode */
    if (mode == VTSS_LED_MODE_NORMAL) {
        /* Error event gone and back to normal LED display */
        led_refresh(); /* Trigger to update normal LED at the next second */
#if defined(LEDTSK_DEBUG_ENABLE)        
        if (iport == NO_OF_BOARD_PORTS) 
            print_str("VTSS_LED_MODE_NORMAL: "); 
#endif //LEDTSK_DEBUG_ENABLE              
        return;
    }

#if defined(FERRET_F11) || defined(FERRET_F10P) || defined(FERRET_F5) || defined(FERRET_F4P)
    _led_mode_set(chip_port, temp_led_port_event);
#elif defined(VTSS_ARCH_LUTON26)
    _led_mode_set(chip_port, temp_led_port_event);
#endif
}
#endif // TRANSIT_VERIPHY || TRANSIT_LOOPDETECT

void led_update_system(vtss_led_mode_type_t mode)
{
#if defined(SYS_LED_SGPIO_PORT)
    _led_mode_set(SYS_LED_SGPIO_PORT, mode);
#endif /* SYS_LED_SGPIO_PORT */
}

void led_1s_timer(void)
{
    static uchar i = 0;
    
#if (TRANSIT_VERIPHY || TRANSIT_LOOPDETECT)   
    led_port_event_set(SYS_LED_SGPIO_PORT, VTSS_LED_EVENT_LED,
                  i++ % 2 ? VTSS_LED_MODE_ON_GREEN : VTSS_LED_MODE_OFF);
#else
    led_update_system(i++ % 2 ? VTSS_LED_MODE_ON_GREEN : VTSS_LED_MODE_OFF);   
#endif //TRANSIT_VERIPHY || TRANSIT_LOOPDETECT    
   
    if (led_mode_timer != 0)
        led_mode_timer--;
}
#endif //FRONT_LED_PRESENT
