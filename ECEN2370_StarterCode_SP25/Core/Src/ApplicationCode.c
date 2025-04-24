/*
 * ApplicationCode.c
 *
 *  Created on: Dec 30, 2023 (updated 11/12/2024) Thanks Donavon! 
 *      Author: Xavion
 */

#include "ApplicationCode.h"

/* Static variables */


extern void initialise_monitor_handles(void); 

#if COMPILE_TOUCH_FUNCTIONS == 1
static ApplicationState_t currentAppState = APP_STATE_INIT;
static STMPE811_TouchData StaticTouchData;
static SlotState_t gameBoard[BOARD_ROWS][BOARD_COLS];
#endif // COMPILE_TOUCH_FUNCTIONS

void ApplicationInit(void)
{
	initialise_monitor_handles(); // Allows printf functionality
    LTCD__Init();
    LTCD_Layer_Init(0);
    LCD_Clear(0,LCD_COLOR_WHITE);

    #if COMPILE_TOUCH_FUNCTIONS == 1
	InitializeLCDTouch();

	// This is the orientation for the board to be direclty up where the buttons are vertically above the screen
	// Top left would be low x value, high y value. Bottom right would be low x value, low y value.
	StaticTouchData.orientation = STMPE811_Orientation_Portrait_2;

	Button_Interrupt_Init();

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
    // Background color setting would go here if opaque is true
    // This requires a fill rectangle function or modifying LCD_DisplayChar

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
            // Make sure LCD_Draw_Circle_Fill exists and works!
            LCD_Draw_Circle_Fill(centerX, centerY, CIRCLE_RADIUS, slotColor);
        }
    }
     printf("Game Board Drawn.\n");
}


void handleTouchInput() {
    //if (currentAppState != APP_STATE_MENU) {
    //    return; // Only handle touch in menu state for now
    //}
	STMPE811_TouchData touchData;
//	STMPE811_State_t touchState = returnTouchStateAndLocation(&touchData);


//	static bool wasReleased = true; // Basic Debounce: Only process if it was previously released

	while(1) {
		returnTouchStateAndLocation(&touchData);

		if (currentAppState == APP_STATE_MENU) {

			// Check Button 2
			if (isTouchInside(touchData.x, touchData.y, BUTTON1_X, BUTTON1_Y, BUTTON_WIDTH, BUTTON_HEIGHT)) {
				printf("Button 2 (2 Player) Selected!\n");
				currentAppState = APP_STATE_GAME_1P_PLAYER_TURN; // Transition state

				//LCD_Clear(0, LCD_COLOR_GREEN); // Placeholder
	            initializeGameBoard(); // Initialize board data
	            drawGameBoard();       // Draw the initial empty board
				addSchedulerEvent(TOUCH_POLLING_EVENT);
				return;
			}
			// Check Button 1
			else if (isTouchInside(touchData.x, touchData.y, BUTTON2_X, BUTTON2_Y, BUTTON_WIDTH, BUTTON_HEIGHT)) {
				printf("Button 1 (1 Player) Selected!\n");
				currentAppState = APP_STATE_GAME_1P_SETUP; // Transition state

				//LCD_Clear(0, LCD_COLOR_RED); // Placeholder
		        initializeGameBoard(); // Initialize board data
		        drawGameBoard();       // Draw the initial empty board
				addSchedulerEvent(TOUCH_POLLING_EVENT);
				return;
			}

			addSchedulerEvent(TOUCH_POLLING_EVENT);
		}  else if (currentAppState == APP_STATE_GAME_1P_PLAYER_TURN) {
	         // Handle touch input during the game (e.g., selecting column, dropping piece)
	         // This will be added later.
	    }  else if (currentAppState == APP_STATE_GAME_1P_AI_TURN) {
	         // Handle touch input during the game (e.g., selecting column, dropping piece)
	         // This will be added later.
	    }
	}
}

// Check if touch coordinates are within a button's bounds
bool isTouchInside(uint16_t touchX, uint16_t touchY, uint16_t btnX, uint16_t btnY, uint16_t btnW, uint16_t btnH) {
    return (touchX >= btnX && touchX < (btnX + btnW) &&
            touchY >= btnY && touchY < (btnY + btnH));
}

void handleHardwareButton() {
    // Simple debounce check
    static uint32_t last_hw_press_time = 0;
    uint32_t current_time = HAL_GetTick();

    if (current_time - last_hw_press_time > 300) { // 300ms debounce
        printf("Hardware Button Action Triggered!\n");
        last_hw_press_time = current_time;

        // --- Define action for HW button ---
        // Example: Reset to Menu screen if in game
        if (currentAppState != APP_STATE_MENU && currentAppState != APP_STATE_INIT) {
            printf("Placing coin\n");
            //currentAppState = APP_STATE_MENU;
            //drawMenuScreen(); // Redraw the menu
        }
         // Example: If already in menu, maybe toggle something? (less useful here)
         // else if (currentAppState == APP_STATE_MENU) { ... }
    } else {
         printf("Hardware Button Bounce Ignored.\n");
    }
}

void LCD_Visual_Demo(void)
{
	visualDemo();
}

void LCD_Start_Menu(void) {
	startMenu();
}

#if COMPILE_TOUCH_FUNCTIONS == 1
void LCD_Touch_Polling_Demo(void)
{
	LCD_Clear(0,LCD_COLOR_GREEN);
	while (1) {
		/* If touch pressed */
		if (returnTouchStateAndLocation(&StaticTouchData) == STMPE811_State_Pressed) {
			/* Touch valid */
			printf("\nX: %03d\nY: %03d\n", StaticTouchData.x, StaticTouchData.y);
			LCD_Clear(0, LCD_COLOR_RED);
		} else {
			/* Touch not pressed */
			printf("Not Pressed\n\n");
			LCD_Clear(0, LCD_COLOR_GREEN);
		}
	}
}
#endif // COMPILE_TOUCH_FUNCTIONS

