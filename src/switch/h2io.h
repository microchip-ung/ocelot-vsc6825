//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#ifndef __H2IO_H__
#define __H2IO_H__

#include <REG52.H>


/*****************************************************************************
 * I/O unprotected functions. Used only at initailization before EA is enabled
 ****************************************************************************/
/* Function to be used for reading registers */
ulong h2_read(ulong addr) small;

/* Assembler functions used by above macro */
void h2_write_addr(ulong addr) small;
void h2_write_val(ulong value) small;

/* Macro to be used for writing registers */
#define h2_write(addr, value)       { h2_write_val(value);  h2_write_addr(addr); }

/* Functions to be used for masked writings.
   The functions are found in h2ioutil.c */
void h2_write_masked(ulong addr, ulong value, ulong mask)   small;

/*****************************************************************************
 * I/O protected functions. Used after EA is enabled;
 ****************************************************************************/
/*****************************************************************************
 *
 * Important !!!
 *
 * The protection can not be achieved by functions as we do not want to
 * enable reentrant to make things more complex
 *
 * For the 8051 software, the interrupt "MUST" be disabled while accessing the
 * switch registers. Otherwise, the read value or write operation maybe unmatch
 * the expected result.
 *
 ****************************************************************************/
#define H2_READ(addr, value)                { EA = 0; value = h2_read(addr); EA = 1; }

#define H2_WRITE(addr, value)               { EA = 0; h2_write(addr, value); EA = 1; }

#define H2_WRITE_MASKED(addr, value, mask)  { EA = 0; h2_write_masked(addr, value, mask); EA = 1; }

#endif // __H2IO_H__
