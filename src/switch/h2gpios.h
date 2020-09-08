//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#ifndef __H2_GPIOS_H__
#define __H2_GPIOS_H__

typedef enum {
    VTSS_GPIO_OUT,
    VTSS_GPIO_IN,
    VTSS_GPIO_IN_INT,
    VTSS_GPIO_ALT_0,
    VTSS_GPIO_ALT_1
} vtss_gpio_mode_t;

typedef enum {
    VTSS_SGPIO_BIT_0,
    VTSS_SGPIO_BIT_1,
    VTSS_SGPIO_BIT_2,
    VTSS_SGPIO_BIT_3,
    VTSS_SGPIO_BIT_END
} vtss_sgpio_bit_t;

/* Refer to chip sepecification: DEVCPU_GCB:SIO_CTRL:SIO_PORT_CFG:BIT_SOURCE
 * Output source select for the four outputs from
 * each port. The source select is encoded using
 * three bits for each output bit: Output bit0 is
 * defined by (2 down to 0), output bit1 is
 * defined by (5 down to 3), output bit2 is
 * defined by (8 down to 6), and output bit3 is
 * defined by (11 down to 9).
 * 0: Forced 0
 * 1: Forced 1
 * 2: Blink mode 0
 * 3: Blink mode 1
 * 4: Link activity blink mode 0
 * 5: Link activity blink mode 1
 * 6: Reserved
 * 7: Reserved
 */
typedef enum {
    VTSS_SGPIO_MODE_OFF,        /**< Forced '0' */
    VTSS_SGPIO_MODE_ON,         /**< Forced '1' */
    VTSS_SGPIO_MODE_BL_0,       /**< Blink mode 0 */
    VTSS_SGPIO_MODE_BL_1,       /**< Blink mode 1 */
    VTSS_SGPIO_MODE_LACT_0,     /**< Link activity blink mode 0 */
    VTSS_SGPIO_MODE_LACT_1,     /**< Link activity blink mode 1 */
    VTSS_SGPIO_MODE_LACT_0_REV, /**< Link activity blink mode 0 inversed polarity */
    VTSS_SGPIO_MODE_LACT_1_REV, /**< Link activity blink mode 1 inversed polarity */
    VTSS_SGPIO_MODE_END
} vtss_sgpio_mode_t;

void  h2_gpio_mode_set(uchar gpio_no, vtss_gpio_mode_t mode);
uchar h2_gpio_read(uchar gpio_no);
void  h2_gpio_write(uchar gpio_no, BOOL output_high);

void  h2_sgpio_enable();
uchar h2_sgpio_read(uchar sgpio_no, uchar bit_no);
void  h2_sgpio_write(uchar sgpio_no, uchar bit_no, ushort mode);

#endif
