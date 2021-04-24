/**
 * Program skeleton for the course "Programming embedded systems"
 *
 * Lab 1: the elevator control system
 */

/**
 * This file defines the safety module, which observes the running
 * elevator system and is able to stop the elevator in critical
 * situations
 */

#include "FreeRTOS.h"
#include "task.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_gpio.h"
#include <stdio.h>
#include "global.h"
#include "assert.h"


#define POLL_TIME (10 / portTICK_RATE_MS)

#define MOTOR_UPWARD   (TIM3->CCR1)
#define MOTOR_DOWNWARD (TIM3->CCR2)
#define MOTOR_STOPPED  (!MOTOR_UPWARD && !MOTOR_DOWNWARD)
#define STOP_PRESSED  GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_3)
#define AT_FLOOR      GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_7)
#define DOORS_CLOSED  GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_8)

static portTickType xLastWakeTime;
	int speed_flag= -1,start=0,stpflag=0;
	s32 position1=0,position2=0,position3=0,speed;

static void check(u8 assertion, char *name) {
  if (!assertion) {
    printf("SAFETY REQUIREMENT %s VIOLATED: STOPPING ELEVATOR\n", name);
    for (;;) {
	  setCarMotorStopped(1);
  	  vTaskDelayUntil(&xLastWakeTime, POLL_TIME);
    }
  }
}

static void safetyTask(void *params) {
  s16 timeSinceStopPressed = -1;
	s16	timeSinceAtFloor = -1;
	s16 timeSinceDoorOpened = -1;

  xLastWakeTime = xTaskGetTickCount();
  for (;;) {
		
		
  // Environment assumption 1: The doors can only be opened if
	//                           the elevator is at a floor and
  //                           the motor is not active

	check((AT_FLOOR && MOTOR_STOPPED) || DOORS_CLOSED,
	       "env1");

		
	// Environment assumption 2: The elevator moves at a maximum speed of 50cm/s
		
	if(speed_flag == 0)
	{
		position2 = getCarPosition();
		speed_flag++;
	}
	else if(speed_flag == -1)
	{
		position1 = getCarPosition();
		speed_flag++;
	}
	if(speed_flag == 1)
		{
		if (position1>position2)
			speed=(position1-position2)/POLL_TIME;
		else if (position1<position2)
			speed=(position2-position1)/POLL_TIME;
		speed_flag=-1;
		}
		check(speed<=50, "env2");
	
	// Environment assumption 3: If the ground floor is put at 0cm in an absolute
	//  												 coordinate system, the second floor is at 400cm and
	//	 												 the third floor at 800cm (the at-floor sensor reports a
	//   												 floor with a threshold of +-0.5cm)
	

		if (AT_FLOOR && MOTOR_STOPPED){
			position3=getCarPosition();
			check(AT_FLOOR &&((-0.5 <= position3 && position3 <= 0.5) || (399.5 <= position3 && position3 <= 400.5) ||
							(799.5 <= position3 && position3 <= 800.5)), "env3");
		}	

		// Environment assumption 4: Motor is active only if the doors are closed
	
		if(!MOTOR_STOPPED)
		check(DOORS_CLOSED, "env4");
		
		// System requirement 1: if the stop button is pressed, the motor is stopped within 1s

	if (STOP_PRESSED) {
	  if (timeSinceStopPressed < 0)
	    timeSinceStopPressed = 0;
      else
	    timeSinceStopPressed += POLL_TIME;

      check(timeSinceStopPressed * portTICK_RATE_MS <= 1000 || MOTOR_STOPPED,
	        "req1");
	} else {
	  timeSinceStopPressed = -1;
	}

   // System requirement 2: the motor signals for upwards and downwards
	 //                       movement are not active at the same time

    check(!MOTOR_UPWARD || !MOTOR_DOWNWARD, "req2");

	// System requirement 3: The elevator may not pass the end positions,
	//  										 that is, go through the roof or the floor
	
	position3=getCarPosition();
	check((position3<=800.5) && (position3>=-0.5), "req3");	

	
	// System requirement 4: A moving elevator halts only if the stop button
	//											 is pressed or the elevator has arrived at a floor
	
if((MOTOR_STOPPED && STOP_PRESSED) && (MOTOR_STOPPED && AT_FLOOR))
	check(1,"req4");
else check(!(MOTOR_STOPPED && STOP_PRESSED) || !(MOTOR_STOPPED && AT_FLOOR), "req4");
	
	// System requirement 5: Once the elevator has stopped at a floor,
	//											 it will wait for at least 1s before it continues to another floor
	
	if ((AT_FLOOR && MOTOR_STOPPED) && start==1) {
	  if (timeSinceAtFloor < 0){
	    timeSinceAtFloor = 0;
			stpflag=1;
			}
     else
	    timeSinceAtFloor += POLL_TIME;
	} 
	else if((MOTOR_UPWARD || MOTOR_DOWNWARD) && stpflag==1){
		check(timeSinceAtFloor * portTICK_RATE_MS >= 1000, "req5");
		timeSinceAtFloor = -1;
		stpflag=0;
	}
		
	if((MOTOR_UPWARD || MOTOR_DOWNWARD) && start==0)
		start = 1;
	
			
	// System requirement 6: Door is open atleast for 1s before closing

	if (!DOORS_CLOSED) {
	  if (timeSinceDoorOpened < 0)
	    timeSinceDoorOpened = 0;
    else
	    timeSinceDoorOpened += POLL_TIME;
	}else if(DOORS_CLOSED){
  		check(timeSinceDoorOpened * portTICK_RATE_MS >= 1000 , "req6");
			timeSinceDoorOpened = -1;
		}
		vTaskDelayUntil(&xLastWakeTime, POLL_TIME);
  }
}
void setupSafety(unsigned portBASE_TYPE uxPriority) {
  xTaskCreate(safetyTask, "safety", 100, NULL, uxPriority, NULL);
}