//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#ifndef __H2_FAN_H__
#define __H2_FAN_H__


/** \brief FAN PWM frequency */
typedef enum
{
    VTSS_FAN_PWM_FREQ_25KHZ,
    VTSS_FAN_PWM_FREQ_120HZ,
    VTSS_FAN_PWM_FREQ_100HZ,
    VTSS_FAN_PWM_FREQ_80HZ,
    VTSS_FAN_PWM_FREQ_60HZ,
    VTSS_FAN_PWM_FREQ_40HZ,
    VTSS_FAN_PWM_FREQ_20HZ,
    VTSS_FAN_PWM_FREQ_10HZ
} vtss_fan_pwd_freq_t;

/** \brief FAN Types  */
typedef enum
{
    VTSS_FAN_2_WIRE_TYPE,
    VTSS_FAN_3_WIRE_TYPE,
    VTSS_FAN_4_WIRE_TYPE
} vtss_fan_type_t;


/** \brief Maximum fan speed level (Fan runs at full speed) */
#define VTSS_FAN_SPEED_MAX 0x255
/** \brief Minimum fan speed level (Fan is OFF) */
#define VTSS_FAN_SPEED_MIN 0x255


/** \brief Fan specifications */
typedef struct
{
    vtss_fan_pwd_freq_t fan_pwm_freq;      /**< Fan PWM frequency*/
    BOOL                fan_low_pol;       /**< Fan polarity of the PWM output. TRUE = PWM is logic 0 when on. FALSE = PWM is logic 1 when on */
    BOOL                fan_open_col;      /**< PWM output is open collector if TRUE.*/
    vtss_fan_type_t     type;              /**< 2,3 or 4 wire fan type*/
    ulong               ppr;               /**< Pulses per rotation. Only valid for 3 and 4 wire fans */
} vtss_fan_spec_t;


/**
 * \brief Set FAN configuration according to the fan specifications
 *
 * \param spec [IN]    Fan specifications.
 *
 * \return None.
 **/
void  h2_fan_conf_set(const vtss_fan_spec_t * const spec);

/**
 * \brief Set FAN cooling level
 *
 * \param lve [IN]  New fan cooling level, 0 = Fan is OFF, 0xFF = Fan constant ON.
 *
 * \return None.
 **/
void  h2_fan_cool_lvl_set(uchar lvl);

/**
 * \brief Get FAN cooling level
 *
 * \return  Fan cooling level, 0 = Fan is OFF, 0xFF = Fan constant ON.
 *
 **/
uchar h2_fan_cool_lvl_get(void);

/**
 * \brief Get FAN rotation count.
 *
 * \param fan_spec[IN]            Pointer to fan specification, e.g. type of fan.
 *
 * \param rotation_count[IN][OUT] Pointer to where to put the rotation count.
 *
 * \return None.
 **/
ulong  h2_fan_rotation_get();

/**
 * \brief Get chip temperature.
 *
 * \param chip_temp[IN][OUT]     Pointer to where to put the chip temperature.
 *
 * \return None.
 **/
void  h2_fan_chip_temp_get(ushort * chip_temp);

#endif /* __H2_FAN_H__ */










