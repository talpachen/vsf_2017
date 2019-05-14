/*!
    \file  gd32f3x0_misc.h
    \brief definitions for the MISC
*/

/*
    Copyright (C) 2017 GigaDevice

    2017-06-06, V1.0.0, firmware for GD32F3x0
*/

#ifndef GD32F3X0_MISC_H
#define GD32F3X0_MISC_H

#include "gd32f3x0.h"

/* constants definitions */
/* set the RAM and FLASH base address */
#define NVIC_VECTTAB_RAM            ((uint32_t)0x20000000)                      /*!< RAM base address */
#define NVIC_VECTTAB_FLASH          ((uint32_t)0x08000000)                      /*!< Flash base address */

/* set the NVIC vector table offset mask */
#define NVIC_VECTTAB_OFFSET_MASK    ((uint32_t)0x1FFFFF80)                      /*!< NVIC vector table offset mask */

/* the register key mask, if you want to do the write operation, you should write 0x5FA to VECTKEY bits */
#define NVIC_AIRCR_VECTKEY_MASK     ((uint32_t)0x05FA0000)                      /*!< NVIC VECTKEY mask */

/* priority group - define the pre-emption priority and the subpriority */
#define NVIC_PRIGROUP_PRE0_SUB4     ((uint32_t)0x700)                           /*!< 0 bits for pre-emption priority 4 bits for subpriority */
#define NVIC_PRIGROUP_PRE1_SUB3     ((uint32_t)0x600)                           /*!< 1 bits for pre-emption priority 3 bits for subpriority */
#define NVIC_PRIGROUP_PRE2_SUB2     ((uint32_t)0x500)                           /*!< 2 bits for pre-emption priority 2 bits for subpriority */
#define NVIC_PRIGROUP_PRE3_SUB1     ((uint32_t)0x400)                           /*!< 3 bits for pre-emption priority 1 bits for subpriority */
#define NVIC_PRIGROUP_PRE4_SUB0     ((uint32_t)0x300)                           /*!< 4 bits for pre-emption priority 0 bits for subpriority */

/* choose the method to enter or exit the lowpower mode */
#define SCB_SCR_SLEEPONEXIT         ((uint8_t)0x02)                             /*!< choose the the system whether enter low power mode by exiting from ISR */
#define SCB_SCR_SLEEPDEEP           ((uint8_t)0x04)                             /*!< choose the the system enter the DEEPSLEEP mode or SLEEP mode */
#define SCB_SCR_SEVONPEND           ((uint8_t)0x10)                             /*!< choose the interrupt source that can wake up the lowpower mode */

#define SCB_LPM_SLEEP_EXIT_ISR      SCB_SCR_SLEEPONEXIT                         /*!< low power mode by exiting from ISR */
#define SCB_LPM_DEEPSLEEP           SCB_SCR_SLEEPDEEP                           /*!< DEEPSLEEP mode or SLEEP mode */
#define SCB_LPM_WAKE_BY_ALL_INT     SCB_SCR_SEVONPEND                           /*!< wakeup by all interrupt */

/* choose the systick clock source */
#define SYSTICK_CLKSOURCE_HCLK_DIV8 ((uint32_t)0xFFFFFFFBU)                     /*!< systick clock source is from HCLK/8 */
#define SYSTICK_CLKSOURCE_HCLK      ((uint32_t)0x00000004U)                     /*!< systick clock source is from HCLK */

#endif /* GD32F3X0_MISC_H */
