/**
 * Program skeleton for the course "Programming embedded systems"
 *
 * Lab 1: the elevator control system
 */

/**
 * The planner module, which is responsible for consuming
 * pin/key events, and for deciding where the elevator
 * should go next
 */

#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>

#include "global.h"
#include "planner.h"
#include "assert.h"
#include "position_tracker.h"


#define MOTOR_UPWARD   (TIM3->CCR1)
#define MOTOR_DOWNWARD (TIM3->CCR2)
#define MOTOR_STOPPED  (!MOTOR_UPWARD && !MOTOR_DOWNWARD)
#define STOP_PRESSED  GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_3)
#define DOORS_CLOSED  GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_8)

static void plannerTask(void *params) {
	int floor1=0,floor2=0,floor3=0, dir, stp_flag, dist, startcar=0,inmotion=0;
	PinEvent event;
	portTickType xLastWakeTime;

for(;;){
	xLastWakeTime = xTaskGetTickCount();
	if(MOTOR_UPWARD)
		dir=1;
	else if (MOTOR_DOWNWARD)
		dir=0;
	dist=getCarPosition();
	xQueueReceive( pinEventQueue, &( event ), 50 / portTICK_RATE_MS);
	
	if(STOP_PRESSED)
	{
		printf("Stopped\n");
		setCarMotorStopped(1);
		stp_flag = 1;
	}
	else if(STOP_RELEASED && stp_flag)
	{
		printf("Released \n");
		stp_flag=0;
		setCarMotorStopped(0);
	}
	if (event == TO_FLOOR_1)
		{
			printf("Floor 1 \n");
			floor1 = 1;
		}
	
	else if (event == TO_FLOOR_2)
		{
				printf("Floor 2 \n");
				floor2=1;
		}
	
	else if (event == TO_FLOOR_3)
		{
			printf("Floor 3 \n");
			floor3=1;
		}
			
		if(MOTOR_STOPPED){
			if(dist >= -1 && dist <= 1 && DOORS_CLOSED && startcar==1) {
        printf("Reached floor 1\n");
				floor1=0;
				inmotion=0;
				vTaskDelay(1000/portTICK_RATE_MS);
      }
      if (dist >= 399 && dist <= 401 && DOORS_CLOSED) {
        printf("Reached floor 2\n");
				floor2=0;
				startcar=1;
				inmotion=0;
				vTaskDelay(1000/portTICK_RATE_MS);
      }
			
      if (dist >= 799 && dist <= 801 && DOORS_CLOSED) {
        printf("Reached floor 3\n");
        floor3=0;
				startcar=1;
				inmotion=0;
				vTaskDelay(1000/portTICK_RATE_MS);
      }
		}
		
		if(dir==1){
			if (floor2==1 && dist<400 && MOTOR_STOPPED && DOORS_CLOSED){
				setCarTargetPosition(400);
				inmotion=1;
			}
			else if (floor3==1 && MOTOR_STOPPED && DOORS_CLOSED){
				setCarTargetPosition(800);
				inmotion=1;
			}
		}
		else if(dir==0){
			if(floor2==1 && dist>400 && MOTOR_STOPPED && DOORS_CLOSED){
				setCarTargetPosition(400);
				inmotion=1;
			}
			else if (floor1==1 && MOTOR_STOPPED && DOORS_CLOSED){
				setCarTargetPosition(0);
				inmotion=1;
			}
		}
	
		if (floor1==1 && MOTOR_STOPPED && DOORS_CLOSED && inmotion==0){
			setCarTargetPosition(0);
		}
		
		else if (floor2==1 && MOTOR_STOPPED && DOORS_CLOSED && inmotion==0){
			setCarTargetPosition(400);	
		}
		
		else if (floor3==1 && MOTOR_STOPPED && DOORS_CLOSED && inmotion==0){
			setCarTargetPosition(800);
		}	
		vTaskDelayUntil(&xLastWakeTime, 10 / portTICK_RATE_MS);
}
}
void setupPlanner(unsigned portBASE_TYPE uxPriority) {
  xTaskCreate(plannerTask, "planner", 100, NULL, uxPriority, NULL);
}