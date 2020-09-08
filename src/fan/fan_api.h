//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#ifndef _FAN_API_H_
#define _FAN_API_H_

//************************************************
// Configuration definition
//************************************************

// Global configuration (configuration that are shared for all switches in the stack)
typedef struct {
    uchar   t_max; // The temperature were fan is running a full speed
    uchar   t_on;  // The temperature were cooling is needed (fan is started)
} fan_conf_t;


// Switch status (local status for a switch in the stack)
typedef struct {
    ushort chip_temp;              // Chip temperature ( in C.)
    uchar  fan_speed_setting_pct; // The fan speed level at which it is set to (in %).
    ulong  fan_speed;             // The speed the fan is currently running (in RPM)
} fan_local_status_t;



//************************************************
// Constants
//************************************************

// Defines chip temperature range (Select to be within a u8 type)
#define FAN_TEMP_MAX 255
#define FAN_TEMP_MIN 0

//************************************************
// Functions
//************************************************
void  fan_spec_init (void);
void  fan_control (void);
void  fan_init (void);

void  fan_get_local_status(fan_local_status_t *status);

uchar read_fan_conf_t_max (void);
uchar read_fan_conf_t_on  (void);
void  write_fan_conf_t_max (uchar t_max);
void  write_fan_conf_t_on  (uchar t_on);

void  load_fan_conf (void);
void  save_fan_conf (void);

#endif // _FAN_API_H_


//***************************************************************************
//  End of file.
//***************************************************************************
