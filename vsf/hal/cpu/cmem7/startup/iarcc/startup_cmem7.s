;*****************************************************************************
;* @file     start_cmem7.h
;*
;* @brief    CMEM7 startup file
;*
;*
;* @version
;* @date    
;*
;* @note               
;*           
;*****************************************************************************

        MODULE  ?cstartup

        ;; Forward declaration of sections.
        SECTION CSTACK:DATA:NOROOT(3)

        SECTION .intvec:CODE:NOROOT(2)

        EXTERN  __iar_program_start
        ;;EXTERN  SystemInit
        PUBLIC  __vector_table

        DATA
__vector_table
        DCD     sfe(CSTACK)
        DCD     Reset_Handler             ; Reset Handler
        DCD     NMI_Handler               ; NMI Handler
        DCD     HardFault_Handler         ; Hard Fault Handler
        DCD     MemManage_Handler         ; MPU Fault Handler
        DCD     BusFault_Handler          ; Bus Fault Handler
        DCD     UsageFault_Handler        ; Usage Fault Handler
        DCD     0                         ; Reserved
        DCD     0                         ; Reserved
        DCD     0                         ; Reserved
        DCD     0                         ; Reserved
        DCD     SVC_Handler               ; SVCall Handler
        DCD     DebugMon_Handler          ; Debug Monitor Handler
        DCD     0                         ; Reserved
        DCD     PendSV_Handler            ; PendSV Handler
        DCD     SysTick_Handler           ; SysTick Handler

        ; External Interrupts
        DCD     ETH_IRQHandler             ; ETH
        DCD     USB_IRQHandler             ; USB
        DCD     DMAC_IRQHandler            ; DMAC
        DCD     CAN0_IRQHandler            ; CAN0
        DCD     CAN1_IRQHandler            ; CAN1
        DCD     FP0_IRQHandler             ; FP[0:15]
        DCD     FP1_IRQHandler
        DCD     FP2_IRQHandler
        DCD     FP3_IRQHandler
        DCD     FP4_IRQHandler
        DCD     FP5_IRQHandler
        DCD     FP6_IRQHandler
        DCD     FP7_IRQHandler
        DCD     FP8_IRQHandler
        DCD     FP9_IRQHandler
        DCD     FP10_IRQHandler
        DCD     FP11_IRQHandler
        DCD     FP12_IRQHandler
        DCD     FP13_IRQHandler
        DCD     FP14_IRQHandler
        DCD     FP15_IRQHandler
        DCD     UART0_IRQHandler          ; UART0
        DCD     UART1_IRQHandler          ; UART1
        DCD     ADC_IRQHandler             ; ADC
        DCD     GPIO_IRQHandler            ; GPIO
        DCD     SPI1_IRQHandler            ; SPI1
        DCD     I2C1_IRQHandler            ; I2C1
        DCD     SPI0_IRQHandler            ; SPI0
        DCD     I2C0_IRQHandler            ; I2C0
        DCD     RTC_1S_IRQHandler          ; RTC 1S
        DCD     RTC_1MS_IRQHandler         ; RTC 1MS
        DCD     WDG_IRQHandler             ; Watchdog
        DCD     TIMER_IRQHandler           ; Timer 0 || 1 || 2 || 3
        DCD     DDRC_SW_PROC_IRQHandler    ; DDRC sw proc       				
        DCD     ETH_PMT_IRQHandler         ; ETH pmt
        DCD     PAD_IRQHandler             ; PAD
        DCD     DDRC_LANE_SYNC_IRQHandler  ; DDRC lane sync
        DCD     UART2_IRQHandler           ; UART2
        DCD     USERADD0_IRQHandler
        DCD     USERADD1_IRQHandler
        DCD     USERADD2_IRQHandler

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Default interrupt handlers.
;;
        THUMB
        PUBWEAK Reset_Handler
        SECTION .text:CODE:REORDER:NOROOT(2)
Reset_Handler
        LDR     R0, =__iar_program_start
        BX      R0

        PUBWEAK NMI_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
NMI_Handler
        B NMI_Handler

        PUBWEAK HardFault_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
HardFault_Handler
        B HardFault_Handler

        PUBWEAK MemManage_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
MemManage_Handler
        B MemManage_Handler

        PUBWEAK BusFault_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
BusFault_Handler
        B BusFault_Handler

        PUBWEAK UsageFault_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
UsageFault_Handler
        B UsageFault_Handler

        PUBWEAK SVC_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
SVC_Handler
        B SVC_Handler

        PUBWEAK DebugMon_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
DebugMon_Handler
        B DebugMon_Handler

        PUBWEAK PendSV_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
PendSV_Handler
        B PendSV_Handler

        PUBWEAK SysTick_Handler
        SECTION .text:CODE:REORDER:NOROOT(1)
SysTick_Handler
        B SysTick_Handler


        PUBWEAK ETH_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
ETH_IRQHandler  
        B ETH_IRQHandler

        PUBWEAK USB_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
USB_IRQHandler  
        B USB_IRQHandler

        PUBWEAK DMAC_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
DMAC_IRQHandler  
        B DMAC_IRQHandler

        PUBWEAK CAN0_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
CAN0_IRQHandler  
        B CAN0_IRQHandler

        PUBWEAK CAN1_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
CAN1_IRQHandler  
        B CAN1_IRQHandler

        PUBWEAK FP0_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
FP0_IRQHandler  
        B FP0_IRQHandler

        PUBWEAK FP1_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
FP1_IRQHandler  
        B FP1_IRQHandler

        PUBWEAK FP2_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
FP2_IRQHandler  
        B FP2_IRQHandler

        PUBWEAK FP3_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
FP3_IRQHandler  
        B FP3_IRQHandler

        PUBWEAK FP4_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
FP4_IRQHandler  
        B FP4_IRQHandler

        PUBWEAK FP5_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
FP5_IRQHandler  
        B FP5_IRQHandler

        PUBWEAK FP6_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
FP6_IRQHandler  
        B FP6_IRQHandler

        PUBWEAK FP7_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
FP7_IRQHandler  
        B FP7_IRQHandler

        PUBWEAK FP8_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
FP8_IRQHandler  
        B FP8_IRQHandler

        PUBWEAK FP9_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
FP9_IRQHandler  
        B FP9_IRQHandler

        PUBWEAK FP10_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
FP10_IRQHandler  
        B FP10_IRQHandler

        PUBWEAK FP11_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
FP11_IRQHandler  
        B FP11_IRQHandler

        PUBWEAK FP12_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
FP12_IRQHandler  
        B FP12_IRQHandler

        PUBWEAK FP13_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
FP13_IRQHandler  
        B FP13_IRQHandler

        PUBWEAK FP14_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
FP14_IRQHandler  
        B FP14_IRQHandler

        PUBWEAK FP15_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
FP15_IRQHandler  
        B FP15_IRQHandler

        PUBWEAK UART0_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
UART0_IRQHandler  
        B UART0_IRQHandler

        PUBWEAK UART1_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
UART1_IRQHandler  
        B UART1_IRQHandler

        PUBWEAK ADC_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
ADC_IRQHandler  
        B ADC_IRQHandler

        PUBWEAK GPIO_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
GPIO_IRQHandler  
        B GPIO_IRQHandler

        PUBWEAK SPI1_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
SPI1_IRQHandler  
        B SPI1_IRQHandler

        PUBWEAK I2C1_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
I2C1_IRQHandler  
        B I2C1_IRQHandler

        PUBWEAK SPI0_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
SPI0_IRQHandler  
        B SPI0_IRQHandler

        PUBWEAK I2C0_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
I2C0_IRQHandler  
        B I2C0_IRQHandler

        PUBWEAK RTC_1S_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
RTC_1S_IRQHandler  
        B RTC_1S_IRQHandler

        PUBWEAK RTC_1MS_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
RTC_1MS_IRQHandler  
        B RTC_1MS_IRQHandler

        PUBWEAK WDG_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
WDG_IRQHandler  
        B WDG_IRQHandler

        PUBWEAK TIMER_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
TIMER_IRQHandler  
        B TIMER_IRQHandler

        PUBWEAK DDRC_SW_PROC_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
DDRC_SW_PROC_IRQHandler  
        B DDRC_SW_PROC_IRQHandler

        PUBWEAK ETH_PMT_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
ETH_PMT_IRQHandler  
        B ETH_PMT_IRQHandler

        PUBWEAK PAD_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
PAD_IRQHandler  
        B PAD_IRQHandler

        PUBWEAK DDRC_LANE_SYNC_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
DDRC_LANE_SYNC_IRQHandler  
        B DDRC_LANE_SYNC_IRQHandler

        PUBWEAK UART2_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
UART2_IRQHandler  
        B UART2_IRQHandler

        PUBWEAK USERADD0_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
USERADD0_IRQHandler  
        B USERADD0_IRQHandler

        PUBWEAK USERADD1_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
USERADD1_IRQHandler  
        B USERADD1_IRQHandler

        PUBWEAK USERADD2_IRQHandler
        SECTION .text:CODE:REORDER:NOROOT(1)
USERADD2_IRQHandler  
        B USERADD2_IRQHandler

        END
        