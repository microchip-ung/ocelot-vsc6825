//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT


#ifndef __MISC3_H__
#define __MISC3_H__

char mac_cmp (uchar xdata *mac_addr_1, uchar xdata *mac_addr_2) small;
char ip_cmp (uchar xdata *ip_addr_1, uchar xdata *ip_addr_2) small;
void mac_copy (uchar xdata *mac_addr_dst, uchar xdata *mac_addr_src);
void ip_copy (uchar xdata *dst_ip_addr, uchar xdata *src_ip_addr) small;

uchar mem_cmp(uchar xdata *dst_mem_addr, uchar xdata *src_mem_addr, uchar size) small;
uchar mem_copy(uchar xdata *dst_mem_addr, uchar xdata *src_mem_addr, uchar size) small;
uchar mem_set(uchar xdata *dst_mem_addr, uchar value, uchar size) small;
#endif
