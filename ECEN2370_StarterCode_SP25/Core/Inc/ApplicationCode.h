/*
 * ApplicationCode.h
 *
 *  Created on: Dec 30, 2023
 *      Author: Xavion
 */

#include "LCD_Driver.h"
#include "Button_Driver.h"
#include "stm32f4xx_hal.h"

#include <stdio.h>


#ifndef INC_APPLICATIONCODE_H_
#define INC_APPLICATIONCODE_H_

// --- Application States ---

typedef enum {
    APP_STATE_INIT,
    APP_STATE_MENU,
    APP_STATE_GAME_1P_SETUP, // Optional intermediate state
    APP_STATE_GAME_1P_PLAYER_TURN,
    APP_STATE_GAME_1P_AI_TURN,
    APP_STATE_GAME_2P_SETUP, // Optional intermediate state
    APP_STATE_GAME_2P_P1_TURN,
    APP_STATE_GAME_2P_P2_TURN,
    APP_STATE_GAME_OVER
} ApplicationState_t;

// --- Button Definitions for Touch ---
#define BUTTON_WIDTH        100
#define BUTTON_HEIGHT       50
#define BUTTON_PADDING      20 // Padding around buttons and screen edges

// Centered horizontally, slightly above vertical center
#define BUTTON_Y_OFFSET     -30
#define BUTTON1_X           ((LCD_PIXEL_WIDTH - (2 * BUTTON_WIDTH + BUTTON_PADDING)) / 2)
#define BUTTON1_Y           (LCD_PIXEL_HEIGHT / 2 - BUTTON_HEIGHT / 2 + BUTTON_Y_OFFSET)
#define BUTTON2_X           (BUTTON1_X + BUTTON_WIDTH + BUTTON_PADDING)
#define BUTTON2_Y           BUTTON1_Y


void ApplicationInit(void);
void ApplicationTask(void); // Main task called by scheduler
void drawMenuScreen(void);
void handleTouchInput(void);
void handleHardwareButton(void);
bool isTouchInside(uint16_t touchX, uint16_t touchY, uint16_t btnX, uint16_t btnY, uint16_t btnW, uint16_t btnH);
// Add other handlers as needed: handleGameLogic, handleGyroInput etc.
void LCD_Visual_Demo(void);
void LCD_Start_Menu(void);

#if (COMPILE_TOUCH_FUNCTIONS == 1)
void LCD_Touch_Polling_Demo(void);
#endif // (COMPILE_TOUCH_FUNCTIONS == 1)

#endif /* INC_APPLICATIONCODE_H_ */
