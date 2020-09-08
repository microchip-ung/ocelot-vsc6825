;Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
;SPDX-License-Identifier: MIT

$NOMOD51
$INCLUDE (REG52.INC)

NAME    MISCUTIL3

PUBLIC  _mac_cmp
PUBLIC  _ip_cmp
PUBLIC  _mac_copy
PUBLIC  _ip_copy
PUBLIC  _mem_cmp
PUBLIC  _mem_copy
PUBLIC  _mem_set

PROG    SEGMENT CODE

        RSEG    PROG

;* ************************************************************************ */
; char mac_cmp (uchar xdata *mac_addr_1, uchar xdata *mac_addr_2) small
_mac_cmp:
;* ------------------------------------------------------------------------ --
;* Purpose     : Compare two mac addresses.
;* Remarks     : Return -1, if mac_addr_1 < mac_addr_2.
;*               Return  1, if mac_addr_1 > mac_addr_2
;*               Return  0, if mac_addr_1 == mac_addr_2
;* Restrictions:
;* See also    :
;* Example     :
; * ************************************************************************ */

        mov     r1, #6          ; Number of bytes to compare

L1_1:   mov     dph, r6         ; Get pointer to byte in first address
        mov     dpl, r7
        movx    a, @dptr        ; Get byte from first address
        mov     r2, a           ; Save byte temporary
        inc     dptr            ; Point to next byte in first address
        mov     r6, dph         ; Save pointer temporarily
        mov     r7, dpl

        mov     dph, r4         ; Get pointer to byte in second address
        mov     dpl, r5
        movx    a, @dptr        ; Get byte from second address
        inc     dptr            ; Point to next byte in second address
        mov     r4, dph         ; Save pointer temporarily
        mov     r5, dpl

        xch     a, r2
        clr     c
        subb    a, r2           ; Compare the two bytes
        jnz     L1_2            ; If different, go check sign

        djnz    r1, L1_1        ; Check if more bytes to compare
        mov     r7, #0          ; All bytes equal, return 0
        ret

L1_2:   jc      L1_3            ; If first address > second address
        mov     r7, #1          ; then return 1
        ret
L1_3:   mov     r7, #0FFH       ; else return -1
        ret

;* ************************************************************************ */
; char ip_cmp (uchar xdata *ip_addr_1, uchar xdata *ip_addr_2) small
_ip_cmp:
;* ------------------------------------------------------------------------ --
;* Purpose     : Compare two ip addresses.
;* Remarks     : Return -1, if ip_addr_1 < ip_addr_2.
;*               Return  1, if ip_addr_1 > ip_addr_2
;*               Return  0, if ip_addr_1 == ip_addr_2
;* Restrictions:
;* See also    :
;* Example     :
; * ************************************************************************ */

        mov     r1, #4          ; Number of bytes to compare
        jmp     L1_1

;* ************************************************************************ */
; void mac_copy (uchar xdata *dst_mac_addr, uchar xdata *src_mac_addr) small
_mac_copy:
;* ------------------------------------------------------------------------ --
;* Purpose     : Copy a mac address.
;* Remarks     : 
;* Restrictions:
;* See also    :
;* Example     :
; * ************************************************************************ */

        mov     r1, #6          ; Number of bytes to copy

L3_1:   mov     dph, r4         ; Get pointer to byte in source address
        mov     dpl, r5
        movx    a, @dptr        ; Read byte from source address
        inc     dptr            ; Point to next byte in source address
        mov     r4, dph         ; Save pointer temporarily
        mov     r5, dpl

        mov     dph, r6         ; Get pointer to byte in destination address
        mov     dpl, r7
        movx    @dptr, a        ; Write byte from source address
        inc     dptr            ; Point to next byte in destination address
        mov     r6, dph         ; Save pointer temporarily
        mov     r7, dpl

        djnz    r1, L3_1        ; Check if copy completed
        ret

;* ************************************************************************ */
; void ip_copy (uchar xdata *dst_ip_addr, uchar xdata *src_ip_addr) small
_ip_copy:
;* ------------------------------------------------------------------------ --
;* Purpose     : Copy an ip address.
;* Remarks     : 
;* Restrictions:
;* See also    :
;* Example     :
; * ************************************************************************ */

        mov     r1, #4          ; Number of bytes to copy
        jmp     L3_1

;* ************************************************************************ */
; uchar mem_cmp (uchar xdata *dst_mem_addr, uchar xdata *src_mem_addr, uchar size) small
_mem_cmp:
;* ------------------------------------------------------------------------ --
;* Purpose     : memory compare.
;* Remarks     : 
;* Restrictions: length must be less than 256, and compare address is xdata
;* See also    :
;* Example     :
; * ************************************************************************ */

        mov     a, r3
        mov     r1, a           ; Number of bytes to compare
        jmp     L1_1

;* ************************************************************************ */
; uchar mem_copy (uchar xdata *dst_mem_addr, uchar xdata *src_mem_addr, uchar size) small
_mem_copy:
;* ------------------------------------------------------------------------ --
;* Purpose     : memory compare.
;* Remarks     : 
;* Restrictions: length must be less than 256, and memory address is xdata
;* See also    :
;* Example     :
; * ************************************************************************ */

        mov     a, r3
        mov     r1, a           ; Number of bytes to copy
        jmp     L3_1

;* ************************************************************************ */
; uchar mem_set (uchar xdata *dst_mem_addr, uchar value, uchar size) small
_mem_set:
;* ------------------------------------------------------------------------ --
;* Purpose     : memory compare.
;* Remarks     : 
;* Restrictions: length must be less than 256, and memory address is xdata
;* See also    :
;* Example     :
; * ************************************************************************ */

        mov     a, r3
        mov     r1, a           ; Number of bytes to copy
        mov     a, r5           ; Get the byte to store
L4_1:   mov     dph, r6         ; Get pointer to byte in destination address
        mov     dpl, r7
        movx    @dptr, a        ; Write byte from source address
        inc     dptr            ; Point to next byte in destination address
        mov     r6, dph         ; Save pointer temporarily
        mov     r7, dpl

        djnz    r1, L4_1        ; Check if copy completed
        ret
        
        END


















