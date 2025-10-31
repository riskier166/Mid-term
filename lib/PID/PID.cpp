#include "PID.h"

PID::PID()
{
}

void PID::setup(float gains[3], float dt_s)
{
    Kp = gains[0];
    Ki = gains[1];
    Kd = gains[2];
    dt = dt_s;
}

float PID::calc(float error)
{
    float U=Kp*error;
    U+=Kd*(error-prev_error)/dt;
    integral+=(dt/2)*(error+prev_error);
    U+=Ki*integral;
    prev_error=error;

    return U;

}
