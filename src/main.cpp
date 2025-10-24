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

// Flag to track if LCD message has been displayed for current state
static bool lcdMessageDisplayed = false;
static bool lcdPhase2Displayed = false;

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
                if (!lcdMessageDisplayed)
                {
                    lcd_clear();
                    lcd_put_cursor(0, 0);
                    lcd_send_string("Welcome, press");
                    lcd_put_cursor(1, 0);
                    lcd_send_string("button to start");
                    lcdMessageDisplayed = true;
                }

                // Move Spindle Home Motor OFF

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

                // Condition Joystick button pressed --> next state
                break;
            case SAMPLE_PLACEMENT:
                if (!lcdMessageDisplayed)
                {
                    lcd_clear();
                    lcd_put_cursor(0, 0);
                    lcd_send_string("Position is");
                    lcd_put_cursor(1, 0);
                    lcd_send_string("correct?");
                    lcdMessageDisplayed = true;
                }

                // Actions: Confirm Placement (Visually)

                if (xboxPressedOnce())
                {
                    changeState(SPINDLE_LOWERING);
                }
                break;
            case SPINDLE_LOWERING:
                if (!lcdMessageDisplayed)
                {
                    lcd_clear();
                    lcd_put_cursor(0, 0);
                    lcd_send_string("Position spindle");
                    lcd_put_cursor(1, 0);
                    lcd_send_string("Up/Down");
                    lcdMessageDisplayed = true;
                }

                // Y-axis movement (up/down) for spindle positioning
                if (xbox.Up())
                {
                    // Move spindle up
                    Dir.set(1);
                    Step.setDuty(90.0);
                }
                else if (xbox.Down())
                {
                    // Move spindle down
                    Dir.set(0);
                    Step.setDuty(90.0);
                }
                else
                {
                    Step.setDuty(0);
                }

                // Accept position and proceed with button press
                if (xboxPressedOnce())
                {
                    changeState(STIRRING_PHASE);
                }
                break;
            case STIRRING_PHASE:
                // Non-blocking 30 second stirring phase
                if ((xTaskGetTickCount() - stateEnteredTicks) < pdMS_TO_TICKS(5000))
                {
                    if (!lcdMessageDisplayed)
                    {
                        lcd_clear();
                        lcd_put_cursor(0, 0);
                        lcd_send_string("Liquid");
                        lcd_put_cursor(1, 0);
                        lcd_send_string("stirring... (30s)");
                        lcdMessageDisplayed = true;
                    }
                    // Actions: Spin Spindle Motor 160RPM for 30 seconds
                }
                else
                {
                    if (!lcdPhase2Displayed)
                    {
                        lcd_clear();
                        lcd_put_cursor(0, 0);
                        lcd_send_string("Mixing Done");
                        lcd_put_cursor(1, 0);
                        lcd_send_string("Ready to Proceed");
                        lcdPhase2Displayed = true;
                    }

                    // Condition Button pressed to continue
                    if (xboxPressedOnce())
                    {
                        changeState(MEASUREMENT_PHASE);
                    }
                }
                break;
            case MEASUREMENT_PHASE:
                // Non-blocking 5 second measurement phase
                if ((xTaskGetTickCount() - stateEnteredTicks) < pdMS_TO_TICKS(5000))
                {
                    if (!lcdMessageDisplayed)
                    {
                        lcd_clear();
                        lcd_put_cursor(0, 0);
                        lcd_send_string("Measuring");
                        lcd_put_cursor(1, 0);
                        lcd_send_string("viscosity... (5s)");
                        lcdMessageDisplayed = true;
                    }

                    // Actions: Reduce Spindle RPM to 20-30 RPM
                    // Capture Torque/Current for 5 seconds
                }
                else
                {
                    changeState(MEASUREMENT_READY);
                }
                break;
            case MEASUREMENT_READY:
                if (!lcdMessageDisplayed)
                {
                    lcd_clear();
                    lcd_put_cursor(0, 0);
                    lcd_send_string("Measuring Done");
                    lcd_put_cursor(1, 0);
                    lcd_send_string("Ready to Proceed");
                    lcdMessageDisplayed = true;
                }

                // Condition Button pressed to continue
                if (xboxPressedOnce())
                {
                    changeState(RESULT_DISPLAY);
                }
                break;
            case RESULT_DISPLAY:
                if (!lcdMessageDisplayed)
                {
                    lcd_clear();
                    lcd_put_cursor(0, 0);
                    lcd_send_string("Calculating");
                    lcd_put_cursor(1, 0);
                    lcd_send_string("average...");
                    lcdMessageDisplayed = true;
                }

                // Actions: Calculating Average of the readings
                // Converting to Viscosity Value
                // Show Result on LCD

                // Condition Measurement done --> next state
                if (xboxPressedOnce())
                {
                    changeState(SAMPLE_REMOVAL);
                }
                break;
            case SAMPLE_REMOVAL:
                if (!lcdMessageDisplayed)
                {
                    lcd_clear();
                    lcd_put_cursor(0, 0);
                    lcd_send_string("Move spindle");
                    lcd_put_cursor(1, 0);
                    lcd_send_string("Up/Down");
                    lcdMessageDisplayed = true;
                }

                // Y-axis movement (up/down) for spindle positioning
                if (xbox.Up())
                {
                    // Move spindle up
                    Dir.set(1);
                    Step.setDuty(90.0);
                }
                else if (xbox.Down())
                {
                    // Move spindle down
                    Dir.set(0);
                    Step.setDuty(90.0);
                }
                else
                {
                    Step.setDuty(0);
                }

                // Condition Spindle Position Reached and proceed
                if (xboxPressedOnce())
                {
                    changeState(SAMPLE_X_MOVEMENT);
                }
                break;
            case SAMPLE_X_MOVEMENT:
                if (!lcdMessageDisplayed)
                {
                    lcd_clear();
                    lcd_put_cursor(0, 0);
                    lcd_send_string("Rotate Spindle");
                    lcd_put_cursor(1, 0);
                    lcd_send_string("Left/Right");
                    lcdMessageDisplayed = true;
                }

                // X-axis movement (left/right rotation)
                if (xbox.Left())
                {
                    // Rotate left
                    Dir.set(0);
                    Step.setDuty(90.0);
                }
                else if (xbox.Right())
                {
                    // Rotate right
                    Dir.set(1);
                    Step.setDuty(90.0);
                }
                else
                {
                    Step.setDuty(0);
                }

                if (xboxPressedOnce())
                {
                    changeState(SAMPLE_Y_MOVEMENT_2);
                }
                break;
            case SAMPLE_Y_MOVEMENT_2:
                if (!lcdMessageDisplayed)
                {
                    lcd_clear();
                    lcd_put_cursor(0, 0);
                    lcd_send_string("Move Spindle");
                    lcd_put_cursor(1, 0);
                    lcd_send_string("Up/Down");
                    lcdMessageDisplayed = true;
                }

                // Y-axis movement (up/down) again
                if (xbox.Up())
                {
                    // Move up
                    Dir.set(1);
                    Step.setDuty(90.0);
                }
                else if (xbox.Down())
                {
                    // Move down
                    Dir.set(0);
                    Step.setDuty(90.0);
                }
                else
                {
                    Step.setDuty(0);
                }

                if (xboxPressedOnce())
                {
                    changeState(CLEANING_STATION);
                }
                break;
            case CLEANING_STATION:
                // Non-blocking 1 minute cleaning phase
                if ((xTaskGetTickCount() - stateEnteredTicks) < pdMS_TO_TICKS(5000))
                {
                    if (!lcdMessageDisplayed)
                    {
                        lcd_clear();
                        lcd_put_cursor(0, 0);
                        lcd_send_string("Cleaning...");
                        lcd_put_cursor(1, 0);
                        lcd_send_string("(60s)");
                        lcdMessageDisplayed = true;
                    }
                    // Actions: Cleaning for 1 minute
                }
                else
                {
                    changeState(CLEANING_Y_MOVEMENT);
                }
                break;
            case CLEANING_Y_MOVEMENT:
                if (!lcdMessageDisplayed)
                {
                    lcd_clear();
                    lcd_put_cursor(0, 0);
                    lcd_send_string("Cleaning Done");
                    lcd_put_cursor(1, 0);
                    lcd_send_string("Move Up/Down");
                    lcdMessageDisplayed = true;
                }

                // Y-axis movement (up/down)
                if (xbox.Up())
                {
                    // Move up
                    Dir.set(1);
                    Step.setDuty(90.0);
                }
                else if (xbox.Down())
                {
                    // Move down
                    Dir.set(0);
                    Step.setDuty(90.0);
                }
                else
                {
                    Step.setDuty(0);
                }

                if (xboxPressedOnce())
                {
                    changeState(CLEANING_X_MOVEMENT);
                }
                break;
            case CLEANING_X_MOVEMENT:
                if (!lcdMessageDisplayed)
                {
                    lcd_clear();
                    lcd_put_cursor(0, 0);
                    lcd_send_string("Rotate Spindle");
                    lcd_put_cursor(1, 0);
                    lcd_send_string("Left/Right");
                    lcdMessageDisplayed = true;
                }

                // X-axis movement (left/right rotation)
                if (xbox.Left())
                {
                    // Rotate left
                    Dir.set(0);
                    Step.setDuty(90.0);
                }
                else if (xbox.Right())
                {
                    // Rotate right
                    Dir.set(1);
                    Step.setDuty(90.0);
                }
                else
                {
                    Step.setDuty(0);
                }

                if (xboxPressedOnce())
                {
                    changeState(CLEANING_Y_MOVEMENT_2);
                }
                break;
            case CLEANING_Y_MOVEMENT_2:
                lcd_clear();
                lcd_put_cursor(0, 0);
                lcd_send_string("Y-axis Movement");
                lcd_put_cursor(1, 0);
                lcd_send_string("Up/Down");

                // Y-axis movement (up/down) again
                if (xbox.Up())
                {
                    // Move up
                    Dir.set(1);
                    Step.setDuty(90.0);
                }
                else if (xbox.Down())
                {
                    // Move down
                    Dir.set(0);
                    Step.setDuty(90.0);
                }
                else
                {
                    Step.setDuty(0);
                }

                if (xboxPressedOnce())
                {
                    changeState(DRYING_STATION);
                }
                break;
            case DRYING_STATION:
                // Non-blocking 2 minute drying phase
                if ((xTaskGetTickCount() - stateEnteredTicks) < pdMS_TO_TICKS(5000))
                {
                    lcd_clear();
                    lcd_put_cursor(0, 0);
                    lcd_send_string("Drying...");
                    lcd_put_cursor(1, 0);
                    lcd_send_string("(120s)");
                    // Actions: Turn Spindle_Motor 160RPM for 2 min
                }
                else
                {
                    changeState(DRYING_Y_MOVEMENT);
                }
                break;
            case DRYING_Y_MOVEMENT:
                lcd_clear();
                lcd_put_cursor(0, 0);
                lcd_send_string("Drying Done");
                lcd_put_cursor(1, 0);
                lcd_send_string("Move Up/Down");

                // Y-axis movement (up/down)
                if (xbox.Up())
                {
                    // Move up
                    Dir.set(1);
                    Step.setDuty(90.0);
                }
                else if (xbox.Down())
                {
                    // Move down
                    Dir.set(0);
                    Step.setDuty(90.0);
                }
                else
                {
                    Step.setDuty(0);
                }

                if (xboxPressedOnce())
                {
                    changeState(DRYING_X_MOVEMENT);
                }
                break;
            case DRYING_X_MOVEMENT:
                lcd_clear();
                lcd_put_cursor(0, 0);
                lcd_send_string("Rotate Spindle");
                lcd_put_cursor(1, 0);
                lcd_send_string("Left/Right");

                // X-axis movement (left/right rotation)
                if (xbox.Left())
                {
                    // Rotate left
                    Dir.set(0);
                    Step.setDuty(90.0);
                }
                else if (xbox.Right())
                {
                    // Rotate right
                    Dir.set(1);
                    Step.setDuty(90.0);
                }
                else
                {
                    Step.setDuty(0);
                }

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

                // Condition Restart Button Pressed
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
    lcdMessageDisplayed = false; // Reset flag for new state
    lcdPhase2Displayed = false;  // Reset phase 2 flag
}

static void IRAM_ATTR timerISR(void *arg)
{
    timer1.setInterrupt();
}
