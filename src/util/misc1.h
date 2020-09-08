//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT


#ifndef __MISC1_H__
#define __MISC1_H__

#if !defined(BRINGUP)
uchar xmemcmp(const uchar xdata *dst, const uchar xdata *src, ushort length);
#endif /* !BRINGUP */
void  xmemcpy(uchar xdata *dst, const uchar xdata *src, ushort length);
void  xmemset(uchar xdata *dst, uchar value, ushort length);

uchar ascii_to_hex_nib (uchar ch);
uchar hex_to_ascii_nib (uchar nib);
char  conv_to_upper_case (char ch);

#endif
