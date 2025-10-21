#ifndef __DEFINITIONS_H__
#define __DEFINITIONS_H__

#include <stdio.h>
#include <SimpleLCD.h>
#include <SimpleUART.h>
#include <SimpleGPIO.h>
#include <SimplePWM.h>
#include <SimpleTimer.h>

// State machine stuff
enum State {
    ESP_ENERGIZED = 0,
    SAMPLE_PLACEMENT,
    JOYSTICK_UNLOCKED_1,
    MIX_PROCESS,
    VISCOSITY_MEASUREMENT,
    AVERAGE_CALC,
    JOYSTICK_UNLOCKED_2,
};

// Timer stuff
SimpleTimer timer1;
bool flag = false;
uint64_t dt_us = 10000; // 10 ms = 10000 us

// UART stuff
SimpleUART LabVIEW(115200);
char buffer[30];int message_length;

// GPIO stuff
SimpleGPIO XD;

#endif // __DEFINITIONS_H__