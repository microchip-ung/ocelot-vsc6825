;Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
;SPDX-License-Identifier: MIT


$NOMOD51
$INCLUDE (REG52.INC)
$INCLUDE (hwconf.inc)
NAME    PIDRV

$IF (USE_PI = 1)

PUBLIC  _pi_read, _pi_write

PROG    SEGMENT CODE

        RSEG    PROG

;* ************************************************************************ */
;ulong pi_read (void *addr) small;
_pi_read:
;* ------------------------------------------------------------------------ --
;* Purpose     : Read specified register.
;* Remarks     : r2:   block mask plus subblock
;*               r1:   register address within block
;* Restrictions: Not c-compatible. Must be called via h2io.a51
;* See also    :
;* Example     :
;* ************************************************************************ */

        ;*
        ;* Build 17-bit address:
        ;*   A16-A14: block id
        ;*   A13-A10: subblock
        ;*   A9 - A2: register address
        ;*   A1 - A0: zeros
        ;* Save A16 in carry and A15-A0 in dptr
        ;*
        mov     a, r1           ; Get reg. addr.
        rlc     a               ; Shift one bit left, C gets MSB
        mov     r1, a           ; Save 7 bits from reg. addr. temporarily
        mov     a, r2           ; Get block id mask (7:5) and subblock (3:0).
        anl     a, #0FH         ; Isolate subblock
        addc    a, r2           ; Add subblock, ie. shift subblock 1 bit left,
                                ; and with C to get MSB of reg. addr.
        xch     a, r1           ; Save temp. and get rem. bits of reg. addr.
        rlc     a               ; Shift one bit more to the left, C gets MSB
        anl     a, #0FCH        ; Reset A1 and A0
        mov     dpl, a          ; Save A7-A0 in dpl
        mov     a, r1           ; Get A16-A9.
        rlc     a               ; Shift one bit left to get A16 in C and
                                ; to get yet another reg. addr. bit.
        mov     dph, a          ; Save A15-A8 in dph

        SELECT_SWITCH           ; Select switch and set A16 according to C

        ; Determine whether it is a fast or slow register by checking block
        ; id (Carry holds most significant bit of block id, acc holds
        ; remaining 2 bits in bit 7 and 6.
        jnc     L2_1            ; Blocks 1-3 are all slow
        anl     a, #0C0H        ; Isolate bit 7 and 6
        jz      L2_2            ; Block 4 is fast
        cjne    a, #0C0H, L2_1  ; Block 5 and 6 are slow
        jmp     L2_2            ; Block 7 is fast

L2_1:   ; Slow register.
        ; Do a dummy read from right address, then read result from the
        ; SLOWDATA register
        movx    a, @dptr        ; Dummy read
        SET_SWITCH_ADDR_MSB     ; Most significant bit of SLOWDATA register
        mov     dptr, #SLOWDATA_ADDR; Remaining address bits of SLOWDATA register

L2_2:   ; Fast register
        movx    a, @dptr        ; Read 1st byte
        mov     r4, a
        inc     dptr
        movx    a, @dptr        ; Read 2nd byte
        mov     r5, a
        inc     dptr
        movx    a, @dptr        ; Read 3rd byte
        mov     r6, a
        inc     dptr
        movx    a, @dptr        ; Read 4th byte
        mov     r7, a

        DESELECT_SWITCH
        ret

;* ************************************************************************ */
; void  pi_write (void *addr, ulong value) small;
_pi_write:
;* ------------------------------------------------------------------------ --
;* Purpose     : Write a value to specified register
;* Remarks     : r2:   block mask plus subblock
;*               r1:   register address within block
;*               r4-7: 32-bit value
;* Restrictions: Not C-compatible. Must be called via h2io.a51
;* See also    :
;* Example     :
; * ************************************************************************ */

        ;*
        ;* Build 17-bit address:
        ;*   A16-A14: block id
        ;*   A13-A10; subblock
        ;*   A9 - A2: register address
        ;*   A1 - A0: zeros
        ;* Save A16 in carry and A15-A0 in dptr
        ;*
        mov     a, r1           ; Get reg. addr.
        rlc     a               ; Shift one bit left, C gets MSB
        mov     r1, a           ; Save 7 bits from reg. addr. temporarily
        mov     a, r2           ; Get block id mask (7:5) and subblock (3:0).
        anl     a, #0FH         ; Isolate subblock
        addc    a, r2           ; Add subblock, ie. shift subblock 1 bit left,
                                ; and with C to get MSB of reg. addr.
        xch     a, r1           ; Save temp. and get rem. bits of reg. addr.
        rlc     a               ; Shift one bit more to the left, C gets MSB
        anl     a, #0FCH        ; Reset A1 and A0
        mov     dpl, a          ; Save A7-A0 in dpl
        mov     a, r1           ; Get A16-A9.
        rlc     a               ; Shift one bit left to get A16 in C and
                                ; to get yet another reg. addr. bit.
        mov     dph, a          ; Save A15-A8 in dph

        SELECT_SWITCH           ; Select switch and set A16 according to C

        mov     a, r4
        movx    @dptr, a        ; Write 1st byte
        inc     dptr
        mov     a, r5
        movx    @dptr, a        ; Write 2nd byte
        inc     dptr
        mov     a, r6
        movx    @dptr, a        ; Write 3rd byte
        inc     dptr
        mov     a, r7
        movx    @dptr, a        ; Write 4th byte

        DESELECT_SWITCH

        ret
$ENDIF
        END


















