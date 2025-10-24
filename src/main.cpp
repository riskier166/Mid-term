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

    // Initialize the LCD
    lcd_init();

    // Clear the display
    lcd_clear();

    if (timer1.interruptAvailable())
    {
        switch (currentState)
        {
        case ESP_ENERGIZED:
            lcd_put_cursor(0, 0);
            lcd_send_string("Welcome, press button to start");
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
            else if (xbox.Pressed())
            {
                changeState(SAMPLE_PLACEMENT);
            }
            else
            {
                Step.setDuty(0);
            }
            
            break;
        case SAMPLE_PLACEMENT:
            lcd_clear();
            lcd_put_cursor(0, 0);
            lcd_send_string("Position is correct?");
            if (xbox.Pressed())
            {
                changeState(JOYSTICK_UNLOCKED_1);
            }
            break;
        case JOYSTICK_UNLOCKED_1:
            lcd_clear();
            lcd_put_cursor(0, 0);
            lcd_send_string("Positioning spindle...");

            lcd_put_cursor(1, 0);
            lcd_send_string("Press button to start mixing");
            if (xbox.Pressed())
            {
                changeState(MIX_PROCESS);
            }
            break;
        case MIX_PROCESS:
            lcd_clear();
            lcd_put_cursor(0, 0);
            lcd_send_string("Liquid stirring...");
            if (xbox.Pressed())
            {
                changeState(VISCOSITY_MEASUREMENT);
            }
            break;
        case VISCOSITY_MEASUREMENT:
            lcd_clear();
            lcd_put_cursor(0, 0);   
            lcd_send_string("Measuring viscosity...");
            if (xbox.Pressed())
            {
                changeState(AVERAGE_CALC);
            }
            break;
        case AVERAGE_CALC:
            lcd_clear();
            lcd_put_cursor(0, 0);
            lcd_send_string("Calculating average...");
            if (xbox.Pressed())
            {
                changeState(JOYSTICK_UNLOCKED_2);
            }
            break;
        case JOYSTICK_UNLOCKED_2:
            lcd_clear();
            lcd_put_cursor(0, 0);
            lcd_send_string("Joystick unlocked");
            if (xbox.Pressed())
            {
                changeState(ESP_ENERGIZED);
            }
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