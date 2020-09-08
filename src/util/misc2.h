//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT


#ifndef __MISC2_H__
#define __MISC2_H__

ushort bytes2ushort (uchar lsb, uchar msb) small;
ulong  ushorts2ulong (ushort lsw, ushort msw) small;
ushort high_w (ulong ul) small;
ulong  bit_mask_32 (uchar bit_no) small;
ushort bit_mask_16 (uchar bit_no) small;
uchar  bit_mask_8 (uchar bit_no) small;

void write_bit_8 (uchar bit_no, uchar value, uchar *dst_ptr) small;
void write_bit_16 (uchar bit_no, uchar value, ushort *dst_ptr) small;
void write_bit_32 (uchar bit_no, uchar value, ulong *dst_ptr) small;

bit test_bit_8 (uchar bit_no, uchar *src_ptr) small;
bit test_bit_16 (uchar bit_no, ushort *src_ptr) small;
bit test_bit_32 (uchar bit_no, ulong *src_ptr) small;

#endif
