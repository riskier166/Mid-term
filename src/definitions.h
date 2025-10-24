#ifndef __DEFINITIONS_H__
#define __DEFINITIONS_H__

#include <stdio.h>
#include <SimpleLCD.h>
#include <SimpleUART.h>
#include <SimpleGPIO.h>
#include <SimplePWM.h>
#include <SimpleTimer.h>
#include <Joystick.h>
#include <SimplePWM.h>

// LCD stuff
#pragma once
#ifdef __cplusplus
extern "C"
{
#endif

#include "i2c_lcd.h"

#ifdef __cplusplus
}
#endif

// I2C configuration
#define I2C_MASTER_SCL_IO 22
#define I2C_MASTER_SDA_IO 21
#define I2C_MASTER_FREQ_HZ 100000
#define LCD_I2C_ADDR 0x27
#define LCD_COLS 16
#define LCD_ROWS 2

// State machine stuff
enum State
{
    IDLE_POWERON = 0,
    SAMPLE_PLACEMENT,
    SPINDLE_LOWERING,
    STIRRING_PHASE,
    MEASUREMENT_PHASE,
    MEASUREMENT_READY,
    RESULT_DISPLAY,
    SAMPLE_REMOVAL,
    SAMPLE_X_MOVEMENT,
    SAMPLE_Y_MOVEMENT_2,
    CLEANING_STATION,
    CLEANING_Y_MOVEMENT,
    CLEANING_X_MOVEMENT,
    CLEANING_Y_MOVEMENT_2,
    DRYING_STATION,
    DRYING_Y_MOVEMENT,
    DRYING_X_MOVEMENT,
    RESTART_READY,
};

// Timer stuff
SimpleTimer timer1;
bool flag = false;
uint64_t dt_us = 10000; // 10 ms = 10000 us

// UART stuff
SimpleUART LabVIEW(115200);
char buffer[30];
int message_length;

// Joystick stuff
Joystick xbox;
const int sw_pin = 33, x_pin = 35, y_pin = 32;

// Stepper Stuff
SimplePWM Step;
SimpleGPIO Dir;
const uint8_t step_pin = 26, dir_pin = 25;
const uint8_t pwm_channel = 0;
TimerConfig stepper_timer{
    .timer = LEDC_TIMER_0,
    .frequency = 32000, // 30kHz
    .bit_resolution = LEDC_TIMER_10_BIT,
    .mode = LEDC_LOW_SPEED_MODE};

#endif // __DEFINITIONS_H__