//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#ifndef __TXT_H__
#define __TXT_H__

#include "txt_moredef.h"

extern char code txt_01 [18];
extern char code txt_02 [10];
extern char code txt_03 [34];
extern char code txt_05 [6];
extern char code txt_06 [8];
extern char code txt_07 [7];
extern char code txt_08 [29];
extern char code txt_09 [29];
extern char code txt_10 [29];

extern char code txt_11 [34];
extern char code txt_12 [9];
extern char code txt_13 [7];
extern char code txt_14 [19];

extern char code txt_20 [9];


#define TXT_NO_STP_PSTATE       TXT_NO_STP_PSTATE_DIS

void print_txt (std_txt_t txt_no);
void print_txt_right (std_txt_t txt_no, uchar width);
void print_txt_left (std_txt_t txt_no, uchar width);
uchar cmp_cmd_txt (cmd_txt_t cmd_txt_no, char xdata *s1);
uchar txt_len (std_txt_t txt_no);
uchar cmd_txt_len (std_txt_t cmd_txt_no);
void print_cmd_txt (cmd_txt_t cmd_txt_no);
void print_cmd_txt_right (cmd_txt_t cmd_txt_no, uchar width);
void print_cmd_txt_left (cmd_txt_t cmd_txt_no, uchar width);


#endif





