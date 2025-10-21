#ifndef __DEFINITIONS_H__
#define __DEFINITIONS_H__

#include <stdio.h>
#include <SimpleLCD.h>
#include <SimpleUART.h>
#include <SimpleGPIO.h>
#include <SimplePWM.h>
#include <SimpleTimer.h>
#include <Viscosimeter.h>
#include <ColorSensor.h>
#include <Stepper.h>
#include <Water_pump.h>
#include "esp_timer.h"

// State machine stuff
enum State {
    CONTAINER_IDENTIFICATION = 0,
    TURRET_ROUTINE,
    BOTTLE_ARRIVE,
    VISCOSITY_READING,
    VISCOSITY_ADJUSTMENT,
    CLEANING_STATE,
};
enum Color {
    RED=1,
    GREEN,
    BLUE,
};
int threshold = 5000; // Example threshold value for stepper position
int visc_values[];
int sampled_visc;
bool adjustment_flag = false;

// Water pump stuff
Water_Pump pump1;
uitn8_t trigger_pin = 4; // Example GPIO pin for water pump trigger

// Viscosimeter motor stuff
Viscosimeter motorA;
uint8_t motor_pwm_pin = 25;     // Example GPIO pin for motor PWM
uint8_t motor_pwm_channel = 0;   // Example PWM channel for motor control

// Stepper stuff
Stepper Stp_motor;
uint8_t dir_buttons[2] = {18, 19}; // Example GPIO pins for direction buttons
uint8_t step_pin = 5;               // Example GPIO pin for step control
uint8_t enable_pin = 21;            // Example GPIO pin for enabling the stepper


// Timer stuff
SimpleTimer timer1;
bool flag = false;
uint64_t dt_us = 10000; // 10 ms = 10000 us

// Color sensor stuff
ColorSensor colorSens;
gpo_pin;gpi_pin;
int color; int viscosity_value;

// UART stuff
SimpleUART LabVIEW(115200);
char buffer[30];int message_length;

// GPIO stuff
SimpleGPIO Bottle_Sensor;
uint8_t bottle_sensor_pin = 22; // Example GPIO pin for bottle sensor
SimpleGPIO buzzer;

// First polling time
uint64_t prev_time = 0;
uint64_t current_time = 0;


#endif // __DEFINITIONS_H__