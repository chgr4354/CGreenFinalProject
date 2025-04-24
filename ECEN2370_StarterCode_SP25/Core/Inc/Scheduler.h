/*
 * Scheduler.h
 *
 *  Created on: Jan 28, 2025
 *      Author: christiangreen
 */

/*

#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include <stdint.h>

#define LED_TOGGLE_EVENT    (1 << 0)
#define DELAY_EVENT         (1 << 1)
#define BUTTON_POLLING_EVENT (1 << 2)
#define APP_DELAY_FLAG_EVENT (1 << 3)
#define DEVICE_ID_AND_TEMP_EVENT (1 << 4)

uint32_t getScheduledEvents();
void addSchedulerEvent(uint32_t event);
void removeSchedulerEvent(uint32_t event);

*/
//#endif /* SCHEDULER_H_ */


#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include <stdint.h>

// Define events relevant to Connect Four project
#define NO_EVENT              (0)      // Indicate no events pending
#define TOUCH_POLLING_EVENT   (1 << 0) // Check for touchscreen input
#define HW_BUTTON_PRESS_EVENT (1 << 1) // Hardware button was pressed (using interrupt)
#define GAME_UPDATE_EVENT     (1 << 2) // Time to update game logic (e.g., check win, AI move)
#define GYRO_READ_EVENT       (1 << 3) // Time to read gyro (if I use for extra credit)
#define RENDER_SCREEN_EVENT   (1 << 4) // Time to redraw parts or all of the screen
#define APP_DELAY_FLAG_EVENT  (1 << 5) // If using delay mechanism

uint32_t getScheduledEvents(void);
void addSchedulerEvent(uint32_t event);
void removeSchedulerEvent(uint32_t event);

#endif /* SCHEDULER_H_ */
