;Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
;SPDX-License-Identifier: MIT


$NOMOD51
$INCLUDE (REG52.INC)

NAME    MISCUTIL2

PUBLIC  _ushorts2ulong, _high_w
PUBLIC  _bytes2ushort
PUBLIC  _bit_mask_32
PUBLIC  _bit_mask_16
PUBLIC  _bit_mask_8
PUBLIC  _write_bit_8
PUBLIC  _write_bit_16
PUBLIC  _write_bit_32
PUBLIC  _test_bit_8
PUBLIC  _test_bit_16
PUBLIC  _test_bit_32

PROG    SEGMENT CODE

        RSEG    PROG
        
;* ************************************************************************ */
; ulong  bytes2ushort (uchar lsb, uchar msb) small;
_bytes2ushort:
;* ------------------------------------------------------------------------ --
;* Purpose     : Create an unsigned long from 2 unsigned shorts.
;* Remarks     : 
;* Restrictions:
;* See also    :
;* Example     :
; * ************************************************************************ */

        ; Exploit the compiler which parses 1st parameter (lsb) in r7 and
        ; 2nd parameter (msb) in r5, the return value
        ; will be a short in r6-r7.
        mov a, r5
        mov r6, a
        ret

;* ************************************************************************ */
; ulong  ushorts2ulong (ushort lsw, ushort msw) small;
_ushorts2ulong:
;* ------------------------------------------------------------------------ --
;* Purpose     : Create an unsigned long from 2 unsigned shorts.
;* Remarks     : 
;* Restrictions:
;* See also    :
;* Example     :
; * ************************************************************************ */

        ; Exploit the compiler which parses 1st parameter (lsw) in r6, r7 and
        ; 2nd parameter (msw) in r4, r5, i.e. without doing anything the return value
        ; will be a long in r4-r7.

        ret

;* ************************************************************************ */
; ushort  high_w (ulong ul) small;
_high_w:
;* ------------------------------------------------------------------------ --
;* Purpose     : Get high 16 bits of an unsigned long.
;* Remarks     : 
;* Restrictions:
;* See also    :
;* Example     :
; * ************************************************************************ */

        mov     a, r4
        mov     r6, a
        mov     a, r5
        mov     r7, a

        ret

; Bit table used for converting a bit number to a bit mask
bit_table:
        db      01H
        db      02H
        db      04H
        db      08H
        db      10H
        db      20H
        db      40H
        db      80H

;* ************************************************************************ */
; ulong  bit_mask_32 (uchar bit_no) small;
_bit_mask_32:
;* ------------------------------------------------------------------------ --
;* Purpose     : Build a 32-bit mask with ONE bit set according to specified
;*               bit number.
;* Remarks     : r7 holds bit number.
;* Restrictions:
;* See also    :
;* Example     :
; * ************************************************************************ */

        mov     a, r7
        mov     b, a    ; temporary

        ; Preset 32-bit mask with 0s
        clr     a
        mov     r7, a
        mov     r6, a
        mov     r5, a
        mov     r4, a

        mov     a, b
        mov     dptr, #bit_table

        ; If bit in range 0-7
        cjne    a, #8, l1_1
l1_1:   jnc     l1_2
        ; bit_no < 8
        movc    a, @a + dptr
        mov     r7, a
        ret

        ; If bit in range 8-15
l1_2:   subb    a, #8
        cjne    a, #8, l1_3
l1_3:   jnc     l1_4
        ; 8 <= bit_no < 16
        movc    a, @a + dptr
        mov     r6, a
        ret

        ; If bit in range 16-23
l1_4:   subb    a, #8
        cjne    a, #8, l1_5
l1_5:   jnc     l1_6
        ; 16 <= bit_no < 24
        movc    a, @a + dptr
        mov     r5, a
        ret

        ; If bit in range 24-31
l1_6:   subb    a, #8
        movc    a, @a + dptr
        mov     r4, a
        ret
;* ************************************************************************ */
; ushort bit_mask_16 (uchar bit_no) small;
_bit_mask_16:
;* ------------------------------------------------------------------------ --
;* Purpose     : Build a 16-bit mask with ONE bit set according to specified
;*               bit number.
;* Remarks     : r7 holds bit number.
;* Restrictions:
;* See also    :
;* Example     :
; * ************************************************************************ */

        clr     a
        mov     r6, a
        xch     a, r7
        mov     dptr, #bit_table

        ; If bit in range 0-7
        cjne    a, #8, l2_1
l2_1:   jnc     l2_2
        ; bit_no < 8
        movc    a, @a + dptr
        mov     r7, a
        ret

        ; If bit in range 8-15
l2_2:   subb    a, #8
        movc    a, @a + dptr
        mov     r6, a
        ret

;* ************************************************************************ */
; uchar bit_mask_8 (uchar bit_no) small;
_bit_mask_8:
;* ------------------------------------------------------------------------ --
;* Purpose     : Build a 8-bit mask with ONE bit set according to specified
;*               bit number.
;* Remarks     : r7 holds bit number.
;* Restrictions:
;* See also    :
;* Example     :
; * ************************************************************************ */

        mov     a, r7
        mov     dptr, #bit_table
        movc    a, @a + dptr
        mov     r7, a
        ret

;* ************************************************************************ */
; void write_bit_8 (uchar bit_no, uchar value, uchar *dst_ptr) small;
_write_bit_8:
;* ------------------------------------------------------------------------ --
;* Purpose     : Set specified bit to specified value in byte pointed to by
;*               dst_ptr.
;* Remarks     : r7 holds bit number, r5 holds value (0 or 1), 
;*               r1-r3 holds destination pointer.
;* Restrictions:
;* See also    :
;* Example     :
; * ************************************************************************ */

        ;* Convert bit number to bit mask and complement bit mask
        ;*----------------------------------------------------------------
        mov     dptr, #bit_table
        mov     a, r7
        movc    a, @a + dptr
        mov     r7, a
        cpl     a
        mov     r6, a

        ;* Check if external or internal ram
        ;*----------------------------------------------------------------
        cjne    r3, #1, l3_3

        ;* External Ram
        ;*----------------------------------------------------------------
        mov     dpl, r1
        mov     dph, r2
        movx    a, @dptr        ; Read byte from external ram
        anl     a, r6           ; Reset bit
        cjne    r5, #0, l3_1    ; Check if bit is to be set
        jmp     l3_2            ; Skip setting bit
l3_1:   orl     a, r7           ; Set bit
l3_2:   movx    @dptr, a        ; Update byte in external ram
        ret

        ;* Skip update if neither external nor internal ram
        ;*----------------------------------------------------------------
l3_3:   cjne    r3, #0, l3_6

        ;* Internal Ram
        ;*----------------------------------------------------------------
        mov     a, r1
        mov     r0, a
        mov     a, @r0          ; Read byte from internal ram
        anl     a, r6           ; Reset bit
        cjne    r5, #0, l3_4    ; Check if bit is to be set
        jmp     l3_5            ; Skip setting bit
l3_4:   orl     a, r7           ; Set bit
l3_5:   mov     @r0, a          ; Update byte in internal ram
l3_6:   ret

;* ************************************************************************ */
; void write_bit_16 (uchar bit_no, uchar value, ushort *dst_ptr) small;
_write_bit_16:
;* ------------------------------------------------------------------------ --
;* Purpose     : Set specified bit to specified value in ushort pointed to by
;*               dst_ptr.
;* Remarks     : r7 holds bit number, r5 holds value (0 or 1), 
;*               r1-r3 holds destination pointer.
;* Restrictions:
;* See also    :
;* Example     :
; * ************************************************************************ */

        ;* Check if bit number is < 8, i.e. bit is located in second byte.
        ;*----------------------------------------------------------------
        cjne    r7, #8, l5_1
l5_1:   jnc     l5_2

        ;* Bit is located in second byte, adjust destination pointer in
        ;* order to use function _write_bit_8
        ;*----------------------------------------------------------------
        inc     r1
        mov     a, r1
        jnz     l5_3
        inc     r2
        jmp     l5_3

        ;* Bit is located in first byte, adjust bit number in ordet to use
        ;* function _write_bit_8
        ;*----------------------------------------------------------------
l5_2:   mov     a, #7
        anl     a, r7
        mov     r7, a

        ;* Update bit in the byte
        ;*----------------------------------------------------------------
l5_3:   lcall   _write_bit_8
        ret

;* ************************************************************************ */
; void write_bit_32 (uchar bit_no, uchar value, ulong *dst_ptr) small;
_write_bit_32:
;* ------------------------------------------------------------------------ --
;* Purpose     : Set specified bit to specified value in ulong pointed to by
;*               dst_ptr.
;* Remarks     : r7 holds bit number, r5 holds value (0 or 1), 
;*               r1-r3 holds destination pointer.
;* Restrictions:
;* See also    :
;* Example     :
; * ************************************************************************ */

        ;* Calc byte offset as (31 - bit_no) / 8
        ;*----------------------------------------------------------------
        mov     a, #31
        clr     c
        subb    a, r7
        rrc     a
        rrc     a
        rrc     a
        anl     a, #3

        ;* Adjust destination pointer according to byte offset
        ;*----------------------------------------------------------------
        add     a, r1
        mov     r1, a
        clr     a
        addc    a, r2
        mov     r2, a

        ;* Adjust bit number to be within byte
        ;*----------------------------------------------------------------
        mov     a, #7
        anl     a, r7
        mov     r7, a

        ;* Update bit in the byte
        ;*----------------------------------------------------------------
        lcall   _write_bit_8
        ret

;* ************************************************************************ */
; bit test_bit_8 (uchar bit_no, uchar *src_ptr) small;
_test_bit_8:
;* ------------------------------------------------------------------------ --
;* Purpose     : Test specified bit in byte pointed to by src_ptr.
;* Remarks     : r7 holds bit number, r1-r3 holds source pointer.
;* Restrictions:
;* See also    :
;* Example     :        
; * ************************************************************************ */

        ;* Convert bit number to bit mask
        ;*----------------------------------------------------------------
        mov     dptr, #bit_table
        mov     a, r7
        movc    a, @a + dptr
        mov     r7, a

        ;* Check if external or internal ram
        ;*----------------------------------------------------------------
        cjne    r3, #1, l4_1

        ;* External Ram
        ;*----------------------------------------------------------------
        mov     dpl, r1
        mov     dph, r2
        movx    a, @dptr        ; Read byte from external ram
        jmp     l4_2

        ;* Skip check if neither external nor internal ram
        ;*----------------------------------------------------------------
l4_1:   cjne    r3, #0, l4_3
        mov     a, r1
        mov     r0, a
        mov     a, @r0          ; Read byte from internal ram

        ;* Check bit
        ;*----------------------------------------------------------------
l4_2:   anl     a, r7
        jz      l4_3
        setb    c
        ret
	
l4_3:   clr     c
        ret

;* ************************************************************************ */
; bit test_bit_16 (uchar bit_no, ushort *src_ptr) small;
_test_bit_16:
;* ------------------------------------------------------------------------ --
;* Purpose     : Test specified bit in ushort pointed to by src_ptr.
;* Remarks     : r7 holds bit number, r1-r3 holds source pointer.
;* Restrictions:
;* See also    :
;* Example     :
; * ************************************************************************ */

        ;* Check if bit number is < 8, i.e. bit is located in second byte.
        ;*----------------------------------------------------------------
        cjne    r7, #8, l6_1
l6_1:   jnc     l6_2

        ;* Bit is located in second byte, adjust destination pointer in
        ;* order to use function _test_bit_8
        ;*----------------------------------------------------------------
        inc     r1
        mov     a, r1
        jnz     l6_3
        inc     r2
        jmp     l6_3

        ;* Bit is located in first byte, adjust bit number in ordet to use
        ;* function _test_bit_8
        ;*----------------------------------------------------------------
l6_2:   mov     a, #7
        anl     a, r7
        mov     r7, a

        ;* Check bit in the byte
        ;*----------------------------------------------------------------
l6_3:   lcall   _test_bit_8

        ret

;* ************************************************************************ */
; void test_bit_32 (uchar bit_no, uchar value, ulong *src_ptr) small;
_test_bit_32:
;* ------------------------------------------------------------------------ --
;* Purpose     : Test specified bit in ulong pointed to by src_ptr.
;* Remarks     : r7 holds bit number, r1-r3 holds source pointer.
;* Restrictions:
;* See also    :
;* Example     :
; * ************************************************************************ */

        ;* Calc byte offset as (31 - bit_no) / 8
        ;*----------------------------------------------------------------
        mov     a, #31
        clr     c
        subb    a, r7
        rrc     a
        rrc     a
        rrc     a
        anl     a, #3

        ;* Adjust destination pointer according to byte offset
        ;*----------------------------------------------------------------
        add     a, r1
        mov     r1, a
        clr     a
        addc    a, r2
        mov     r2, a

        ;* Adjust bit number to be within byte
        ;*----------------------------------------------------------------
        mov     a, #7
        anl     a, r7
        mov     r7, a

        ;* Check bit in the byte
        ;*----------------------------------------------------------------
        lcall   _test_bit_8

        ret

        END


















