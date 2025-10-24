#include "definitions.h"

// Machine state stuff
enum State;
void changeState(State newState);
State currentState = ESP_ENERGIZED;

// Timer stuff
static void IRAM_ATTR timerISR(void *arg);

extern "C" void app_main(void)
{
    // Timer
    timer1.setup(timerISR, "MainTimer");timer1.startPeriodic(dt_us); // While timer

    // Joystick
    xbox.setup(x_pin, y_pin, sw_pin);xbox.calibrate(1000000); // 1 second

    // Stepper motor stuff
    Step.setup(step_pin, pwm_channel, &stepper_timer);Dir.setup(dir_pin, GPO);

    if (timer1.interruptAvailable())
    {
        switch (currentState)
        {
        case ESP_ENERGIZED:
            if (xbox.Right())
            {
                Dir.set(1);
                Step.setDuty(90.0);
            }
            else if (xbox.Left())
            {
                Dir.set(0);
                Step.setDuty(90.0);
            }
            else
            {
                Step.setDuty(0);
            }
            
            break;
        case SAMPLE_PLACEMENT:
            break;
        case JOYSTICK_UNLOCKED_1:
            break;
        case MIX_PROCESS:
            break;
        case VISCOSITY_MEASUREMENT:
            break;
        case AVERAGE_CALC:
            break;
        case JOYSTICK_UNLOCKED_2:
            
            break;
        }
    }
}

void changeState(State newState)
{
    currentState = newState;
}

static void IRAM_ATTR timerISR(void *arg)
{
    timer1.setInterrupt();
}