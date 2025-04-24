/*
 * InterruptControl.h
 *
 *  Created on: Feb 21, 2025
 *      Author: christiangreen
 */

#ifndef INTERRUPTCONTROL_H
#define INTERRUPTCONTROL_H

#include <stdint.h>
#include "stm32f4xx_hal.h"

// --- IRQ Number for the User Button ---

#define EXTI0_IRQ_NUMBER 6

// Prototypes
void NVIC_IRQEnable(uint8_t irqNumber);
void NVIC_IRQDisable(uint8_t irqNumber);
void NVIC_IRQClearPending(uint8_t irqNumber);
void NVIC_IRQSetPending(uint8_t irqNumber);
void EXTI_ClearPendingBit(uint8_t pinNumber);



#endif /* INTERRUPTCONTROL_H_ */
