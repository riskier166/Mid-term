#ifndef __DEFINITIONS_H__
#define __DEFINITIONS_H__

#include <stdio.h>
#include <SimpleLCD.h>
#include <SimpleUART.h>
#include <SimpleGPIO.h>
#include <SimplePWM.h>
#include <SimpleTimer.h>
#include <ColorSensor.h>

// State machine stuff
enum State {
    CONTAINER_IDENTIFICATION = 0,
    TURRET_ROUTINE,
    BOTTLE_ARRIVE,
    VISCOSITY_ADJUSTMENT,
    CLEANING_STATE,
};

enum Color {
    RED=1,
    GREEN,
    BLUE,
};

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
SimpleGPIO XD;

#endif // __DEFINITIONS_H__