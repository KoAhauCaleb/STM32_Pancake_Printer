/*
 * Move.cpp
 *
 *  Created on: Oct 24, 2023
 *      Author: Caleb
 */

#include <stdbool.h>
#include "main.h"
#include "Movement.h"

#define X_STEPS_PER_MM 20 //6.5mm - 27.6
#define Y_STEPS_PER_MM 20 //23mm - 43.5
#define Z_STEPS_PER_MM 168

#define ST 1500//step time ticks 1400-1600

TIM_HandleTypeDef htim2;

float current_x = 0;
float current_y = 0;

float speed = 20; //mm/s

bool extrusion_on = false;

void delay(){
	for(int d = 0; d < ST; d++){}
}
void delayus(uint16_t us){
	__HAL_TIM_SET_COUNTER(&htim2,0);  // set the counter value a 0
	while (__HAL_TIM_GET_COUNTER(&htim2) < us);  // wait for the counter to reach the us input in the parameter
}

void set_timer(TIM_HandleTypeDef htim){
	htim2 = htim;
}

void set_speed(float new_speed){
	speed = new_speed;
}

void move(float x, float y){
	int x_steps = (x - current_x) * X_STEPS_PER_MM;
	int y_steps = (y - current_y) * Y_STEPS_PER_MM;
	HAL_GPIO_WritePin(X_DIR_GPIO_Port, X_DIR_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(Y_DIR_GPIO_Port, Y_DIR_Pin, GPIO_PIN_RESET);
	if(x_steps < 0){
		HAL_GPIO_WritePin(X_DIR_GPIO_Port, X_DIR_Pin, GPIO_PIN_SET);
		x_steps *= -1;
	}
	if(y_steps < 0){
		HAL_GPIO_WritePin(Y_DIR_GPIO_Port, Y_DIR_Pin, GPIO_PIN_SET);
		y_steps *= -1;
	}

	int max_steps = x_steps;
	if(y_steps > x_steps){
		max_steps = y_steps;
	}

	int x_step_ratio = max_steps / x_steps;
	int y_step_ratio = max_steps / y_steps;

	int x_taken = 0;
	int y_taken = 0;

	for(int i = 0; i < max_steps; i++){
		if(i % x_step_ratio == 0){
			HAL_GPIO_WritePin(X_STEP_GPIO_Port, X_STEP_Pin, GPIO_PIN_SET);
			x_taken++;

		}
		if(i % y_step_ratio == 0){
			HAL_GPIO_WritePin(Y_STEP_GPIO_Port, Y_STEP_Pin, GPIO_PIN_SET);
			y_taken++;
		}
		delay();
		HAL_GPIO_WritePin(X_STEP_GPIO_Port, X_STEP_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(Y_STEP_GPIO_Port, Y_STEP_Pin, GPIO_PIN_RESET);
		delayus(1000000 / (X_STEPS_PER_MM * speed)); // 1000000us/1s / (step/mm * mm/s)
	}

	while (x_steps > x_taken){
		HAL_GPIO_WritePin(X_STEP_GPIO_Port, X_STEP_Pin, GPIO_PIN_SET);
		delay();
		HAL_GPIO_WritePin(X_STEP_GPIO_Port, X_STEP_Pin, GPIO_PIN_RESET);
		x_taken++;
	}
	while (y_steps > y_taken){
		HAL_GPIO_WritePin(Y_STEP_GPIO_Port, Y_STEP_Pin, GPIO_PIN_SET);
		delay();
		HAL_GPIO_WritePin(Y_STEP_GPIO_Port, Y_STEP_Pin, GPIO_PIN_RESET);
		y_taken++;
	}
	current_x = x;
	current_y = y;
}


void home() {
	HAL_GPIO_WritePin(X_DIR_GPIO_Port, X_DIR_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(Y_DIR_GPIO_Port, Y_DIR_Pin, GPIO_PIN_SET);
	//move fast until limit switch hit
	while(!HAL_GPIO_ReadPin(X_STOP_GPIO_Port, X_STOP_Pin)){
		HAL_GPIO_WritePin(X_STEP_GPIO_Port, X_STEP_Pin, GPIO_PIN_SET);
		delay();
		HAL_GPIO_WritePin(X_STEP_GPIO_Port, X_STEP_Pin, GPIO_PIN_RESET);
		delayus(1000000 / (X_STEPS_PER_MM * speed));
	}
	while(!HAL_GPIO_ReadPin(Y_STOP_GPIO_Port, Y_STOP_Pin)){
		HAL_GPIO_WritePin(Y_STEP_GPIO_Port, Y_STEP_Pin, GPIO_PIN_SET);
		delay();
		HAL_GPIO_WritePin(Y_STEP_GPIO_Port, Y_STEP_Pin, GPIO_PIN_RESET);
		delayus(1000000 / (X_STEPS_PER_MM * speed));
	}

	//move away from limit switch by small distance
	move(10,10);

	//move slow towards limit switch
	HAL_GPIO_WritePin(X_DIR_GPIO_Port, X_DIR_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(Y_DIR_GPIO_Port, Y_DIR_Pin, GPIO_PIN_SET);
	//move fast until limit switch hit
	while(!HAL_GPIO_ReadPin(X_STOP_GPIO_Port, X_STOP_Pin)){
		HAL_GPIO_WritePin(X_STEP_GPIO_Port, X_STEP_Pin, GPIO_PIN_SET);
		delay();
		HAL_GPIO_WritePin(X_STEP_GPIO_Port, X_STEP_Pin, GPIO_PIN_RESET);
		delayus(1000000 / (X_STEPS_PER_MM * 10));

	}
	while(!HAL_GPIO_ReadPin(Y_STOP_GPIO_Port, Y_STOP_Pin)){
		HAL_GPIO_WritePin(Y_STEP_GPIO_Port, Y_STEP_Pin, GPIO_PIN_SET);
		delay();
		HAL_GPIO_WritePin(Y_STEP_GPIO_Port, Y_STEP_Pin, GPIO_PIN_RESET);
		delayus(1000000 / (X_STEPS_PER_MM * 10));
	}
	current_x = 0;
	current_y = 0;
}

void start_extrusion(){
	extrusion_on = true;
}
void stop_extrusion(){
	extrusion_on = false;
}

