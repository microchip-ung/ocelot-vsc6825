//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#include "common.h"     /* Always include common.h at the first place of user-defined herder files */
#include "timer.h"
#include "misc2.h"
#include "h2io.h"
#include "vtss_api_base_regs.h"
#include "h2fan.h"
#include "phytsk.h"

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
 * Public data
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

#if TRANSIT_FAN_CONTROL
/* =================================================================
 * FAN speed control
* =================================================================*/

/*
 * Set FAN configuration according to the fan specifications
 *
 * In :  spec  - Fan specifications
 *
 */

void h2_fan_conf_set(const vtss_fan_spec_t * const spec) {

    // Set GPIO alternate functions. PWM is bit 29 and TACHO is bit 4.
    // Must be set to overlaid function 1 = 0x01
    const ulong PWM_BIT_MASK = 0x20000010;

    H2_WRITE_MASKED(VTSS_DEVCPU_GCB_GPIO_GPIO_ALT(1),          0x0, PWM_BIT_MASK);
    H2_WRITE_MASKED(VTSS_DEVCPU_GCB_GPIO_GPIO_ALT(0), PWM_BIT_MASK, PWM_BIT_MASK);

    // Set PWM frequency
    H2_WRITE_MASKED(VTSS_DEVCPU_GCB_FAN_CTRL_FAN_CFG,
           VTSS_F_DEVCPU_GCB_FAN_CTRL_PWM_FREQ_PWM_FREQ(spec->fan_pwm_freq),
           VTSS_M_DEVCPU_GCB_FAN_CTRL_PWM_FREQ_PWM_FREQ);

    // Set PWM polarity
    H2_WRITE_MASKED(VTSS_DEVCPU_GCB_FAN_CTRL_FAN_CFG,
           spec->fan_low_pol ? VTSS_M_DEVCPU_GCB_FAN_CTRL_FAN_CFG_INV_POL : 0,
           VTSS_M_DEVCPU_GCB_FAN_CTRL_FAN_CFG_INV_POL);

    // Set PWM open collector
    H2_WRITE_MASKED(VTSS_DEVCPU_GCB_FAN_CTRL_FAN_CFG,
           spec->fan_open_col ? VTSS_M_DEVCPU_GCB_FAN_CTRL_FAN_CFG_PWM_OPEN_COL_ENA : 0,
           VTSS_M_DEVCPU_GCB_FAN_CTRL_FAN_CFG_PWM_OPEN_COL_ENA);

    // Set fan speed measurement
    if (spec->type == VTSS_FAN_3_WIRE_TYPE) {
        // Enable gating for 3-wire fan types.
        H2_WRITE_MASKED(VTSS_DEVCPU_GCB_FAN_CTRL_FAN_CFG,
               VTSS_F_DEVCPU_GCB_FAN_CTRL_FAN_CFG_GATE_ENA(1),
               VTSS_M_DEVCPU_GCB_FAN_CTRL_FAN_CFG_GATE_ENA);
    } else {
        //  For 4-wire fan types we need to disable gating (2-wire types doesn't matter)
        H2_WRITE_MASKED(VTSS_DEVCPU_GCB_FAN_CTRL_FAN_CFG,
                VTSS_F_DEVCPU_GCB_FAN_CTRL_FAN_CFG_GATE_ENA(0),
                VTSS_M_DEVCPU_GCB_FAN_CTRL_FAN_CFG_GATE_ENA);
    }
}

/*
 * Set FAN cooling level
 *
 * In :  lvl - New fan cooling level, 0 = Fan is OFF, 0xFF = Fan constant ON.
 *
 */

void h2_fan_cool_lvl_set(uchar lvl) {

    // Set PWM duty cycle (fan speed)
    H2_WRITE_MASKED(VTSS_DEVCPU_GCB_FAN_CTRL_FAN_CFG,
           VTSS_F_DEVCPU_GCB_FAN_CTRL_FAN_CFG_DUTY_CYCLE(lvl),
           VTSS_M_DEVCPU_GCB_FAN_CTRL_FAN_CFG_DUTY_CYCLE);

}

#if 0
/*
 * Get FAN cooling level
 *
 * Return : Fan cooling level, 0 = Fan is OFF, 0xFF = Fan constant ON.
 *
 */

uchar h2_fan_cool_lvl_get(void) {

    ulong fan_cfg_reg;

    // Read the register
    H2_READ(VTSS_DEVCPU_GCB_FAN_CTRL_FAN_CFG, fan_cfg_reg);

    // Get PWM duty cycle
    return VTSS_X_DEVCPU_GCB_FAN_CFG_FAN_CFG_DUTY_CYCLE(fan_cfg_reg);
}
#endif

/*
 * Get FAN rotation count.
 *
 * In : fan_spec - Pointer to fan specification, e.g. type of fan.
 *
 * In/Out - rotation_count - Pointer to where to put the rotation count.
 *
 */
ulong h2_fan_rotation_get() {

    ulong fan_cnt_reg;
    // Read the register
    H2_READ(VTSS_DEVCPU_GCB_FAN_CTRL_FAN_CNT, fan_cnt_reg);

    return fan_cnt_reg;
}

/*
 * Get chip temperature.
 *
 * In/Out - chip_temp - Pointer to where to put the chip temperature.
 *
 */
void h2_fan_chip_temp_get(ushort * chip_temp) {
    *chip_temp = phy_get_sys_temp();

}
#endif

