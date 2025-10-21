#include "definitions.h"

// Machine state stuff
enum State;
void changeState(State newState);
State currentState = CONTAINER_IDENTIFICATION;

// Timer stuff
static void IRAM_ATTR timerISR(void *arg);

extern "C" void app_main(void)
{
    // INITIALIZATIONS
    timer1.setup(timerISR, "MainTimer");timer1.startPeriodic(dt_us);
    colorSens.setup(gpo_pin,gpi_pin);

    if (timer1.interruptAvailable())
    {
        switch (currentState)
        {
        case CONTAINER_IDENTIFICATION:
            color=colorSens.read();
            switch (color){
                case RED:
                    changeState(TURRET_ROUTINE);
                    viscosity_value=50;//example value
                    break;
                case GREEN:
                    changeState(TURRET_ROUTINE);
                    viscosity_value=30;//example value
                    break;
                case BLUE:
                    changeState(TURRET_ROUTINE);
                    viscosity_value=10;//example value
                    break;
            }
            switch (currentState)
            {
            case TURRET_ROUTINE:
                // Implement TURRET_ROUTINE behavior
                break;
            case BOTTLE_ARRIVE:
                // Implement BOTTLE_ARRIVE behavior
                break;
            case VISCOSITY_ADJUSTMENT:
                // Implement VISCOSITY_ADJUSTMENT behavior
                break;
            case CLEANING_STATE:
                // Implement CLEANING_STATE behavior
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