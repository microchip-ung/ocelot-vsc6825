//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#ifndef __UARTDRV_H__
#define __UARTDRV_H__

#define IIR_INTERRUPT_ID_MASK 0x0F
#define IIR_RX_AVAIL		  0x04
#define IIR_RX_CHAR_TIMEOUT	  0x0C
#define IIR_TX_EMPTY		  0x02

extern bit uart_rx_buf_overrun;

void  uart_init (void) small;
void  uart_interrupt (void) small;
#if !defined(NDEBUG) || (TRANSIT_LLDP && !TRANSIT_LLDP_REDUCED)
void  uart_redirect (uchar xdata *ptr) small;
#endif /* !defined(NDEBUG) || (TRANSIT_LLDP && !TRANSIT_LLDP_REDUCED) */
void  uart_interrupt (void) small;
bool  uart_byte_ready (void) small;
uchar uart_get_byte (void) small;
void  uart_put_byte (uchar ch) small;


#endif /* __UARTDRV_H__ */
