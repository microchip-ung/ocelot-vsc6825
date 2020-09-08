//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#include <string.h>

#include "common.h"     /* Always include common.h at the first place of user-defined herder files */
#include "print.h"
#include "uartdrv.h"
#include "misc1.h"
#include "misc2.h"  // For TEST_PORT_BIT_MASK()/test_bit_32()
#include "hwport.h"

#if TRANSIT_FTIME
#include "timer.h"
#endif

#ifndef NO_DEBUG_IF
#include "txt_moredef.h"

/*****************************************************************************
 *
 *
 * Defines
 *
 *
 *
 ****************************************************************************/

#define LEFT  0
#define RIGHT 1

#if TRANSIT_FTIME
#define ONE_DAY     ( 24 * 60 * 60 )
#define TEN_HOURS   ( 10 * 60 * 60 )
#define ONE_HOUR    (      60 * 60 )
#define TEN_MINUTES (      10 * 60 )
#define ONE_MINUTE  (           60 )
#define TEN_SECONDS (           10 )
#define HUNDRED_MS  (          100 )
#define TEN_MS      (           10 )
#endif

/*****************************************************************************
 *
 *
 * Typedefs and enums
 *
 *
 *
 ****************************************************************************/

/*****************************************************************************
 *
 *
 * Prototypes for local functions
 *
 *
 *
 ****************************************************************************/

static void print_dec_32 (ulong value, uchar adjust, uchar fieldwidth);

/*****************************************************************************
 *
 *
 * Local data
 *
 *
 ****************************************************************************/

#if TRANSIT_FTIME
struct timeb    g_delta_time;
#endif

/*****************************************************************************
 *
 *
 * Public Functions
 *
 *
 ****************************************************************************/

/* ************************************************************************ */
void print_str (const char *s)
/* ------------------------------------------------------------------------ --
 * Purpose     : Print a 0-terminated string.
 * Remarks     : s points to string.
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    while (*s != 0) {
        uart_put_byte(*s++);
    }
}

/* ************************************************************************ */
void print_n_str (const char *s,uchar nchar_width)
/* ------------------------------------------------------------------------ --
 * Purpose     : Print a 0-terminated string in n charecter width.
 * Remarks     : s points to string.
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    while ((*s != 0) || nchar_width) {
			  if (*s == 0) {
					uart_put_byte(' ');
				} else {
					uart_put_byte(*s++);
				}
			  nchar_width--;
    }
}

/* ************************************************************************ */
void println_str (const char *s)
/* ------------------------------------------------------------------------ --
 * Purpose     : Print a 0-terminated string.
 * Remarks     : s points to string.
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    print_str(s);
    print_cr_lf();
}

/* Show (32 bits) value with binary format
 * For example: 0000.0000.0000.0000.0000.0101.1110.1110
*/
void print_binary_32(ulong value)
{
    char idx;

    for (idx = 31; idx >= 0; idx--) {
        uart_put_byte((value & VTSS_BIT(idx)) ? '1' : '0');
        if (idx &&!(idx % 4)) {
            uart_put_byte('.');
        }
    }
}

/* Show (32 bits) value with various formats (Hex/Decimal/Binary)
 * For example:
 * Hex        Decimal    31     24 23     16 15      8 7       0
 * ---------- ---------- --------- --------- --------- ---------
 * 0x000005ee       1518 0000.0000.0000.0000.0000.0101.1110.1110
*/
void print_parse_info_32(ulong reg_val)
{
    /* Target and offset as parameter */
    print_str("Hex        Decimal    31     24 23     16 15      8 7       0");
    print_cr_lf();
    print_str("---------- ---------- --------- --------- --------- ---------");
    print_cr_lf();
    
    // Hex
    print_hex_prefix();
    print_hex_dw(reg_val);
    print_spaces(1);
    
    // Decimal
    print_dec_right(reg_val);
    print_spaces(1);
    
    // Binary
    print_binary_32(reg_val);
    print_cr_lf();
}

/* ************************************************************************ */
void print_hex_b (uchar value)
/* ------------------------------------------------------------------------ --
 * Purpose     : Print value of a byte as 2 hex nibbles.
 * Remarks     : value holds byte value to print.
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    uart_put_byte(hex_to_ascii_nib(value >> 4));
    uart_put_byte(hex_to_ascii_nib(value & 0x0f));
}

/* ************************************************************************ */
void print_hex_w (ushort value)
/* ------------------------------------------------------------------------ --
 * Purpose     : Print value of a word (16-bit integer) as 4 hex nibbles.
 * Remarks     : value holds word value to print.
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    print_hex_b(value >> 8);
    print_hex_b(value & 0xff);
}

/* ************************************************************************ */
void print_hex_dw (ulong value)
/* ------------------------------------------------------------------------ --
 * Purpose     : Print value of a dword (32-bit integer) as 8 hex nibbles.
 * Remarks     : value holds dword value to print.
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    print_hex_w(value >> 16);
    print_hex_w(value & 0xffff);
}

#if !defined(BRINGUP)
/* ************************************************************************ */
void print_dec (ulong value)
/* ------------------------------------------------------------------------ --
 * Purpose     : Print value of a dword (32-bit integer) as a decimal number.
 * Remarks     : value holds dword value to print.
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    uchar buf [10];
    uchar no_of_digits;

    /* Determine number of significant digits and isolate digits */
    no_of_digits = 0;
    while (value > 0) {
        buf[no_of_digits] = value % 10;
        value = value / 10;
        no_of_digits++;
    }

    /* Print each significant digit */
    if (no_of_digits == 0) {
        uart_put_byte('0');
    } else {
        no_of_digits--;
        while (no_of_digits != 0xff) {
            uart_put_byte(buf[no_of_digits] + '0');
            no_of_digits--;
        }
    }
}

#ifndef UNMANAGED_REDUCED_DEBUG_IF
/* ************************************************************************ */
void print_dec_8_right_2 (uchar value)
/* ------------------------------------------------------------------------ --
 * Purpose     : Print value of a byte as a 2-digit decimal number.
 * Remarks     : value holds byte value to print.
 * Restrictions: Value may not exceed 99.
 * See also    :
 * Example     :
 ****************************************************************************/
{
    if (value > 9) {
        uart_put_byte(((value / 10) % 10) + '0');
    } else {
        uart_put_byte(' ');
    }
    uart_put_byte((value % 10) + '0');
}
#endif
#endif /* BRINGUP */

#if UNMANAGED_PORT_STATISTICS_IF  || UNMANAGED_PORT_MAPPINGS
/* ************************************************************************ */
void print_dec_right (ulong value)
/* ------------------------------------------------------------------------ --
 * Purpose     : Print value of a dword (32-bit integer) as a decimal number
 *               right adjusted in a 10-char field.
 * Remarks     : value holds dword value to print.
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    print_dec_32(value, RIGHT, 10);
}
#endif


#if UNMANAGED_EEE_DEBUG_IF
/* ************************************************************************ */
void print_dec_nright (ulong value, uchar fieldwidth)
/* ------------------------------------------------------------------------ --
 * Purpose     : Print value of a dword (32-bit integer) as a decimal number
 *               right adjusted in a <fieldwidth>-char field.
 * Remarks     : value holds dword value to print.
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    print_dec_32(value, RIGHT, fieldwidth);
}
#endif

#if TRANSIT_VERIPHY || TRANSIT_FAN_CONTROL
#if UNMANAGED_TRANSIT_VERIPHY_DEBUG_IF || UNMANAGED_FAN_DEBUG_IF || defined(UNMANAGED_ENHANCEMENT)
void print_dec_16_right (ushort value, uchar width)
/* ------------------------------------------------------------------------ --
 * Purpose     : Print value of a word (16-bit integer) as a decimal number
 *               right adjusted in a field with width specified by parameter.
 * Remarks     : value holds word value to print. width specifies width of
 *               field (range 1-5).
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    uchar str [5];
    uchar j;

    if (width > 5) {
        width = 5;
    } else if (width == 0) {
        width = 1;
    }

    str[0] = '0';
    memset(&str[1], ' ', 4);

    j = 0;
    while (value != 0) {
        str[j] = (value % 10) + '0';
        value /= 10;
        j++;
    }

    while (width-- > 0) {
        uart_put_byte(str[width]);
    }
}
#endif
#endif

/* ************************************************************************ */
void print_hex_prefix (void)
/* ------------------------------------------------------------------------ --
 * Purpose     : Print a hex prefix, i.e. print "0x".
 * Remarks     :
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    uart_put_byte('0');
    uart_put_byte('x');
}

/* ************************************************************************ */
void print_cr_lf (void)
/* ------------------------------------------------------------------------ --
 * Purpose     : Print a <CR> (0x0d) and a <LF> (0x0a).
 * Remarks     :
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    uart_put_byte('\r');
    uart_put_byte('\n');
}

/* ************************************************************************ */
void print_ch (uchar ch)
/* ------------------------------------------------------------------------ --
 * Purpose     : Print a single char.
 * Remarks     : ch holds char to print in ASCII format.
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    uart_put_byte(ch);
}

#if !defined(BRINGUP)
#ifndef UNMANAGED_REDUCED_DEBUG_IF
/* ************************************************************************ */
void print_spaces (uchar count)
/* ------------------------------------------------------------------------ --
 * Purpose     : Print a specified number of spaces.
 * Remarks     : count holds number of spaces to print.
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    while (count-- > 0) {
        uart_put_byte(' ');
    }
}
#endif
#endif /* BRINGUP */

#if UNMANAGED_LLDP_DEBUG_IF
/* ************************************************************************ */
void print_ip_addr (const uchar xdata *ip_addr)
/* ------------------------------------------------------------------------ --
 * Purpose     : Print ip address in format xxx.xxx.xxx.xxx.
 * Remarks     : ip_addr points to a 4-byte array holding ip address.
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    uchar j;

    for (j = 0; j < 4; j++) {
        print_dec(*ip_addr++);
        if (j < 3) {
            print_ch('.');
        }
    }
}
#endif

#if !defined(BRINGUP)
/* ************************************************************************ */
void print_mac_addr (const uchar *mac_addr)
/* ------------------------------------------------------------------------ --
 * Purpose     : Print mac address in format xx-xx-xx-xx-xx-xx.
 * Remarks     : mac_addr points to a 6-byte array holding mac address.
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    uchar j;

    for (j = 0; j < 6; j++) {
        print_hex_b(*mac_addr++);
        if (j < 5) {
            print_ch('-');
        }
    }
}

#ifndef UNMANAGED_REDUCED_DEBUG_IF
/* ************************************************************************ */
void print_port_mac_addr (uchar port_no)
/* ------------------------------------------------------------------------ --
 * Purpose     : Print mac address for the specified port in format
 *               xx-xx-xx-xx-xx-xx.
 * Remarks     :
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    mac_addr_t mac_addr;

    get_mac_addr(port_no, mac_addr);
    print_mac_addr(mac_addr);
}
#endif
#endif /* BRINGUP */

/* ************************************************************************ */

void print_line (uchar width)
/* ------------------------------------------------------------------------ --
 * Purpose     : Print a line ('-') with specified width.
 * Remarks     :
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    while (width > 0) {
        uart_put_byte('-');
        width--;
    }
}

#if UNMANAGED_EEE_DEBUG_IF || UNMANAGED_PORT_STATISTICS_IF || UNMANAGED_PORT_MAPPINGS
/* ************************************************************************ */
static void print_dec_32 (ulong value, uchar adjust, uchar fieldwidth)
/* ------------------------------------------------------------------------ --
 * Purpose     : Print a 32-bit decimal value either left adjusted with no
 *               trailing spaces or right adjusted in a 10-char field with
 *               spaces in front.
 * Remarks     :
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    uchar buf [10];
    uchar no_of_digits;

    while(fieldwidth > 10) {
        uart_put_byte(' ');
        fieldwidth--;
    };

    memset(buf, ' ', sizeof(buf));

    /* Determine number of significant digits and isolate digits */
    no_of_digits = 0;
    while (value > 0) {
        buf[no_of_digits] = (value % 10) + '0';
        value = value / 10;
        no_of_digits++;
    }

    if (no_of_digits == 0) {
        buf[0] = '0';
        no_of_digits = 1;
    }

    if (adjust == RIGHT) {
        no_of_digits = fieldwidth;
    }

    while (no_of_digits-- > 0) {
        uart_put_byte(buf[no_of_digits]);
    }
}
#endif

#endif

#if TRANSIT_FTIME
void print_uptime(void)
{
    uchar           buf[13]; /* Buffer for: 12:34:56.789 */
    uchar           i = 0;
    struct timeb    t;

    ftime(&t);

    /* support less than 1 days */
    t.time = t.time % ONE_DAY;

    /* [0-2]_:__:__.___ */
    buf[i]    = '0';
    buf[i++] += t.time / TEN_HOURS;
    t.time    = t.time % TEN_HOURS;

    /* _[0-9]:__:__.___ */
    buf[i]    = '0';
    buf[i++] += t.time / ONE_HOUR;
    t.time    = t.time % ONE_HOUR;

    buf[i++]  = ':';

    /* __:[0-5]_:__.___ */
    buf[i]    = '0';
    buf[i++] += t.time / TEN_MINUTES;
    t.time    = t.time % TEN_MINUTES;

    /* __:_[0-9]:__.___ */
    buf[i]    = '0';
    buf[i++] += t.time / ONE_MINUTE;
    t.time    = t.time % ONE_MINUTE;

    buf[i++]  = ':';

    /* __:__:[0-5]_.___ */
    buf[i]    = '0';
    buf[i++] += t.time / TEN_SECONDS;
    t.time    = t.time % TEN_SECONDS;

    /* __:__:_[0-9].___ */
    buf[i]    = '0';
    buf[i++] += t.time;

    buf[i++]  = '.';

    /* __:__:__.[0-9]__ */
    buf[i]     = '0';
    buf[i++]  += t.millitm / HUNDRED_MS;
    t.millitm  = t.millitm % HUNDRED_MS;

    /* __:__:__._[0-9]_ */
    buf[i]     = '0';
    buf[i++]  += t.millitm / TEN_MS;
    t.millitm  = t.millitm % TEN_MS;

    /* __:__:__.__[0-9] */
    buf[i]     = '0';
    buf[i++]  += t.millitm;

    buf[i]    = 0; /* NULL-terminator */

    print_str(buf);
}

void print_delta(uchar check)
{
    if (check == 0)
        ftime(&g_delta_time);
    else {
        struct timeb    t;
        u16             delta;

        ftime(&t);

        delta  = (t.time - g_delta_time.time) * 1000;

        if (t.millitm >= g_delta_time.millitm)
            delta += t.millitm - g_delta_time.millitm;
        else
            delta -= g_delta_time.millitm - t.millitm;

        g_delta_time = t;

        if (delta) {
            print_str("check");
            print_ch ('0' + check);
            print_str(": ");
            print_dec(delta);
            print_cr_lf();
        }
    }
}
#endif /* TRANSIT_FTIME */

#if defined(DEBUG_PRINTF_ENABLE)
static void print_unsigned(unsigned v)
{
    uchar buf[6];
    uchar i;

    if (v == 0) {
        print_ch('0');
        return;
    }
    i = 0;
    while (v) {
        buf[i++] = ('0' + (v % 10));
        v /= 10;
    }
    while (i--)
        print_ch(buf[i]);
}

static void vtss_vprintf(const char *fmt, va_list ap)
{
    char *cp, *xp, ch;
    int fieldw = -1;
    bit hflg = 0, lflg = 0, nfw = 0;

    for (cp = fmt; *cp; cp++) {
        ch = *cp;
        if (ch == '%') {
            lflg = hflg = 0;
morearg:
            ch = *++cp;
            switch (ch) {
            case 'l' :
                lflg = 1;
                goto morearg;
            case 'h' :
                hflg = 1;
                goto morearg;
            case '-' :
                nfw = 1;
                goto morearg;
            case '0' :
            case '1' :
            case '2' :
            case '3' :
            case '4' :
            case '5' :
            case '6' :
            case '7' :
            case '8' :
            case '9' :
                if (fieldw < 0) {
                    fieldw = 0;
                }
                fieldw = fieldw * 10 + (ch - '0');
                goto morearg;
            case 's' :
                xp = va_arg(ap, char *);
                print_str(xp);
                break;
            case 'u' :
                if (lflg) {
                    print_dec(va_arg(ap, ulong));
                } else {
                    print_unsigned(hflg ? va_arg(ap, uchar) : va_arg(ap, unsigned));
                }
                break;
            case 'd' :
                if (lflg) {
                    long dv = va_arg(ap, long);
                    if (dv < 0) {
                        print_ch('-');
                        dv = -dv;
                    }
                    print_dec(dv);
                } else {
                    int dv = hflg ? va_arg(ap, char) : va_arg(ap, int);
                    if (dv < 0) {
                        print_ch('-');
                        dv = -dv;
                    }
                    print_unsigned(dv);
                }
                break;
            case 'x' :
                if (lflg) {
                    print_hex_dw(va_arg(ap, ulong));
                } else if (hflg) {
                    print_hex_b(va_arg(ap, uchar));
                } else {
                    print_hex_w(va_arg(ap, ushort));
                }
                break;
            case 'c' :
                print_ch(va_arg(ap, char));
                break;
            case '\0' :
                cp--;
                /*FALLTHROUGH*/
            case '%' :
                print_ch('%');
                break;
            default :
                print_ch('%');
                print_ch(ch);
                break;
            }
        } else {
            if (ch == '\r' && cp[1] != '\n')
                print_ch(ch);
            else if (ch == '\n')
                print_cr_lf();
            else
                print_ch(ch);
        }
    }
}

void debug_printf(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    vtss_vprintf(fmt, ap);
    va_end(ap);
}
#endif // DEBUG_PRINTF_ENABLE

#if TRANSIT_UNMANAGED_MAC_OPER_GET
void print_port_list(port_bit_mask_t port_mask)
{
    uchar port_no;
    uchar first;

    if (port_mask == 0) {
        print_str("None"); /* None */
    } else {
        first = TRUE;
        for (port_no = 0; port_no <= NO_OF_CHIP_PORTS; port_no++) {
            if (TEST_PORT_BIT_MASK(port_no, &port_mask)) {
                if (!first) {
                    print_ch(',');
                }
                if (port_no == CPU_CHIP_PORT) {
                    print_ch('C');
                } else {
                    print_dec(cport2uport(port_no));
                }
                first = FALSE;
            }
        }
    }
}
#endif // TRANSIT_UNMANAGED_MAC_OPER_GET
