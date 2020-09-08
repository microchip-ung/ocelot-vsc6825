//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT


//==========================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####
// -------------------------------------------
// This file is part of eCos, the Embedded Configurable Operating System.
// Copyright (C) 2005, 2006 Free Software Foundation, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free
// Software Foundation; either version 2 or (at your option) any later
// version.
//
// eCos is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License
// along with eCos; if not, write to the Free Software Foundation, Inc.,
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
// As a special exception, if other files instantiate templates or use
// macros or inline functions from this file, or you compile this file
// and link it with other works to produce a work based on this file,
// this file does not by itself cause the resulting work to be covered by
// the GNU General Public License. However the source code for this file
// must still be made available in accordance with section (3) of the GNU
// General Public License v2.
//
// This exception does not invalidate any other reasons why a work based
// on this file might be covered by the GNU General Public License.
// -------------------------------------------
// ####ECOSGPLCOPYRIGHTEND####
//==========================================================================
//==========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):
// Contributors:  James Lin
// Date:          2010-08-04
// Description:   I2C driver for Microchip VCore-III
//####DESCRIPTIONEND####
//==========================================================================
#ifndef I2C_H_H
#define I2C_H_H

//
// Internal VCOREIII functions
//

void i2c_init(void);

ulong i2c_tx(const uchar i2c_scl_gpio,
            const uchar i2c_address,
            const uchar *tx_data,
            ulong count);
ulong i2c_rx(const uchar i2c_scl_gpio,
            const uchar i2c_address,
            uchar *rx_data,
            ulong count);
ulong i2c_eeprom_read(const uchar i2c_address,
                      ulong *mem_addr,
                      char *i2c_data);

#endif //I2C_H_H

