/*
 * Button_Driver.h
 *
 *  Created on: Feb 18, 2025
 *      Author: christiangreen
 */

#ifndef BUTTON_DRIVER_H_
#define BUTTON_DRIVER_H_


#include <stdbool.h>
//#include "InterruptControl.h"
#include "stm32f4xx_hal.h"


#define BUTTON_GPIO_PORT    GPIOA
#define BUTTON_PIN_NUMBER   0
#define BUTTON_PRESSED      1
#define BUTTON_NOT_PRESSED  0

void Button_Init(void);
bool Button_IsPressed(void);
void Button_Interrupt_Init(void);


#endif /* BUTTON_DRIVER_H_ */
