#ifndef __JOYSTICK_H__
#define __JOYSTICK_H__

#pragma once
#include <SimpleADC.h>
#include <esp_timer.h>
#include <esp_task_wdt.h>
#include <SimpleGPIO.h>

class Joystick
{
public:
    Joystick();
    void setup(int pinx, int piny, int pinBtn);
    bool calibrate(uint64_t timeMicros);

    bool Up();
    bool Down();
    bool Right();
    bool Left();
    bool Pressed();
    bool result();

private:
    SimpleADC _ejeX, _ejeY;
    SimpleGPIO _button;
    int readX();
    int readY();
    int _centerX, _centerY;
    int _deadZoneX, _deadZoneY;
    int _state;

};

#endif // __JOYSTICK_H__