#include "Joystick.h"

Joystick::Joystick()
{
    _centerX = _centerY = 2048;
    _deadZoneX = _deadZoneY = 400;
}

void Joystick::setup(int pinx, int piny, int pinBtn)
{
    _ejeX.setup(pinx);
    _ejeY.setup(piny);
    _button.setup(pinBtn, GPIO_MODE_INPUT, GPIO_PULLUP_ONLY);
}

bool Joystick::calibrate(uint64_t timeMicros)
{
    int64_t current = 0, prev = 0;
    int64_t begin = 0;

    int minX = 4095, maxX = 0;
    int minY = 4095, maxY = 0;
    int sumX = 0, sumY = 0, samples = 0;

    prev = esp_timer_get_time();
    begin = esp_timer_get_time();
    int stop = begin + timeMicros;

    while (current < stop)
    {
        current = esp_timer_get_time();
        if (current - prev >= 1000)
        {
            int valX = readX();
            int valY = readY();

            sumX += valX;
            sumY += valY;

            if (valX < minX)
                minX = valX;
            if (valX > maxX)
                maxX = valX;

            if (valY < minY)
                minY = valY;
            if (valY > maxY)
                maxY = valY;

            prev = current;
            samples++;
        }
    }
    if (samples == 0)
        return false;
    _centerX = sumX / samples;
    _centerY = sumY / samples;

    _deadZoneX = (maxX - minX) * 0.90;
    _deadZoneY = (maxY - minY) * 1.05;

    printf("Calibration: %d %d %d %d\n", _centerX, _centerY, _deadZoneX, _deadZoneY);
    return true;
}

int Joystick::readX()
{
    return _ejeX.read(ADC_READ_RAW);
}

int Joystick::readY()
{
    return _ejeY.read(ADC_READ_RAW);
}

bool Joystick::Up()
{
    return readY() < (_centerY - _deadZoneY);
}

bool Joystick::Down()
{
    return readY() > (_centerY + _deadZoneY);
}

bool Joystick::Right()
{
    return readX() > (_centerX + _deadZoneX);
}

bool Joystick::Left()
{
    return readX() < (_centerX - _deadZoneX);
}

bool Joystick::result()
{
  if (readX() > readY())
    {
        if (Right())
            printf("Right\n");
        else if (Up())
            printf("Up\n");


    }
    else if (readY() > readX())
    {
        if (Down())
            printf("Down\n");
        else if (Left())
            printf("Left\n");
    }
    return true;
}

bool Joystick::Pressed()
{
    return _button.get();
}
