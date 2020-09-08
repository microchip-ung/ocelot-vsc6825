;Copyright (c) 2004-2020 Microchip Technology Inc. and its subsidiaries.
;SPDX-License-Identifier: MIT


;* ************************************************************************ **
;*
;*
;* Internal 8051 registers
;*
;* Below is the list of Special Function Registers (SFR).
;*
;*
;* ************************************************************************ */

                GPR             DATA    080H
                PAGE_SEL        DATA    0B0H
                EPAGE_SEL       DATA    0C0H
                MMAP            DATA    0F2H
                RA_CTRL         DATA    0F4H
                RA_AD0_RD       DATA    0F6H
                RA_AD0_WR       DATA    0F7H
                RA_AD1          DATA    0F9H
                RA_AD2          DATA    0FAH
                RA_AD3          DATA    0FBH
                RA_DA0          DATA    0FCH
                RA_DA1          DATA    0FDH
                RA_DA2          DATA    0FEH
                RA_DA3          DATA    0FFH

                ; Paging bits
                PG_OP_0         BIT     0B0H
                PG_OP_1         BIT     0B1H
                PG_OP_2         BIT     0B2H
                PG_OP_3         BIT     0B3H
                PG_IFP_0        BIT     0B4H
                PG_IFP_1        BIT     0B5H
                PG_IFP_2        BIT     0B6H
                PG_IFP_3        BIT     0B7H

                ; General Purpose Regiser bits
                GPR_0           BIT     080H
                GPR_1           BIT     081H
                GPR_2           BIT     082H
                GPR_3           BIT     083H
                GPR_4           BIT     084H
                GPR_5           BIT     085H
                GPR_6           BIT     086H
                GPR_7           BIT     087H

;* ************************************************************************ **
;*
;*
;* Select chip interface
;*
;* Define which chip interface to use, either 8-bit parallel interface, SI
;* interface or SFR interface.
;*
;* Set USE_PI to 1 to use 8-bit interface, otherwise set USE_PI to 0.
;* Set USE_SI to 1 to use SI interface, otherwise set USE_SI to 0.
;* Set USE_SFR to 1 to use SFR interface, otherwise set USE_SFR to 0.
;* Set BOOT_VIA_SPI to 1 to boot from SPI Flash, otherwise set BOOT_VIA_SPI to 0.
;*
;*
;* ************************************************************************ */

                ; !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                ; ! Please, keep c-file hwconf.h updated accordingly !
                ; !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                $SET (USE_PI  = 0)
                $SET (USE_SI  = 0)
                $SET (USE_SFR = 1)
                $SET (BOOT_VIA_SPI = 1)

;* ************************************************************************ **
;*
;*
;* SPI configuration
;*
;*
;*
;* ************************************************************************ */

                ; SPI pins
                SS              BIT     094H    ; P1^4
                MOSI            BIT     095H    ; P1^5
                MISO            BIT     096H    ; P1^6
                SCK             BIT     097H    ; P1^7

                ; SPI registers present if the controller is e.g. a SST89V564
                SPCR            DATA    0D5H
                SPSR            DATA    0AAH
                SPDR            DATA    086H

SPI_DELAY_1     MACRO
                ; no delay needed
                ENDM

;* ************************************************************************ **
;*
;*
;* Select MOVX/MOVC for reading from flash pages
;*
;*
;*
;* ************************************************************************ */

                ;*
                ;* 0; movx
                ;* 1: movc
                ;*
                $SET (FAR_ACCESS_METHOD = 1)

;* ************************************************************************ **
;*
;*
;* Flash chip configuration
;*
;*
;*
;* ************************************************************************ */

$IF (UNMANAGED_ENHANCEMENT = 1)
                ;*
                ;* Define number of (64K) code banks used for a single flash image.
                ;* The number is used in l51_bank.a51 and it must be set to a value
                ;* that is supported by the code in l51_bank_ocelot.a51.
                ;*
                CODE_BANK_NUM   EQU    16
$ENDIF

;* ************************************************************************ **
;*
;*
;*      XRAM definitions
;*
;*      Define absolute start address (XDATASTART) and length (XDATALEN) of
;*      extern RAM (on-chip).
;*
;*      Used in startup.a51.
;*
;*
;* ************************************************************************ */

$IF (UNMANAGED_ENHANCEMENT = 1)
;    Be careful about the address here, if the code in common area is bigger than 0x8000 
;    (for example, no code are allocated into other bank), the addres needs to be bigger.
;    because this affects the "loading the internal memory from FLASH" in startup.a51 address setting.
                XDATASTART      EQU     08000H

                XDATALEN        EQU     08000H
$ELSE
                XDATASTART      EQU     0E000H

                XDATALEN        EQU     02000H
$ENDIF

/* ************************************************************************ **
 *
 *
 * Memory mapping
 * 0xC000 to 0xFFFF are reserved for XDATA RAM space
 *
 *
 * ************************************************************************ */
#define IMAGE_SIZE 0CFFFH




;* ************************************************************************ **
;*
;*
;* Macro for initializing control pins for external memory
;*
;*
;*
;* ************************************************************************ */

                ; This macro presets outputs for control of external memory.
                ; Used in startup.a51
STARTUP_XMEM    MACRO
                ENDM

;* ************************************************************************ **
;*
;*
;* xxx
;*
;*
;*
;* ************************************************************************ */



