/*
 * Scheduler.c
 *
 *  Created on: Jan 28, 2025
 *      Author: christiangreen
 */


#include "Scheduler.h"

static uint32_t scheduledEvents = 0;

uint32_t getScheduledEvents() {
	return scheduledEvents;
}

void addSchedulerEvent(uint32_t event) {
	scheduledEvents |= event;
}

void removeSchedulerEvent(uint32_t event) {
	scheduledEvents &= ~event;
}
