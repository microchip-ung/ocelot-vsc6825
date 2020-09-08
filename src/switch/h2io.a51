;Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
;SPDX-License-Identifier: MIT

$NOMOD51
$INCLUDE (REG52.INC)
$INCLUDE (hwconf.inc)
NAME    H2IO

$IF (USE_PI = 1)
EXTRN  CODE (_pi_write, _pi_read)
$ELSE
$IF (USE_SI = 1)
EXTRN  CODE (_spi_write, _spi_read)
$ENDIF
$ENDIF

PUBLIC  _h2_read, _h2_write_val, _h2_write_addr


PROG    SEGMENT CODE

        RSEG    PROG

;* ************************************************************************ */
; ulong h2_read (ulong addr) small;
_h2_read:
;* ------------------------------------------------------------------------ --
;* Purpose     : Read a register from the SBA bus.
;* Remarks     : R4 is MSB and R7 is LSB.
;* Restrictions:
;* See also    :
;* Example     :
; * ************************************************************************ */
$IF (USE_SFR = 1)
        mov     RA_AD3, r4
        mov     RA_AD2, r5
        mov     RA_AD1, r6
        mov     RA_AD0_RD, r7

        mov     r4, RA_DA3
        mov     r5, RA_DA2
        mov     r6, RA_DA1
        mov     r7, RA_DA0
        ret
$ENDIF

;* ************************************************************************ */
; void h2_write_addr (ulong addr) small;
_h2_write_addr:
;* ------------------------------------------------------------------------ --
;* Purpose     : Save register address for writing. To be followed by call
;*               to _h2_write_ in order to complete the writing.
;* Remarks     : Writing to a register has been split in two functions in
;*               order to exploit the Keil c-compilers way of utilizing
;*               registers for parameters.
;* Restrictions: Use macro h2_write (in h2io.h) for access from c-modules.
;* See also    : _h2_write_
;* Example     :
; * ************************************************************************ */
        ;* Save register address for coming call to h2_write
$IF (USE_SFR = 1)        
        mov     RA_AD3, r4    ;MSB
        mov     RA_AD2, r5
        mov     RA_AD1, r6
        mov     RA_AD0_WR, r7 ;LSB
        
$ENDIF        
        ret

;* ************************************************************************ */
; void h2_write_val (ulong value);
_h2_write_val:
;* ------------------------------------------------------------------------ --
;* Purpose     : Completes a writing.
;* Remarks     : 
;* Restrictions: _h2_write_addr must have been called before calling _h2_write_.
;*               Use macro h2_write (in h2io.h) for access from c-modules.
;* See also    : _h2_write_addr
;* Example     :
; * ************************************************************************ */

$if (USE_SFR = 1)

        mov     RA_DA3, r4  ;MSB
        mov     RA_DA2, r5
        mov     RA_DA1, r6
        mov     RA_DA0, r7  ;LSB

        ret
$ENDIF

        END


















