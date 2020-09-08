//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#ifndef __EVENT_H__
#define __EVENT_H__

#include "common.h"

void  callback_delayed_eee_lpi             (void);
void  callback_link_up                     (vtss_cport_no_t chip_port);
void  callback_link_down                   (vtss_cport_no_t chip_port);

#endif /* __EVENT_H__ */

