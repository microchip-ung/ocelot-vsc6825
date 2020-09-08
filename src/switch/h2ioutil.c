//Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
//SPDX-License-Identifier: MIT



#include "common.h"     /* Always include common.h at the first place of user-defined herder files */
#include "vtss_api_base_regs.h"
#include "h2io.h"
#include "misc2.h"

#pragma NOAREGS
/*****************************************************************************
 *
 *
 * Defines
 *
 *
 *
 ****************************************************************************/

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

/*****************************************************************************
 *
 *
 * Local data
 *
 *
 *
 ****************************************************************************/

/* ************************************************************************ */
void h2_write_masked(ulong addr, ulong value, ulong mask)   small
/* ------------------------------------------------------------------------ --
 * Purpose     : Update specified bit(s) of a switch chip register.
 * Remarks     : addr: Luton26 register address, see vtss_luton26_regs.h.
 *               value: Holds bits (in right positions) to be written.
 *               mask: Bit mask specifying the bits to be updated.
 * Restrictions:
 * See also    :
 * Example     :
 ****************************************************************************/
{
    h2_write(addr, (h2_read(addr) & ~mask) | (value & mask));
}
