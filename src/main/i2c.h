//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#ifndef __I2C_H__
#define __I2C_H__

void  i2c_start (void);
void  i2c_stop (void);
uchar i2c_send_byte (uchar d);
uchar i2c_get_byte (uchar do_ack);

#endif




