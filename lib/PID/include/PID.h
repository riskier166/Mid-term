#ifndef _PID_H
#define _PID_H

#include <math.h>

class PID
{
public:
    PID();
    void setup(float gains[3],float dt_s);
    float calc(float error); // Measurement and Set

private:
float Kp, Ki, Kd=0.0f;
float dt;
float prev_error=0.0f;
float integral=0.0f;
};

#endif // _PID_H