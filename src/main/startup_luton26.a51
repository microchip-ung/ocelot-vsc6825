$NOMOD51
$INCLUDE(hwconf.inc)
;------------------------------------------------------------------------------
;  This file is part of the C51 Compiler package
;  Copyright (c) 1988-2002 Keil Elektronik GmbH and Keil Software, Inc.
;------------------------------------------------------------------------------
;  STARTUP.A51:  This code is executed after processor reset.
;
;  To translate this file use A51 with the following invocation:
;
;     A51 STARTUP.A51
;
;  To link the modified STARTUP.OBJ file to your application use the following
;  BL51 invocation:
;
;     BL51 <your object file list>, STARTUP.OBJ <controls>
;
;------------------------------------------------------------------------------
;
;  User-defined Power-On Initialization of Memory
;
;  With the following EQU statements the initialization of memory
;  at processor reset can be defined:
;
;               ; the absolute start-address of IDATA memory is always 0
IDATALEN        EQU    0FFH     ; the length of IDATA memory in bytes.
;
; See hwconf.inc for XDATASTART
; See hwconf.inc for XDATALEN
;
PDATASTART      EQU     0H      ; the absolute start-address of PDATA memory
PDATALEN        EQU     0H      ; the length of PDATA memory in bytes.
;
;  Notes:  The IDATA space overlaps physically the DATA and BIT areas of the
;          8051 CPU. At minimum the memory space occupied from the C51 
;          run-time routines must be set to zero.
;------------------------------------------------------------------------------
;
;  Reentrant Stack Initilization
;
;  The following EQU statements define the stack pointer for reentrant
;  functions and initialized it:
;
;  Stack Space for reentrant functions in the SMALL model.
IBPSTACK        EQU     0       ; set to 1 if small reentrant is used.
IBPSTACKTOP     EQU     0FFH+1  ; set top of stack to highest location+1.
;
;  Stack Space for reentrant functions in the LARGE model.      
XBPSTACK        EQU     0       ; set to 1 if large reentrant is used.
XBPSTACKTOP     EQU     0FFFFH+1; set top of stack to highest location+1.
;
;  Stack Space for reentrant functions in the COMPACT model.    
PBPSTACK        EQU     0       ; set to 1 if compact reentrant is used.
PBPSTACKTOP     EQU     0FFFFH+1; set top of stack to highest location+1.
;
;------------------------------------------------------------------------------
;
;  Page Definition for Using the Compact Model with 64 KByte xdata RAM
;
;  The following EQU statements define the xdata page used for pdata
;  variables. The EQU PPAGE must conform with the PPAGE control used
;  in the linker invocation.
;
PPAGEENABLE     EQU     0       ; set to 1 if pdata object are used.
;
PPAGE           EQU     0       ; define PPAGE number.
;
PPAGE_SFR       DATA    0A0H    ; SFR that supplies uppermost address byte
;               (most 8051 variants use P2 as uppermost address byte)
;
;------------------------------------------------------------------------------

; Standard SFR Symbols 
ACC     DATA    0E0H
B       DATA    0F0H
SP      DATA    81H
DPL     DATA    82H
DPH     DATA    83H
AUXR    DATA    8EH

;------------------------------------------------------------------------------

                NAME    ?C_STARTUP

?C_C51STARTUP   SEGMENT CODE
?STACK          SEGMENT IDATA

                RSEG    ?STACK
                DS      1

                EXTRN CODE (?C_START)
                PUBLIC  ?C_STARTUP

                CSEG    AT      0
?C_STARTUP:     LJMP    STARTUP1

                RSEG    ?C_C51STARTUP
STARTUP1:
                ; Mapping before memory and stack inialization
                MOV     SP,     #?STACK-1

                ; Use GPR to determin if boot code is there
                MOV     A,      GPR
                CJNE    A,      #1BH,   RUNONROM
                LJMP    RUNONRAM

RUNONROM:       CALL    hw_init

RUNONRAM:       NOP        

IF IDATALEN <> 0
                MOV     R0,     #IDATALEN - 1
                CLR     A
IDATALOOP:      MOV     @R0,    A
                DJNZ    R0,     IDATALOOP
ENDIF

IF XDATALEN <> 0
                STARTUP_XMEM

                MOV     DPTR,   #XDATASTART
                MOV     R7,     #LOW (XDATALEN)
  IF (LOW (XDATALEN)) <> 0
                MOV     R6,     #(HIGH (XDATALEN)) + 1
  ELSE
                MOV     R6,     #HIGH (XDATALEN)
  ENDIF
                CLR     A
XDATALOOP:      MOVX    @DPTR,  A
                INC     DPTR
                DJNZ    R7,     XDATALOOP
                DJNZ    R6,     XDATALOOP
ENDIF


IF PPAGEENABLE <> 0
                MOV     PPAGE_SFR, #PPAGE
ENDIF

IF PDATALEN <> 0
                MOV     R0,     #LOW (PDATASTART)
                MOV     R7,     #LOW (PDATALEN)
                CLR     A
PDATALOOP:      MOVX    @R0,    A
                INC     R0
                DJNZ    R7,     PDATALOOP
ENDIF

IF IBPSTACK <> 0
EXTRN DATA (?C_IBP)

                MOV     ?C_IBP, #LOW (IBPSTACKTOP)
ENDIF

IF XBPSTACK <> 0
EXTRN DATA (?C_XBP)

                MOV     ?C_XBP,     #HIGH (XBPSTACKTOP)
                MOV     ?C_XBP + 1, #LOW (XBPSTACKTOP)
ENDIF

IF PBPSTACK <> 0
EXTRN DATA (?C_PBP)
                MOV     ?C_PBP, #LOW (PBPSTACKTOP)
ENDIF

                MOV     SP,     #?STACK-1

; This code is required if you use L51_BANK.A51 with Banking Mode 4
; EXTRN CODE (?B_SWITCH0)
;               CALL    ?B_SWITCH0      ; init bank mechanism to code bank 0
                LJMP    ?C_START

hw_init:
                ; Speed up the SI interface by writing to IPU_CFG:SPI_MST_CFG.
                MOV     RA_DA3, #0
                MOV     RA_DA2, #0
                MOV     RA_DA1, #007H
                MOV     RA_DA0, #0E5H
                MOV     RA_AD3, #070H
                MOV     RA_AD2, #0
                MOV     RA_AD1, #0
                MOV     RA_AD0_WR, #050H        ; this write start the AHB write!

$IF (BOOT_VIA_SPI = 1)
                ; Configure registers for loading the internal memory from FLASH. ICPU_CFG:MEMACC
                MOV     RA_DA3, #HIGH (IMAGE_SIZE)
                MOV     RA_DA2, #LOW (IMAGE_SIZE)
                MOV     RA_DA1, #0
                MOV     RA_DA0, #0
                ; MOV     RA_AD3, #070H
                ; MOV     RA_AD2, #0
                ; MOV    RA_AD1, #0
                MOV     RA_AD0_WR, #078H ; this write start the AHB write!

                ; Start the actual load, the 8051 will be gated while the load is going on,
                ; so we can just continue as if nothing had happend (8051 will be released
                ; once the onchip memory contains the progam). ICPU_CFG:MEMACC_CTRL
                MOV     RA_DA3, #0
                MOV     RA_DA2, #0
                ; MOV     RA_DA1, #0
                MOV     RA_DA0, #001H
                ; MOV     RA_AD3, #070H
                ; MOV     RA_AD2, #0
                ; MOV     RA_AD1, #0
                MOV     RA_AD0_WR, #074H ; this write start the AHB write!
$ENDIF
  
                ; Errata, clear SFR register 0x8E prior to mapping internal memory.
                MOV     8EH,    #000H

                ; Enable mapping of onchip memory, note that we use SFR reg - not the CSR
                ; counterpart, this means that if the 8051 is reset (without resetting the
                ; the AHB system), then we will again load from external FLASH!
                MOV     MMAP,   #0AFH ; map all accesses to onchip memory (until 8051 reset)

                RET

                END

