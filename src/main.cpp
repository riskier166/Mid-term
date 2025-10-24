#include "definitions.h"

// Machine state stuff
enum State;
void changeState(State newState);
State currentState = IDLE_POWERON;

// Simple edge-detection helpers for the xbox button with debounce.
// xboxPressedOnce() returns true only on the transition from not-pressed -> pressed
// and only if the last accepted edge was more than debounce time ago.
// xboxReleasedOnce() is the same but for release.
// These prevent holding the button or button bounce from triggering multiple state transitions.
static bool xboxPrevPressed = false;
static TickType_t xboxLastEdgeTicks = 0;
static const TickType_t xboxDebounceTicks = pdMS_TO_TICKS(50); // 50 ms debounce

static bool xboxPressedOnce()
{
    bool curr = xbox.Pressed();
    bool rising = (curr && !xboxPrevPressed);
    TickType_t now = xTaskGetTickCount();
    if (rising && ((now - xboxLastEdgeTicks) >= xboxDebounceTicks))
    {
        xboxLastEdgeTicks = now;
        xboxPrevPressed = curr;
        return true;
    }
    xboxPrevPressed = curr;
    return false;
}

static bool xboxReleasedOnce()
{
    bool curr = xbox.Pressed();
    bool falling = (!curr && xboxPrevPressed);
    TickType_t now = xTaskGetTickCount();
    if (falling && ((now - xboxLastEdgeTicks) >= xboxDebounceTicks))
    {
        xboxLastEdgeTicks = now;
        xboxPrevPressed = curr;
        return true;
    }
    xboxPrevPressed = curr;
    return false;
}

// Track when we entered the current state so we can do non-blocking delays
static TickType_t stateEnteredTicks = 0;

// Timer stuff
static void IRAM_ATTR timerISR(void *arg);

extern "C" void app_main(void)
{
    // Timer
    timer1.setup(timerISR, "MainTimer");
    timer1.startPeriodic(dt_us); // While timer

    // Joystick
    xbox.setup(x_pin, y_pin, sw_pin);
    xbox.calibrate(1000000); // 1 second

    // Stepper motor stuff
    Step.setup(step_pin, pwm_channel, &stepper_timer);
    Dir.setup(dir_pin, GPO);

    // Initialize the LCD
    lcd_init();

    // Clear the display
    lcd_clear();
    // initialize button state and state entry time
    // Read current physical button so we don't treat a held button at startup as a new press
    xboxPrevPressed = xbox.Pressed();
    xboxLastEdgeTicks = xTaskGetTickCount();
    changeState(currentState);

    while (true)
    {
        if (timer1.interruptAvailable())
        {
            switch (currentState)
            {
            case IDLE_POWERON:
                lcd_put_cursor(0, 0);
                lcd_send_string("Welcome, press");
                lcd_put_cursor(1, 0);
                lcd_send_string("button to start");

                //Move Spindle Home Motor OFF

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
                else if (xboxPressedOnce())
                {
                    changeState(SAMPLE_PLACEMENT);
                }
                else
                {
                    Step.setDuty(0);
                }

                //Condition Joystick button pressed --> next state
                break;
            case SAMPLE_PLACEMENT:
                lcd_clear();
                lcd_put_cursor(0, 0);
                lcd_send_string("Position is");
                lcd_put_cursor(1, 0);
                lcd_send_string("correct?");

                //Actions: Confirm Placement (Visually)

                if (xboxPressedOnce())
                {
                    changeState(SPINDLE_LOWERING);
                }
                break;
            case SPINDLE_LOWERING:
                // Non-blocking 3 second display of "Positioning spindle..."
                if ((xTaskGetTickCount() - stateEnteredTicks) < pdMS_TO_TICKS(3000))
                {
                    lcd_clear();
                    lcd_put_cursor(0, 0);
                    lcd_send_string("Positioning");
                    lcd_put_cursor(1, 0);
                    lcd_send_string("spindle...");

                    //Action Lower Spindle Linear Motor with Joystick down
                }
                else
                {
                    lcd_clear();
                    lcd_put_cursor(0, 0);
                    lcd_send_string("Press button to");
                    lcd_put_cursor(1, 0);
                    lcd_send_string("start mixing");
                    if (xboxPressedOnce())
                    {
                        changeState(STIRRING_PHASE);
                    }
                }
                break;
            case STIRRING_PHASE:
                lcd_clear();
                lcd_put_cursor(0, 0);
                lcd_send_string("Liquid");
                lcd_put_cursor(1, 0);
                lcd_send_string("stirring...");

                //Actions: Spin Spindle Motor 160RPM for 30 seconds

                //Condition Stirring Timer Ends (30sec)
                if (xboxPressedOnce())
                {
                    changeState(MEASUREMENT_PHASE);
                }
                break;
            case MEASUREMENT_PHASE:
                lcd_clear();
                lcd_put_cursor(0, 0);
                lcd_send_string("Measuring");
                lcd_put_cursor(1, 0);
                lcd_send_string("viscosity...");
                
                //Actions: Reduce Spindle RPM to 20-30 RPM 
                //Capture Torque/Current for 5 seconds 

                //Condition Measurement Timer Ends (5sec)
                if (xboxReleasedOnce())
                {
                    changeState(RESULT_DISPLAY);
                }
                break;
            case RESULT_DISPLAY:
                lcd_clear();
                lcd_put_cursor(0, 0);
                lcd_send_string("Calculating");
                lcd_put_cursor(1, 0);
                lcd_send_string("average...");

                //Actions: Calculating Average of the readings
                //Converting to Viscosity Value
                //Show Result on LCD

                //Condition Measurement done --> next state
                if (xboxPressedOnce())
                {
                    changeState(SAMPLE_REMOVAL);
                }
                break;
            case SAMPLE_REMOVAL:
                lcd_clear();
                lcd_put_cursor(0, 0);
                lcd_send_string("Take sample to");
                lcd_put_cursor(1, 0);
                lcd_send_string("cleaning station");

                //Actions: Move Spindle to Home Position with Joystick UP
                //Wait until the dripping is stopped (maybe 20sec)

                //Condition Spindle Home Position Reached
                //and Waiting until Dripping Timer Ends 
                if (xboxPressedOnce())
                {
                    changeState(CLEANING_STATION);
                }
                break;
            case CLEANING_STATION:
                //Action: Lower Spindle to Cleaning Station Joystick DOWN
                //Condition Reached Lowering Position (Button Pressed)
                lcd_clear();
                lcd_put_cursor(0, 0);
                lcd_send_string("Cleaning...");
                lcd_put_cursor(1, 0);
                lcd_send_string("1:00");
                //Condition: Cleaning Timer Ends (1 min)

                //Actions: Move Spindle to Home Position with Joystick UP

                //Condition Spindle Home Position Reached
                //Actions: Joystick to turn 
                
                //Condition Cleaning Positions Reached (Button Pressed)
                if (xboxPressedOnce())
                {
                    changeState(DRYING_STATION);
                }
                break;
                case DRYING_STATION:
                //Actions: Lower Spindle for Drying Station Joystick DOWN
                
                //Condition: Drying Position Reached (Button Pressed)
                //Actions: Turn Spindle_Motor 160RPM for 2 min
                lcd_clear();
                lcd_put_cursor(0, 0);
                lcd_send_string("Drying...");
                lcd_put_cursor(1, 0);
                lcd_send_string("2:00");

                //Condition Drying Timer Ends (2 min)
                //Actions: Move Spindle to Home Position with Joystick UP

                //Condition Spindle Home Position Reached
                //Actions: Joystick to turn 

                //Measurement Position Reached (Button Pressed)
                if (xboxPressedOnce())
                {
                    changeState(RESTART_READY);
                }
                break;
                case RESTART_READY:
                lcd_clear();
                lcd_put_cursor(0, 0);
                lcd_send_string("End of Process");
                lcd_put_cursor(1, 0);
                lcd_send_string("Ready to Restart");

                //Condition Restart Button Pressed
                if (xboxPressedOnce())
                {
                    changeState(IDLE_POWERON);
                }
                break;
            }
            
        }
    }
}
void changeState(State newState)
{
    currentState = newState;
    stateEnteredTicks = xTaskGetTickCount();
}

static void IRAM_ATTR timerISR(void *arg)
{
    timer1.setInterrupt();
}
