//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#ifndef __CLIHND_H__
#define __CLIHND_H__

#define CLI_PROMPT()    { uart_put_byte('>'); }
void cli_tsk (void);
bool  cmd_ready (void);

#endif
