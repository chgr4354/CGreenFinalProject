/*
 * Scheduler.h
 *
 *  Created on: Jan 28, 2025
 *      Author: christiangreen
 */


#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#include <stdint.h>

#define NO_EVENT              (0)      // Indicate no events pending
#define TOUCH_POLLING_EVENT   (1 << 0) // Check for touchscreen input
#define HW_BUTTON_POLLING_EVENT (1 << 1) // Hardware button was pressed
#define GAME_UPDATE_EVENT     (1 << 2) // Time to update game logic
#define GYRO_READ_EVENT       (1 << 3) // Time to read gyro (if I use for extra credit)
#define RENDER_SCREEN_EVENT   (1 << 4) // Time to redraw parts or all of the screen
#define APP_DELAY_FLAG_EVENT  (1 << 5) // delay mechanism

uint32_t getScheduledEvents(void);
void addSchedulerEvent(uint32_t event);
void removeSchedulerEvent(uint32_t event);

#endif /* SCHEDULER_H_ */
