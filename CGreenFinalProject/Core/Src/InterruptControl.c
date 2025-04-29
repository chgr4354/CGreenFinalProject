/*
 * InterruptControl.c
 *
 *  Created on: Feb 21, 2025
 *      Author: christiangreen
 */

#include "InterruptControl.h"

void NVIC_IRQEnable(uint8_t irqNumber) {
//    if (irqNumber < 32) {
//        *NVIC_ISER0 |= (1 << irqNumber);
//    }
	HAL_NVIC_EnableIRQ(irqNumber);
}

void NVIC_IRQDisable(uint8_t irqNumber) {
//    if (irqNumber < 32) {
//        *NVIC_ICER0 |= (1 << irqNumber);
//    }
	HAL_NVIC_DisableIRQ(irqNumber);
}

void NVIC_IRQClearPending(uint8_t irqNumber) {
//    if (irqNumber < 32) {
//        *NVIC_ICPR0 |= (1 << irqNumber);
//    }
	HAL_NVIC_ClearPendingIRQ(irqNumber);
}
void NVIC_IRQSetPending(uint8_t irqNumber) {
//    if (irqNumber < 32) {
//        *NVIC_ISPR0 |= (1 << irqNumber);
//    }
	HAL_NVIC_SetPendingIRQ(irqNumber);
}

void EXTI_ClearPendingBit(uint8_t pinNumber) {
    EXTI->PR |= (1 << pinNumber);
}

