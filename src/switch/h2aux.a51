;Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
;SPDX-License-Identifier: MIT

$NOMOD51
$INCLUDE (REG52.INC)

NAME    H2AUX

PUBLIC  _build_phy_write_cmd

H2PROG    SEGMENT CODE

        RSEG    H2PROG


;* ************************************************************************ */
;ulong build_phy_write_cmd (ushort value, uchar reg_no, uchar phy_no) small;
_build_phy_write_cmd:
;* ------------------------------------------------------------------------ --
;* Purpose     : Build 32-bit value to be written to MIIMCMD register for
;*               a write command. written in assembler for performance reasons.
;* Remarks     : value is in r6-r7, reg_no in r5 and phy_no in r3.
;* Restrictions:
;* See also    :
;* Example     :
; * ************************************************************************ */

        ;* mov phy_no to bit 25:21, ie. bit 9:5 of most significant 16 bits
        mov     a, r3   ; get phy no
        mov     b, #020H; shift left 5 times, ie. multiply with 32
        mul     ab      ; now b,a hold 16 most significant bits except for register no.
        ; mov reg_no to bit 20:16, ie. bit 4:0 of most significant 16 bits
        orl     a, r5   ; insert reg no

        ; deliver result in r4-r7 (r6-r7 already contain value)
        mov     r4, b
        mov     r5, a
		
        RET

        END





























