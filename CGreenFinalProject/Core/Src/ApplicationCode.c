/*
 * ApplicationCode.c
 *
 *  Created on: Dec 30, 2023 (updated 11/12/2024) Thanks Donavon! 
 *      Author: Xavion
 */

#include "ApplicationCode.h"
#include <string.h>

/* Static variables */


extern void initialise_monitor_handles(void); 

//#if COMPILE_TOUCH_FUNCTIONS == 1

// --- Global/Static Variables ---
static ApplicationState_t currentAppState = APP_STATE_INIT;
static STMPE811_TouchData StaticTouchData;
static SlotState_t gameBoard[BOARD_ROWS][BOARD_COLS];
//#endif // COMPILE_TOUCH_FUNCTIONS

// --- Game State Variables ---
static SlotState_t currentPlayer; // Tracks the current player (SLOT_PLAYER1 or SLOT_PLAYER2)
static int8_t currentColumn = BOARD_COLS / 2; // Start preview coin in the middle column (0-6)
static uint16_t previousPreviewCoinX = 0; // To help erase previous preview coin

// Score & Timer Variables
static uint32_t player1Score = 0;
static uint32_t player2Score = 0;
static uint32_t roundStartTime = 0;
static uint32_t roundEndTime = 0;
static SlotState_t roundWinner = SLOT_EMPTY; // Tracks who won the round
static bool isOnePlayerMode = false; // Track game mode

// --- Debounce Timers ---
static uint32_t lastTouchMoveTime = 0;
static uint32_t lastButtonPressTime = 0;
#define TOUCH_MOVE_DEBOUNCE 100 // ms between allowed preview coin moves
#define BUTTON_PRESS_DEBOUNCE 300 // ms between allowed button presses

// External Handles
extern RNG_HandleTypeDef hrng; // Need RNG handle for AI

void ApplicationInit(void)
{
	initialise_monitor_handles(); // Allows printf functionality
    LTCD__Init();
    LTCD_Layer_Init(0);
    LCD_Clear(0,LCD_COLOR_WHITE);

    #if COMPILE_TOUCH_FUNCTIONS == 1
	InitializeLCDTouch();

	// This is the orientation for the board to be directly up where the buttons are vertically above the screen
	// Top left would be low x value, high y value. Bottom right would be low x value, low y value.
	StaticTouchData.orientation = STMPE811_Orientation_Portrait_2;

	Button_Init();

    currentAppState = APP_STATE_MENU;
    drawMenuScreen();
    printf("Initial Menu Drawn. Entering Scheduler Loop.\n");
    printf("----------------------\n");

	#endif // COMPILE_TOUCH_FUNCTIONS
}

// Simple rectangle drawing
void LCD_Draw_Rect(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color) {
    // Draw horizontal lines
    for (uint16_t i = 0; i < width; i++) {
        LCD_Draw_Pixel(x + i, y, color);
        LCD_Draw_Pixel(x + i, y + height - 1, color);
    }
    // Draw vertical lines
    for (uint16_t i = 1; i < height - 1; i++) { // Avoid drawing corners twice
        LCD_Draw_Pixel(x, y + i, color);
        LCD_Draw_Pixel(x + width - 1, y + i, color);
    }
}

// Simple string display
void LCD_DisplayString(uint16_t Xpos, uint16_t Ypos, uint8_t *ptr, FONT_t* font, uint16_t textColor, uint16_t bgColor, bool opaque) {
    LCD_SetFont(font);
    LCD_SetTextColor(textColor);

    while (*ptr != '\0') {
        // Add bounds checking
        if (Xpos >= LCD_PIXEL_WIDTH - font->Width || Ypos >= LCD_PIXEL_HEIGHT - font->Height) {
            break;
        }
        LCD_DisplayChar(Xpos, Ypos, *ptr); // Assumes transparent background
        Xpos += font->Width;
        ptr++;
    }
}

void drawMenuScreen() {

		LCD_Clear(0, LCD_COLOR_BLUE); // Background

		// --- Draw Button 1 (1 Player) ---
		LCD_Draw_Rect(BUTTON1_X, BUTTON1_Y, BUTTON_WIDTH, BUTTON_HEIGHT, LCD_COLOR_YELLOW); // Outline
		LCD_DisplayString(BUTTON1_X + 5, BUTTON1_Y + (BUTTON_HEIGHT - Font16x24.Height)/2, (uint8_t*)"1P", &Font16x24, LCD_COLOR_WHITE, LCD_COLOR_BLUE, false);

		// --- Draw Button 2 (2 Player) ---
		LCD_Draw_Rect(BUTTON2_X, BUTTON2_Y, BUTTON_WIDTH, BUTTON_HEIGHT, LCD_COLOR_YELLOW); // Outline
		LCD_DisplayString(BUTTON2_X + 5, BUTTON2_Y + (BUTTON_HEIGHT - Font16x24.Height)/2, (uint8_t*)"2P", &Font16x24, LCD_COLOR_WHITE, LCD_COLOR_BLUE, false);

		// --- Title ---
		LCD_DisplayString((LCD_PIXEL_WIDTH - 12*Font16x24.Width)/2, 30, (uint8_t*)"Connect Four", &Font16x24, LCD_COLOR_YELLOW, LCD_COLOR_BLUE, false); // Centered title

		//handleTouchInput();
		addSchedulerEvent(TOUCH_POLLING_EVENT);
}


uint16_t getSlotColor(SlotState_t state) {
    switch(state) {
        case SLOT_PLAYER1:
            return PLAYER1_COLOR;
        case SLOT_PLAYER2:
            return PLAYER2_COLOR;
        case SLOT_EMPTY:
        default:
            return EMPTY_SLOT_COLOR;
    }
}

void initializeGameBoard() {
    printf("Initializing Game Board...\n");
    for (int r = 0; r < BOARD_ROWS; r++) {
        for (int c = 0; c < BOARD_COLS; c++) {
            gameBoard[r][c] = SLOT_EMPTY;
        }
    }
}

void drawGameBoard() {
    printf("Drawing Game Board...\n");
    // 1. Clear the screen or draw a background
    LCD_Clear(0, BACKGROUND_COLOR); // Or use LCD_Fill_Rect for just the screen area

    // 2. Draw the main board structure (the blue rectangle)
    LCD_Fill_Rect(BOARD_X_OFFSET, BOARD_Y_OFFSET, BOARD_DRAW_WIDTH, BOARD_DRAW_HEIGHT, BOARD_COLOR);

    // 3. Draw the slots (circles) based on the gameBoard state
    for (int r = 0; r < BOARD_ROWS; r++) {
        for (int c = 0; c < BOARD_COLS; c++) {
            // Calculate the center coordinates of the circle for slot (r, c)
            uint16_t centerX = BOARD_X_OFFSET + BOARD_SPACING + CIRCLE_RADIUS + c * (CIRCLE_DIAMETER + BOARD_SPACING);
            uint16_t centerY = BOARD_Y_OFFSET + BOARD_SPACING + CIRCLE_RADIUS + r * (CIRCLE_DIAMETER + BOARD_SPACING);

            // Get the color for the current slot state
            uint16_t slotColor = getSlotColor(gameBoard[r][c]);

            // Draw the filled circle representing the slot/coin
            LCD_Draw_Circle_Fill(centerX, centerY, CIRCLE_RADIUS, slotColor);
        }
    }
     printf("Game Board Drawn.\n");
}

void drawGameOverScreen() {
    LCD_Clear(0, BACKGROUND_COLOR); // Background (Black)

    char message[40];
    uint16_t text_color;
    uint16_t text_x, text_y; // Variables for calculated positions

    // --- Display Winner/Tie Message ---
    LCD_SetFont(&Font16x24);
    if (roundWinner == SLOT_PLAYER1) {
        text_color = PLAYER1_COLOR; // Red
        sprintf(message, "Player 1 Wins!");
    } else if (roundWinner == SLOT_PLAYER2) {
        text_color = PLAYER2_COLOR; // Yellow
        sprintf(message, "%s Wins!", isOnePlayerMode ? "AI" : "Player 2");
    } else {
        text_color = LCD_COLOR_WHITE; // White for Tie
        sprintf(message, "It's a Tie!");
    }
    LCD_DisplayString((LCD_PIXEL_WIDTH - strlen(message) * Font16x24.Width) / 2, 60, (uint8_t*)message, &Font16x24, text_color, BACKGROUND_COLOR, false);



    // --- Display Scores ---
    LCD_SetFont(&Font12x12);
    text_color = LCD_COLOR_WHITE;

    sprintf(message, "P1:(%lu) - P2:(%lu)", player1Score, player2Score);
    text_x = (LCD_PIXEL_WIDTH - strlen(message) * Font12x12.Width) / 2; // Center horizontally
    text_y = 120; // Vertical position

    printf("Drawing Score at X=%d, Y=%d: %s\n", text_x, text_y, message); // Add debug print
    LCD_DisplayString(text_x, text_y, (uint8_t*)message, &Font12x12, text_color, BACKGROUND_COLOR, false);


    // --- Display Round Time ---
    uint32_t duration_ms = roundEndTime - roundStartTime;
    uint32_t duration_s = duration_ms / 1000;
    sprintf(message, "Time: %lu seconds", duration_s);
    text_x = (LCD_PIXEL_WIDTH - strlen(message) * Font12x12.Width) / 2; // Center horizontally
    text_y = 150; // Vertical position

    printf("Drawing Time at X=%d, Y=%d: %s\n", text_x, text_y, message); // Add debug print
    LCD_DisplayString(text_x, text_y, (uint8_t*)message, &Font12x12, text_color, BACKGROUND_COLOR, false);


    // --- Draw Restart Button ---
    LCD_Fill_Rect(RESTART_BUTTON_X, RESTART_BUTTON_Y, RESTART_BUTTON_WIDTH, RESTART_BUTTON_HEIGHT, LCD_COLOR_GREY);
    LCD_Draw_Rect(RESTART_BUTTON_X, RESTART_BUTTON_Y, RESTART_BUTTON_WIDTH, RESTART_BUTTON_HEIGHT, LCD_COLOR_WHITE); // Outline
    LCD_SetFont(&Font16x24);
    text_color = LCD_COLOR_WHITE; // White text for button
    sprintf(message, "Restart");
    text_x = RESTART_BUTTON_X + (RESTART_BUTTON_WIDTH - strlen(message) * Font16x24.Width) / 2;
    text_y = RESTART_BUTTON_Y + (RESTART_BUTTON_HEIGHT - Font16x24.Height) / 2;
    LCD_DisplayString(text_x, text_y, (uint8_t*)message, &Font16x24, text_color, LCD_COLOR_GREY, true); // Use opaque=true here since we filled bg
}

void drawPreviewCoin() {
    // 1. Calculate Y position (fixed, above the board)
    uint16_t previewCoinY = BOARD_Y_PREVIEW_OFFSET - BOARD_SPACING - CIRCLE_RADIUS;

    // 2. Calculate X position based on currentColumn
    uint16_t previewCoinX = BOARD_X_OFFSET + BOARD_SPACING + CIRCLE_RADIUS + currentColumn * (CIRCLE_DIAMETER + BOARD_SPACING);

    // 3. Erase previous coin position (draw background color circle)
    //    Only erase if the position actually changed
    if (previousPreviewCoinX != 0 && previousPreviewCoinX != previewCoinX) {
         LCD_Draw_Circle_Fill(previousPreviewCoinX, previewCoinY, CIRCLE_RADIUS + 1, BACKGROUND_COLOR); // +1 to cover edges
    }

    // 4. Draw the new coin with the current player's color
    uint16_t playerColor = getSlotColor(currentPlayer);
    LCD_Draw_Circle_Fill(previewCoinX, previewCoinY, CIRCLE_RADIUS, playerColor);

    // 5. Store current position for next erase
    previousPreviewCoinX = previewCoinX;
}

int findLowestEmptyRow(int col) {
    if (col < 0 || col >= BOARD_COLS) {
        return -1; // Invalid column
    }
    // Start from the bottom row and go up
    for (int r = BOARD_ROWS - 1; r >= 0; r--) {
        if (gameBoard[r][col] == SLOT_EMPTY) {
            return r; // Found an empty slot
        }
    }
    return -1; // Column is full
}

void dropCoin() {
    printf("Attempting to drop coin in column %d for player %d\n", currentColumn, currentPlayer);

    int row = findLowestEmptyRow(currentColumn);

    if (row != -1) { // Column has space
        printf("Placing coin at (%d, %d)\n", row, currentColumn);
        gameBoard[row][currentColumn] = currentPlayer;
        drawGameBoard(); // Redraw board with the new piece

        // --- Check for Win ---
        if (checkWin(currentPlayer)) {
            printf("Player %d Wins!\n", (currentPlayer == SLOT_PLAYER1) ? 1 : 2);
            roundWinner = currentPlayer;
            roundEndTime = HAL_GetTick(); // Record end time
            if (currentPlayer == SLOT_PLAYER1) player1Score++;
            else player2Score++;
            currentAppState = APP_STATE_GAME_OVER;
            drawGameOverScreen(); // Draw the game over screen immediately
            return; // End the turn sequence
        }

        // --- Check for Tie ---
        if (checkTie()) {
            printf("Game is a Tie!\n");
            roundWinner = SLOT_EMPTY; // Indicate a tie
            roundEndTime = HAL_GetTick(); // Record end time
            // No score update for a tie
            currentAppState = APP_STATE_GAME_OVER;
            drawGameOverScreen();
            return; // End the turn sequence
        }

        // --- No Win, No Tie -> Switch Player ---
        //SlotState_t previousPlayer = currentPlayer;
        if (isOnePlayerMode) {
            // Human just played (must be P1), switch to AI
            if (currentPlayer == SLOT_PLAYER1) {
                 currentPlayer = SLOT_PLAYER2; // AI is Player 2
                 currentAppState = APP_STATE_GAME_AI_TURN;
                 printf("Switched to AI turn.\n");
                 addSchedulerEvent(GAME_UPDATE_EVENT); // Trigger AI move
            } else { // AI just played (P2), switch to Human (P1)
                 currentPlayer = SLOT_PLAYER1;
                 currentAppState = APP_STATE_GAME_P1_TURN;
                 printf("Switched to Player 1 turn.\n");
            }
        } else { // Two Player Mode
             if (currentPlayer == SLOT_PLAYER1) {
                 currentPlayer = SLOT_PLAYER2;
                 currentAppState = APP_STATE_GAME_P2_TURN;
                 printf("Switched to Player 2 turn.\n");
             } else {
                 currentPlayer = SLOT_PLAYER1;
                 currentAppState = APP_STATE_GAME_P1_TURN;
                 printf("Switched to Player 1 turn.\n");
             }
        }

        // Erase the dropped preview coin and draw the new player's preview coin
        uint16_t previewCoinY = BOARD_Y_OFFSET - BOARD_SPACING - CIRCLE_RADIUS;
        // Use previous player's color to erase the spot where the coin *was*
        uint16_t eraseX = BOARD_X_OFFSET + BOARD_SPACING + CIRCLE_RADIUS + currentColumn * (CIRCLE_DIAMETER + BOARD_SPACING);
        LCD_Draw_Circle_Fill(eraseX, previewCoinY, CIRCLE_RADIUS + 1, BACKGROUND_COLOR);
        previousPreviewCoinX = 0; // Reset previous X
        if (currentAppState != APP_STATE_GAME_AI_TURN) { // Don't draw preview if AI is thinking immediately
             drawPreviewCoin();
        }

    } else {
        printf("Column %d is full!\n", currentColumn);
        // Optional feedback
    }
}


void handleTouchInput() {
	STMPE811_TouchData touchData;
	touchData.orientation = STMPE811_Orientation_Portrait_2;
    STMPE811_State_t touchState = returnTouchStateAndLocation(&touchData);
    if (touchState != STMPE811_State_Pressed) return;
    uint32_t currentTime = HAL_GetTick();
    if (currentTime - lastTouchMoveTime < TOUCH_MOVE_DEBOUNCE) return;
    lastTouchMoveTime = currentTime;

	// --- State Machine for Touch ---
    switch(currentAppState) {
    	case APP_STATE_MENU:
			// --- Start Timer on Game Start ---
			// Check Button 1 (Left Button - Should be 1 Player)
			if (isTouchInside(touchData.x, touchData.y, BUTTON1_X, BUTTON1_Y, BUTTON_WIDTH, BUTTON_HEIGHT)) {
				printf("Button 1 (1 Player) Selected!\n");
				isOnePlayerMode = true;
				initializeGameBoard();
				currentPlayer = SLOT_PLAYER1;
				currentColumn = BOARD_COLS / 2;
				roundStartTime = HAL_GetTick();
				currentAppState = APP_STATE_GAME_P1_TURN; // Start P1 turn (Human)
				drawGameBoard();
				drawPreviewCoin();
			}
			// Check Button 2 (Right Button - Should be 2 Player)
			else if (isTouchInside(touchData.x, touchData.y, BUTTON2_X, BUTTON2_Y, BUTTON_WIDTH, BUTTON_HEIGHT)) {
				printf("Button 2 (2 Player) Selected!\n");
				isOnePlayerMode = false;
				initializeGameBoard();
				currentPlayer = SLOT_PLAYER1;
				currentColumn = BOARD_COLS / 2;
				roundStartTime = HAL_GetTick();
				currentAppState = APP_STATE_GAME_P1_TURN; // Start P1 turn
				drawGameBoard();
				drawPreviewCoin();
			}
			break; // End of APP_STATE_MENU case

        case APP_STATE_GAME_P1_TURN:
        case APP_STATE_GAME_P2_TURN:
             // Only handle touch for moving the preview coin in 2P mode
             // or if it's P1's turn in 1P mode
             if (!isOnePlayerMode || currentPlayer == SLOT_PLAYER1) {
                  handleGameTouchInput(touchData.x, touchData.y);
             }
             break;

        case APP_STATE_GAME_OVER:
             // --- Handle Restart Button ---
             if (isTouchInside(touchData.x, touchData.y, RESTART_BUTTON_X, RESTART_BUTTON_Y, RESTART_BUTTON_WIDTH, RESTART_BUTTON_HEIGHT)) {
                  printf("Restart Button Pressed.\n");
                  currentAppState = APP_STATE_MENU;
                  // No need to reset scores here, they persist
                  drawMenuScreen(); // Go back to main menu
             }
             break;

        default:
            break;
    }
	    //addSchedulerEvent(TOUCH_POLLING_EVENT);
	     // Clear FIFO after processing a press that was acted upon
	     //I2C3_Write(STMPE811_ADDRESS, STMPE811_FIFO_STA, 0x01);
	     //I2C3_Write(STMPE811_ADDRESS, STMPE811_FIFO_STA, 0x00);
}



// Check if touch coordinates are within a button's bounds
bool isTouchInside(uint16_t touchX, uint16_t touchY, uint16_t btnX, uint16_t btnY, uint16_t btnW, uint16_t btnH) {
    return (touchX >= btnX && touchX < (btnX + btnW) &&
            touchY >= btnY && touchY < (btnY + btnH));
}

void handleGameTouchInput(uint16_t touchX, uint16_t touchY) {
     // Check if touch is on the left half of the screen
     if (touchX < LCD_PIXEL_WIDTH / 2) {
          // Move Left
          if (currentColumn > 0) {
                currentColumn--;
                printf("Preview Coin Moved Left to Column %d\n", currentColumn);
                drawPreviewCoin(); // Redraw preview coin in new position
          }
     }
     // Check if touch is on the right half of the screen
     else {
          // Move Right
          if (currentColumn < BOARD_COLS - 1) {
                currentColumn++;
                 printf("Preview Coin Moved Right to Column %d\n", currentColumn);
                drawPreviewCoin(); // Redraw preview coin in new position
          }
     }
}

void pollHardwareButton() {
    // Check if it's a player's turn state
    if (currentAppState != APP_STATE_GAME_P1_TURN && currentAppState != APP_STATE_GAME_P2_TURN) {
        return; // Only handle button presses during game turns
    }

    uint32_t currentTime = HAL_GetTick();

    // Read the button state (Assumes Button_IsPressed() uses HAL_GPIO_ReadPin)
    // Assumes button press pulls LOW (requires internal/external pull-up on PA0)
    if (Button_IsPressed()) { // Or directly: HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_RESET
    //if (HAL_GPIO_ReadPin(B1_GPIO_Port, B1_Pin) == GPIO_PIN_RESET) { // Check for LOW signal

        // Debounce check
        if (currentTime - lastButtonPressTime > BUTTON_PRESS_DEBOUNCE) {
            lastButtonPressTime = currentTime; // Update time of valid press
            printf("HW Button Pressed during game turn.\n");
            dropCoin(); // Trigger the coin drop logic
        }
    } else {
         // Button is not pressed (HIGH signal due to pull-up)
         // Can optionally reset debounce timer here if needed, but usually not necessary
         // lastButtonPressTime = 0; // Allow immediate press after release
    }
}

void handleAITurn() {
    if (currentAppState != APP_STATE_GAME_AI_TURN) {
        return; // Should not happen, but safety check
    }

    printf("AI Thinking...\n");
    // Add a small delay for realism?
    HAL_Delay(500); // Example: 500ms thinking time

    uint32_t random_val;
    int random_col;
    int available_row;
    int attempts = 0;
    const int max_attempts = 50; // Safety limit

    do {
        // Generate random number
        if (HAL_RNG_GenerateRandomNumber(&hrng, &random_val) != HAL_OK) {
             printf("RNG Error!\n");
             // Default to a safe move? (e.g., first available column)
             random_col = 0; // Fallback, find first valid column below
             break;
        }
        random_col = random_val % BOARD_COLS; // Get column 0-6

        // Check if this column is valid
        available_row = findLowestEmptyRow(random_col);

        attempts++;
        if (attempts > max_attempts) {
             printf("AI Timeout finding random move, trying linear scan.\n");
             // Fallback: Find the first available column linearly
             for (random_col = 0; random_col < BOARD_COLS; random_col++) {
                  available_row = findLowestEmptyRow(random_col);
                  if (available_row != -1) break; // Found one
             }
             // If still no valid column, something is wrong (should have tied)
             if (available_row == -1) {
                  printf("AI Error: No valid moves found on full(?) board.\n");
                  // Maybe force a tie state?
                  currentAppState = APP_STATE_GAME_OVER;
                  roundWinner = SLOT_EMPTY;
                  roundEndTime = HAL_GetTick();
                  drawGameOverScreen();
                  return;
             }
             break; // Exit loop with linearly found column
        }

    } while (available_row == -1); // Keep trying if random column was full

    // We have a valid column (random_col)
    currentColumn = random_col; // Set the column the AI will drop into
    printf("AI chooses column %d\n", currentColumn);
    dropCoin(); // Execute the drop
}

bool checkWin(SlotState_t player) {
    // Check horizontal win
    for (int r = 0; r < BOARD_ROWS; r++) {
        for (int c = 0; c <= BOARD_COLS - 4; c++) {
            if (gameBoard[r][c] == player &&
                gameBoard[r][c+1] == player &&
                gameBoard[r][c+2] == player &&
                gameBoard[r][c+3] == player) {
                return true;
            }
        }
    }

    // Check vertical win
    for (int r = 0; r <= BOARD_ROWS - 4; r++) {
        for (int c = 0; c < BOARD_COLS; c++) {
            if (gameBoard[r][c] == player &&
                gameBoard[r+1][c] == player &&
                gameBoard[r+2][c] == player &&
                gameBoard[r+3][c] == player) {
                return true;
            }
        }
    }

    // Check diagonal win (down-right)
    for (int r = 0; r <= BOARD_ROWS - 4; r++) {
        for (int c = 0; c <= BOARD_COLS - 4; c++) {
            if (gameBoard[r][c] == player &&
                gameBoard[r+1][c+1] == player &&
                gameBoard[r+2][c+2] == player &&
                gameBoard[r+3][c+3] == player) {
                return true;
            }
        }
    }

    // Check diagonal win (up-right)
    for (int r = 3; r < BOARD_ROWS; r++) {
        for (int c = 0; c <= BOARD_COLS - 4; c++) {
            if (gameBoard[r][c] == player &&
                gameBoard[r-1][c+1] == player &&
                gameBoard[r-2][c+2] == player &&
                gameBoard[r-3][c+3] == player) {
                return true;
            }
        }
    }

    return false; // No win condition found
}

bool checkTie(void) {
    // If any slot in the top row is empty, the board isn't full
    for (int c = 0; c < BOARD_COLS; c++) {
        if (gameBoard[0][c] == SLOT_EMPTY) {
            return false;
        }
    }
    // If we get here, the top row is full, so the board is full
    return true;
}

//void LCD_Visual_Demo(void)
//{
//	visualDemo();
//}
//
//void LCD_Start_Menu(void) {
//	startMenu();
//}

//#if COMPILE_TOUCH_FUNCTIONS == 1
//void LCD_Touch_Polling_Demo(void)
//{
//	LCD_Clear(0,LCD_COLOR_GREEN);
//	while (1) {
//		/* If touch pressed */
//		if (returnTouchStateAndLocation(&StaticTouchData) == STMPE811_State_Pressed) {
//			/* Touch valid */
//			printf("\nX: %03d\nY: %03d\n", StaticTouchData.x, StaticTouchData.y);
//			LCD_Clear(0, LCD_COLOR_RED);
//		} else {
//			/* Touch not pressed */
//			printf("Not Pressed\n\n");
//			LCD_Clear(0, LCD_COLOR_GREEN);
//		}
//	}
//}
//#endif // COMPILE_TOUCH_FUNCTIONS

