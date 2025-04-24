/*
 * Button_Driver.c
 *
 *  Created on: Feb 18, 2025
 *      Author: christiangreen
 */


#include "Button_Driver.h"



void Button_Init(void) {
	__HAL_RCC_GPIOA_CLK_ENABLE();
	GPIO_InitTypeDef buttonConfig;

    // Configure the button pin as input
    buttonConfig.Pin = GPIO_PIN_0;
    buttonConfig.Mode = GPIO_MODE_INPUT;
    buttonConfig.Pull = GPIO_NOPULL;
    buttonConfig.Speed = GPIO_SPEED_FREQ_LOW;
    buttonConfig.Alternate = GPIO_MODE_OUTPUT_OD;

    // Enable the clock for the button's GPIO port
    //GPIO_ClockControl(BUTTON_GPIO_PORT, ENABLE);

    // Initialize the GPIO pin
    //GPIO_Init(BUTTON_GPIO_PORT, &buttonConfig);
    HAL_GPIO_Init(GPIOA, &buttonConfig);
}

bool Button_IsPressed(void) {
    // Read the button's state using the GPIO driver
    //uint8_t pinState = GPIO_ReadInputPin(BUTTON_GPIO_PORT, BUTTON_PIN_NUMBER);
    //return (pinState == BUTTON_PRESSED);
	uint32_t pressed = HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_0);
	if (pressed) {
		return BUTTON_PRESSED;
	} else {
		return BUTTON_NOT_PRESSED;
	}
}

void Button_Interrupt_Init(void) {
	GPIO_InitTypeDef buttonConfig;

    buttonConfig.Pin = GPIO_PIN_0;
    buttonConfig.Mode = GPIO_MODE_INPUT;
    buttonConfig.Pull = GPIO_NOPULL;
    buttonConfig.Speed = GPIO_SPEED_FREQ_LOW;
    buttonConfig.Alternate = GPIO_MODE_OUTPUT_OD;

    //GPIO_ClockControl(BUTTON_GPIO_PORT, ENABLE);
    //GPIO_Init(BUTTON_GPIO_PORT, &buttonConfig);
    //GPIO_IRQConfig(EXTI0_IRQ_NUMBER, ENABLE); // Enable the interrupt
    HAL_GPIO_Init(GPIOA, &buttonConfig);
    HAL_NVIC_EnableIRQ(EXTI0_IRQn);
}
