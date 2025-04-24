/*
 * ApplicationCode.h
 *
 *  Created on: Dec 30, 2023
 *      Author: Xavion
 */

#include "LCD_Driver.h"
#include "Button_Driver.h"
#include "Scheduler.h"
#include "stm32f4xx_hal.h"

#include <stdio.h>


#ifndef INC_APPLICATIONCODE_H_
#define INC_APPLICATIONCODE_H_


// --- Game Board Configuration ---
#define BOARD_ROWS          6
#define BOARD_COLS          7
#define PREVIEW_AREA_HEIGHT 50 // Space above the board for the moving coin
#define BOARD_Y_PREVIEW_OFFSET      PREVIEW_AREA_HEIGHT // Adjust board Y position

#define CIRCLE_RADIUS       14 // Adjust for desired size
#define CIRCLE_DIAMETER     (2 * CIRCLE_RADIUS)
#define BOARD_SPACING       4  // Pixels between circles and border

// Calculate board dimensions based on circles and spacing
#define BOARD_DRAW_WIDTH    (BOARD_COLS * CIRCLE_DIAMETER + (BOARD_COLS + 1) * BOARD_SPACING)
#define BOARD_DRAW_HEIGHT   (BOARD_ROWS * CIRCLE_DIAMETER + (BOARD_ROWS + 1) * BOARD_SPACING)

// Calculate top-left corner position to center the board (approx)
// Adjust Y offset to leave space for status/title/dropping coin area
#define BOARD_X_OFFSET      ((LCD_PIXEL_WIDTH - BOARD_DRAW_WIDTH) / 2)
#define BOARD_Y_OFFSET      50 // Start board lower down the screen

// --- Game Colors ---
#define BOARD_COLOR         LCD_COLOR_BLUE
#define EMPTY_SLOT_COLOR    LCD_COLOR_WHITE // Color of the holes
#define PLAYER1_COLOR       LCD_COLOR_RED   // P1 is Red
#define PLAYER2_COLOR       LCD_COLOR_YELLOW // P2 is Yellow
#define BACKGROUND_COLOR    LCD_COLOR_BLACK // Screen background

// Game Over Screen Button
#define RESTART_BUTTON_WIDTH  120
#define RESTART_BUTTON_HEIGHT 40
#define RESTART_BUTTON_X      ((LCD_PIXEL_WIDTH - RESTART_BUTTON_WIDTH) / 2)
#define RESTART_BUTTON_Y      (LCD_PIXEL_HEIGHT - RESTART_BUTTON_HEIGHT - 30) // Near bottom

// --- Slot State ---
typedef enum {
    SLOT_EMPTY,
    SLOT_PLAYER1,
    SLOT_PLAYER2
} SlotState_t;

// --- Coin Struct (Potentially for dropping animation, not board state) ---
typedef struct {
    uint16_t xPos;      // Current screen X position (center)
    uint16_t yPos;      // Current screen Y position (center)
    uint16_t color;     // PLAYER1_COLOR or PLAYER2_COLOR
    int8_t targetCol;   // Column the coin is aiming for (-1 if not active)
} Coin_t; // Renamed to avoid conflict if needed elsewhere

// --- Application States ---
typedef enum {
    APP_STATE_INIT,
    APP_STATE_MENU,
    APP_STATE_GAME_P1_TURN, // Player 1's turn (Human or AI)
    APP_STATE_GAME_P2_TURN, // Player 2's turn (Human or AI)
	APP_STATE_GAME_AI_TURN,
    APP_STATE_GAME_CHECK_WIN,   // State to check win condition after drop
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
void initializeGameBoard(void); // Sets all slots to empty
void drawGameBoard(void);
void drawMenuScreen(void);
void handleTouchInput(void);
void pollHardwareButton(void); // Replaces handleHardwareButton if using polling task
void handleGameTouchInput(uint16_t touchX, uint16_t touchY); // Specific game touch handler
void drawPreviewCoin(void);     // Draws the coin above the board
void dropCoin(void);            // Handles the logic for dropping the coin
int findLowestEmptyRow(int col); // Finds available row in a column
uint16_t getSlotColor(SlotState_t state); // Helper to get draw color

bool isTouchInside(uint16_t touchX, uint16_t touchY, uint16_t btnX, uint16_t btnY, uint16_t btnW, uint16_t btnH);
void LCD_Visual_Demo(void);
void LCD_Start_Menu(void);
void LCD_DisplayString(uint16_t Xpos, uint16_t Ypos, uint8_t *ptr, FONT_t* font, uint16_t textColor, uint16_t bgColor, bool opaque);

bool checkWin(SlotState_t player);
bool checkTie(void);
void drawGameOverScreen(void);
void handleAITurn(void);


#if (COMPILE_TOUCH_FUNCTIONS == 1)
void LCD_Touch_Polling_Demo(void);
#endif // (COMPILE_TOUCH_FUNCTIONS == 1)

#endif /* INC_APPLICATIONCODE_H_ */
