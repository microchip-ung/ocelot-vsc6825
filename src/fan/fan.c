//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#if 0
/*lint -esym(459,status_flag) */
#include "common.h"     /* Always include common.h at the first place of user-defined herder files */
#include "vtss_common_os.h"

#if TRANSIT_FAN_CONTROL

#include "h2fan.h"

#include "fan.h"
#include "fan_api.h"
#include "fan_custom_api.h"

#include "spiflash.h"
#include "timer.h"

//************************************************
// Global Variables
//************************************************
static fan_conf_t   fan_conf; // Current confiugration for this switch.

// 0 = fan off, 255 = Fan at full speed
static uchar fan_speed_lvl = 0; // The fan speed level ( PWM duty cycle)


//************************************************
// Temperature control
//************************************************

// Function that returns this switch's status
//
// Out : status - Pointer to where to put chip status
void fan_get_local_status(fan_local_status_t *status)
{
    ushort rotation_count = 0;
    vtss_fan_spec_t fan_spec;
    ushort temp;

    h2_fan_chip_temp_get(&temp);

    status->chip_temp = temp;
    status->fan_speed_setting_pct = fan_speed_lvl * 100 / 255;


    // Setup the kind of FAN that is used.

    // To make lint happy - In this case it is OK to have a constant
    // if (true) in the case where FAN_CUSTOM_TYPE = VTSS_FAN_4_WIRE_TYPE
    /*lint --e{550} --e{506} */
    if (FAN_CUSTOM_TYPE == VTSS_FAN_4_WIRE_TYPE) {
        fan_spec.fan_pwm_freq = VTSS_FAN_PWM_FREQ_25KHZ; // 4 wire fans supports high pwm frequency
    } else {
        fan_spec.fan_pwm_freq = VTSS_FAN_PWM_FREQ_120HZ;
    }

    fan_spec.fan_low_pol  = FAN_CUSTOM_POL;    // FAN PWM polarity.
    fan_spec.fan_open_col = FAN_CUSTOM_OC;    // Open collector
    fan_spec.type         = FAN_CUSTOM_TYPE;
    fan_spec.ppr          = FAN_CUSTOM_PPR;

    status->fan_speed = h2_fan_rotation_get();
}



// See Section 4 in AN0xxxx
void fan_control(void)
{


    //Because some result from the calculations below will be lower the zero, and we don't
    //to use floating point operations, we multiply with the resolution constant.
    const ushort resolution = 1000;
    //
    int fan_speed_lvl_pct; // The fan speed in percent. ( Negative number = fan stopped )
    const uchar fan_level_steps_pct = 10;  // How much the fan shall change for every adjustment.

    ulong delta_t; // delta_t is described in AN0xxx section 4.

    // Get the chip temperature
    fan_local_status_t status;
    fan_get_local_status(&status); // Get chip temperature



    // Figure 4 in AN0xxx shows a state machine, but instead of a state machine I have
    // use the following which does the same. In this way we don't have to have fixed
    // number of states (Each fan speed level corresponds to a state).


    delta_t = (fan_conf.t_max - fan_conf.t_on) * resolution *  fan_level_steps_pct / 100;

    // Calculate the fan speed in percent
    if (delta_t == 0) {
        // avoid divide by zero
        fan_speed_lvl_pct = 0;
    } else {
        fan_speed_lvl_pct = (status.chip_temp - fan_conf.t_on) * resolution * fan_level_steps_pct / delta_t ;
    }

    // Make sure that fan that the fan speed doesn't get below the PWM need to keep the fan running
    if (fan_speed_lvl_pct < FAN_CUSTOM_MIN_PWM_PCT && fan_speed_lvl_pct > 0) {
        fan_speed_lvl_pct = FAN_CUSTOM_MIN_PWM_PCT;
    }

    if (status.chip_temp > fan_conf.t_on) {
        if (fan_speed_lvl_pct > 100) {
            fan_speed_lvl = 255;
        } else {
            fan_speed_lvl = 255 * fan_speed_lvl_pct / 100;
        }
    } else {
        fan_speed_lvl = 0;
    }

    if (fan_speed_lvl_pct <= FAN_CUSTOM_KICK_START_LVL_PCT && fan_speed_lvl_pct != 0) {
        h2_fan_cool_lvl_set(255);
        delay(FAN_CUSTOM_KICK_START_ON_TIME);
    }

    VTSS_COMMON_TRACE(VTSS_COMMON_TRLVL_WARNING, ("A Fan speed level = %d, chip_temp =%d, delta_t = %lu, temp_fan_speed_lvl_pct = %lu",
                      fan_speed_lvl, status.chip_temp, delta_t, fan_speed_lvl_pct));

    // FJ API call to set PWM duty cycle.
    h2_fan_cool_lvl_set(fan_speed_lvl);

}

/* ************************************************************************ */
void fan_init (void)
/* ------------------------------------------------------------------------ --
 * Purpose     :
 * Remarks     :
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    load_fan_conf();
    fan_spec_init();
}

/* ************************************************************************ */
void fan_spec_init (void)
/* ------------------------------------------------------------------------ --
 * Purpose     : initialising the fan specifications.
 * Remarks     :
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    vtss_fan_spec_t fan_spec;

    // To make lint happy - In this case it is OK to have a constant
    // if (true) in the case where FAN_CUSTOM_TYPE = VTSS_FAN_4_WIRE_TYPE
    /*lint --e{550} --e{506} */
    if (FAN_CUSTOM_TYPE == VTSS_FAN_4_WIRE_TYPE) {
        // 4 wire fans supports high pwm frequency
        fan_spec.fan_pwm_freq = VTSS_FAN_PWM_FREQ_25KHZ;
    } else {
        fan_spec.fan_pwm_freq = VTSS_FAN_PWM_FREQ_120HZ;
    }

    // FAN PWM polarity.
    fan_spec.fan_low_pol = FAN_CUSTOM_POL;


    // Open collector
    fan_spec.fan_open_col = FAN_CUSTOM_OC;


    fan_spec.type = FAN_CUSTOM_TYPE;
    fan_spec.ppr  = FAN_CUSTOM_PPR;

    // FJ - DO API CALL
    h2_fan_conf_set(&fan_spec);

}

#if UNMANAGED_FAN_DEBUG_IF
uchar read_fan_conf_t_max (void)
{
    return fan_conf.t_max;
}

uchar read_fan_conf_t_on (void)
{
    return fan_conf.t_on;
}
#endif

void write_fan_conf_t_max (uchar t_max)
{
    fan_conf.t_max = t_max;
}

void write_fan_conf_t_on (uchar t_on)
{
    fan_conf.t_on = t_on;
}



void load_fan_conf (void)
{
    write_fan_conf_t_max (75);
    write_fan_conf_t_on  (50);
}

#endif
#endif //#if 0
