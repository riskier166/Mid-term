#include "definitions.h"

// Machine state stuff
enum State;
void changeState(State newState);
State currentState = ESP_ENERGIZED;

// Timer stuff
static void IRAM_ATTR timerISR(void *arg);

extern "C" void app_main(void)
{
    // INITIALIZATIONS
    timer1.setup(timerISR, "MainTimer");timer1.startPeriodic(dt_us);

    if (timer1.interruptAvailable())
    {
        switch (currentState)
        {
        case ESP_ENERGIZED:

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