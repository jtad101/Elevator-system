/**
 * Program skeleton for the course "Programming embedded systems"
 *
 * Lab 1: the elevator control system
 */

/**
 * Class for keeping track of the car position.
 */

#include "FreeRTOS.h"
#include "task.h"
#include "position_tracker.h"
#include "assert.h"

int old_state = 0;
int new_state = 0;
portTickType xLastWakeTime;

static void positionTrackerTask(void *params) {

  PositionTracker *mytracker = (PositionTracker *)params ;

	for (;;) {
		xLastWakeTime = xTaskGetTickCount();
		new_state = GPIO_ReadInputDataBit(mytracker->gpio, mytracker->pin);
		if (new_state > old_state) {
			if(xSemaphoreTake(mytracker->lock, portMAX_DELAY)){
				if (mytracker->direction == Up) {
					mytracker->position++;
				}
				else if (mytracker->direction == Down) {
					mytracker->position--;
				}else if (mytracker->position == Unknown) {
			//REMAINS SAME	new_tracker->position = new_tracker->position 
				}
				printf("%ld\n",mytracker->position);
				xSemaphoreGive(mytracker->lock);
			}
			else printf("Semaphore failed \n");
    }
		old_state = new_state;
   vTaskDelayUntil(&xLastWakeTime, mytracker->pollingPeriod);
  } 
}

void setupPositionTracker(PositionTracker *tracker,
                          GPIO_TypeDef * gpio, u16 pin,
						  portTickType pollingPeriod,
						  unsigned portBASE_TYPE uxPriority) {
  portBASE_TYPE res;	

  tracker->position = 0;
  tracker->lock = xSemaphoreCreateMutex();
  assert(tracker->lock != NULL);
  tracker->direction = Unknown;
  tracker->gpio = gpio;
  tracker->pin = pin;
  tracker->pollingPeriod = pollingPeriod;

  res = xTaskCreate(positionTrackerTask, "position tracker",
                    80, (void*)tracker, uxPriority, NULL);
  assert(res == pdTRUE);
}

void setDirection(PositionTracker *tracker, Direction dir) {

	tracker->direction=dir;
	xSemaphoreGive(tracker->lock);
}

s32 getPosition(PositionTracker *tracker) {

  return tracker->position;

}

