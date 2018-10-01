	.syntax unified
	.cpu cortex-m3
	.fpu softvfp
	.thumb

.global  g_pfnVectors
.global  Default_Handler

.word  _sidata
.word  _sdata
.word  _edata
.word  _sbss
.word  _ebss


	.section  .text.Reset_Handler
	.weak  Reset_Handler
	.type  Reset_Handler, %function
Reset_Handler:
	ldr   sp, =_estack
	movs  r1, #0
	b     LoopCopyDataInit

CopyDataInit:
	ldr   r3, =_sidata
	ldr   r3, [r3, r1]
	str   r3, [r0, r1]
	adds  r1, r1, #4

LoopCopyDataInit:
	ldr   r0, =_sdata
	ldr   r3, =_edata
	adds  r2, r0, r1
	cmp   r2, r3
	bcc   CopyDataInit
	ldr   r2, =_sbss
	b     LoopFillZerobss

FillZerobss:
	movs  r3, #0
	str   r3, [r2], #4

LoopFillZerobss:
	ldr   r3, = _ebss
	cmp   r2, r3
	bcc   FillZerobss

	bl  SystemInit

	bl    main
	bx    lr
.size   Reset_Handler, .-Reset_Handler


	.section  .text.Default_Handler,"ax",%progbits
Default_Handler:
Infinite_Loop:
	b  Infinite_Loop
	.size  Default_Handler, .-Default_Handler


	.section  .isr_vector,"a",%progbits
	.type  g_pfnVectors, %object
	.size  g_pfnVectors, .-g_pfnVectors

g_pfnVectors:
	.word  _estack
	.word  Reset_Handler
	.word  NMI_Handler
	.word  HardFault_Handler
	.word  MemManage_Handler
	.word  BusFault_Handler
	.word  UsageFault_Handler
	.word  0
	.word  0
	.word  0
	.word  0
	.word  SVC_Handler
	.word  DebugMon_Handler
	.word  0
	.word  PendSV_Handler
	.word  SysTick_Handler
	.word  ETH_IRQHandler
	.word  USB_IRQHandler
	.word  DMAC_IRQHandler
	.word  CAN0_IRQHandler
	.word  CAN1_IRQHandler
	.word  FP0_IRQHandler
	.word  FP1_IRQHandler
	.word  FP2_IRQHandler
	.word  FP3_IRQHandler
	.word  FP4_IRQHandler
	.word  FP5_IRQHandler
	.word  FP6_IRQHandler
	.word  FP7_IRQHandler
	.word  FP8_IRQHandler
	.word  FP9_IRQHandler
	.word  FP10_IRQHandler
	.word  FP11_IRQHandler
	.word  FP12_IRQHandler
	.word  FP13_IRQHandler
	.word  FP14_IRQHandler
	.word  FP15_IRQHandler
	.word  UART0_IRQHandler
	.word  UART1_IRQHandler
	.word  ADC_IRQHandler
	.word  GPIO_IRQHandler
	.word  SPI1_IRQHandler
	.word  I2C1_IRQHandler
	.word  SPI0_IRQHandler
	.word  I2C0_IRQHandler
	.word  RTC_1S_IRQHandler
	.word  RTC_1MS_IRQHandler
	.word  WDG_IRQHandler
	.word  TIMER_IRQHandler
	.word  DDRC_SW_PROC_IRQHandler
	.word  ETH_PMT_IRQHandler
	.word  PAD_IRQHandler
	.word  DDRC_LANE_SYNC_IRQHandler
	.word  UART2_IRQHandler


	.weak  NMI_Handler
	.thumb_set NMI_Handler,Default_Handler
	
	.weak  HardFault_Handler
	.thumb_set HardFault_Handler,Default_Handler
	
	.weak  MemManage_Handler
	.thumb_set MemManage_Handler,Default_Handler
	
	.weak  BusFault_Handler
	.thumb_set BusFault_Handler,Default_Handler

	.weak  UsageFault_Handler
	.thumb_set UsageFault_Handler,Default_Handler

	.weak  SVC_Handler
	.thumb_set SVC_Handler,Default_Handler

	.weak  DebugMon_Handler
	.thumb_set DebugMon_Handler,Default_Handler

	.weak  PendSV_Handler
	.thumb_set PendSV_Handler,Default_Handler

	.weak  SysTick_Handler
	.thumb_set SysTick_Handler,Default_Handler

	.weak  ETH_IRQHandler
	.thumb_set ETH_IRQHandler,Default_Handler

	.weak  USB_IRQHandler
	.thumb_set USB_IRQHandler,Default_Handler

	.weak  DMAC_IRQHandler
	.thumb_set DMAC_IRQHandler,Default_Handler

	.weak  CAN0_IRQHandler
	.thumb_set CAN0_IRQHandler,Default_Handler

	.weak  CAN1_IRQHandler
	.thumb_set CAN1_IRQHandler,Default_Handler

	.weak  FP0_IRQHandler
	.thumb_set FP0_IRQHandler,Default_Handler

	.weak  FP1_IRQHandler
	.thumb_set FP1_IRQHandler,Default_Handler

	.weak  FP2_IRQHandler
	.thumb_set FP2_IRQHandler,Default_Handler

	.weak  FP3_IRQHandler
	.thumb_set FP3_IRQHandler,Default_Handler

	.weak  FP4_IRQHandler
	.thumb_set FP4_IRQHandler,Default_Handler

	.weak  FP5_IRQHandler
	.thumb_set FP5_IRQHandler,Default_Handler

	.weak  FP6_IRQHandler
	.thumb_set FP6_IRQHandler,Default_Handler

	.weak  FP7_IRQHandler
	.thumb_set FP7_IRQHandler,Default_Handler

	.weak  FP8_IRQHandler
	.thumb_set FP8_IRQHandler,Default_Handler

	.weak  FP9_IRQHandler
	.thumb_set FP9_IRQHandler,Default_Handler

	.weak  FP10_IRQHandler
	.thumb_set FP10_IRQHandler,Default_Handler

	.weak  FP11_IRQHandler
	.thumb_set FP11_IRQHandler,Default_Handler

	.weak  FP12_IRQHandler
	.thumb_set FP12_IRQHandler,Default_Handler

	.weak  FP13_IRQHandler
	.thumb_set FP13_IRQHandler,Default_Handler

	.weak  FP14_IRQHandler
	.thumb_set FP14_IRQHandler,Default_Handler

	.weak  FP15_IRQHandler
	.thumb_set FP15_IRQHandler,Default_Handler

	.weak  UART0_IRQHandler
	.thumb_set UART0_IRQHandler,Default_Handler

	.weak  UART1_IRQHandler
	.thumb_set UART1_IRQHandler,Default_Handler

	.weak  ADC_IRQHandler
	.thumb_set ADC_IRQHandler,Default_Handler

	.weak  GPIO_IRQHandler
	.thumb_set GPIO_IRQHandler,Default_Handler

	.weak  SPI1_IRQHandler
	.thumb_set SPI1_IRQHandler,Default_Handler

	.weak  I2C1_IRQHandler
	.thumb_set I2C1_IRQHandler,Default_Handler

	.weak  SPI0_IRQHandler
	.thumb_set SPI0_IRQHandler,Default_Handler

	.weak  I2C0_IRQHandler
	.thumb_set I2C0_IRQHandler,Default_Handler

	.weak  RTC_1MS_IRQHandler
	.thumb_set RTC_1MS_IRQHandler,Default_Handler

	.weak  WDG_IRQHandler
	.thumb_set WDG_IRQHandler,Default_Handler

	.weak  TIMER_IRQHandler
	.thumb_set TIMER_IRQHandler,Default_Handler

	.weak  DDRC_SW_PROC_IRQHandler
	.thumb_set DDRC_SW_PROC_IRQHandler,Default_Handler

	.weak  ETH_PMT_IRQHandler
	.thumb_set ETH_PMT_IRQHandler,Default_Handler

	.weak  PAD_IRQHandler
	.thumb_set PAD_IRQHandler,Default_Handler

	.weak  DDRC_LANE_SYNC_IRQHandler
	.thumb_set DDRC_LANE_SYNC_IRQHandler,Default_Handler

	.weak  UART2_IRQHandler
	.thumb_set UART2_IRQHandler,Default_Handler
