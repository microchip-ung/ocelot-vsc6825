//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#ifndef _FAN_CUSTOM_API_H_
#define _FAN_CUSTOM_API_H_

/**
 * \file fan_custom_api.h
 * \brief This file defines the data for the specific fan used for colling the chip.
 *
 */


/**
  * \brief Fan-specific type
  *
  * The fan type - 3 types of fans are supported.
  *                   a. VTSS_FAN_2_WIRE_TYPE
  *                   b. VTSS_FAN_3_WIRE_TYPE
  *                   c. VTSS_FAN_4_WIRE_TYPE
  */
#define FAN_CUSTOM_TYPE VTSS_FAN_3_WIRE_TYPE

/**
  * \brief Fan-specific PPR (Pulses Per Rotation).
  *
  * This parameter is only valid for 3 and 4 wire fan types.
  * 3 and 4 wire fans have the possibility to signal their rotation
  * speed by sending pulses to the chip. To be able to calculate
  * the RPM (Rotation Per Minute) we must know the number of pulses
  * the fan gives for each rotation.
  *
  */
#define FAN_CUSTOM_PPR 1


/**
  * \brief Fan-specific polarity for PWM.
  *
  * 0: PWM is logic 1 when "on"
  * 1: PWM is logic 0 when "on"
  */
#define FAN_CUSTOM_POL 0


/**
  * \brief Fan-specific open collector
  *
  * 0: FAN is driven with "high/low" output.
  * 1: FAN is driven with "open collector" output. (Requires external pull up)
  */
#define FAN_CUSTOM_OC 1

/**
  * \brief Fan-specific minimum PWM
  *
  * Some fans can not run with very low pulse width. FAN_CUSTOM_MIN_PWM_PCT specifies
  * the minimum pulse width for the fan used. Specified in percent.
  *
  */
#define FAN_CUSTOM_MIN_PWM_PCT 25

/**
  * \brief Fan-specific kick-start level
  *
  * Some fans needs to be "kick-started" in order to start at low PWM, FAN_CUSTOM_KICK_START_LVL_PCT defines
  * at which PWM level the fan used shall be kick-started
  */
#define FAN_CUSTOM_KICK_START_LVL_PCT 50

/**
  * \brief Fan-specific kick-start time
  *
  * Defines the time that the fan needs to be kick-started in mili seconds
  */
#define FAN_CUSTOM_KICK_START_ON_TIME MSEC_400


#endif
/****************************************************************************/
/*                                                                          */
/*  End of file.                                                            */
/*                                                                          */
/****************************************************************************/
