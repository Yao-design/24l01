
;-------------------------------------------------------------------------------
; Context Switch Code (ARMCC/armasm syntax)
;-------------------------------------------------------------------------------
    AREA    |.context_switch|, CODE, READONLY, ALIGN=2
    THUMB
    PRESERVE8

;-------------------------------------------------------------------------------
; Conditional Export Declarations
;-------------------------------------------------------------------------------
  ;  IF :DEF:TS_OTP_ROM || :DEF:TS_FULL_MODE || :DEF:TS_2G4_VER
    EXPORT CS_contextSave
  ;  ENDIF

  ;  IF :DEF:TS_OTP_ROM
    EXPORT CS_contextRestore
 ;   ENDIF

;-------------------------------------------------------------------------------
; CS_contextSave
;-------------------------------------------------------------------------------
 ;   IF :DEF:TS_OTP_ROM || :DEF:TS_FULL_MODE || :DEF:TS_2G4_VER
CS_contextSave PROC
    MRS    R2, CONTROL          ; Load the CONTROL register into R2
    MRS    R1, PSP              ; Load the process stack pointer into R1
    LDR    R0, =0
    MSR    CONTROL, R0          ; Switch to Main Stack Pointer
    ISB                         ; Instruction Synchronization Barrier

    PUSH   {R4-R7, LR}          ; Store R4-R7 and LR (5 words)
    MOV    R3, R8               ; Move {R8-R12} to {R3-R7}
    MOV    R4, R9
    MOV    R5, R10
    MOV    R6, R11
    MOV    R7, R12
    PUSH   {R3-R7}              ; Store R8-R12 (5 words)

    LDR    R4, =0x40080024      ; Load address of AON_CTRL->SOFT_RECORD1
    MRS    R3, MSP              ; Load the main stack pointer into R3
    STR    R3, [R4]             ; Store the MSP into AON_CTRL->SOFT_RECORD1

    PUSH   {R1, R2}             ; Store PSP and CONTROL

    DSB                         ; Data Synchronization Barrier

    ; all saved, Trigger deep sleep
    LDR    R4, =0x40080048      ; Setup AON_CTRL->GOSLEEP = 1
    MOVS   R0, #0x1
    STR    R0, [R4]

    WFI                         ; Wait for Interrupt (enter deep sleep)

while_1
	  B while_1

    ENDP
 ;   ENDIF

;    END
;-------------------------------------------------------------------------------
; CS_contextRestore
;-------------------------------------------------------------------------------
 ;   IF :DEF:TS_OTP_ROM     ; Export if TS_OTP_ROM is NOT defined
    EXPORT CS_contextRestore
CS_contextRestore PROC
;    LDR    R4, =0x40000100
;    MOVS   R5, #0xF8
;    STR    R5, [R4]             ; for test: set wdg timer: 1s
    
    LDR    R4, =0x40080080
    LDR    R4, [R4]
    MOVS   R5, #0x8
    ANDS   R4, R5
    BEQ.N  normal_branch        ; if not watch dog reset
    
    LDR    R4, =0x40080020
    MOVS   R5, #0x0
    STR    R5, [R4]             ; set PC to 0
    
    LDR    R4, =0x40080084
    MOVS   R5, #0xA
    STR    R5, [R4]             ; clr wdg/deep sleep reset flag
    
    LDR    R4, =0x40000008
    MOVS   R5, #0x1
    STR    R5, [R4]             ; reset system
    WFI

normal_branch
    ; Normal context restore
    LDR    R4, =0x40080024      ; load address of AON_CTRL->SOFT_RECORD1
    LDR    R4, [R4]             ; load the MSP from AON_CTRL->SOFT_RECORD1
    MSR    MSP, R4              ; restore the MSP from R4
    
    SUB    SP, #0x8
    POP    {R0, R1}             ; load PSP into R0, CONTROL into R1
	
    POP    {R3-R7}              ; load R8-R12 (via R3-R7)
    MOV    R8, R3
    MOV    R9, R4
    MOV    R10, R5
    MOV    R11, R6
    MOV    R12, R7
    
    POP    {R4-R7}              ; restore R4-R7
    POP    {R2}                 ; load LR
    
    MSR    PSP, R0              ; restore PSP
    MSR    CONTROL, R1          ; restore CONTROL register
    ISB                         ; instruction barrier
	
    BX     R2                   ; return using restored LR
    
    ENDP
 ;   ENDIF

    ALIGN 4
    END
