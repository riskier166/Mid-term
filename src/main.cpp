#include "definitions.h"

enum Mode { AUTO_MODE, MANUAL_MODE };
enum State;
void changeState(State newState);
State currentState = CONTAINER_IDENTIFICATION;
Mode currentMode = AUTO_MODE;

static void IRAM_ATTR timerISR(void *arg);

extern "C" void app_main(void)
{
    timer1.setup(timerISR, "MainTimer");
    timer1.startPeriodic(dt_us);
    colorSens.setup(gpo_pin, gpi_pin);
    Stp_motor.setup(step_pin, enable_pin, dir_buttons[]);
    Bottle_Sensor.setup(22,GPI);
    Viscosimeter.setup(motor_pwm_pin,motor_pwm_channel);
    buzzer.setup(15,GPO);
    manualButton.setup(25, GPI);
    modeSwitch.setup(26, GPI);

    if (timer1.interruptAvailable())
    {
        if (modeSwitch.get() == 1) currentMode = MANUAL_MODE; else currentMode = AUTO_MODE;

        switch (currentState)
        {
        case CONTAINER_IDENTIFICATION:
            if (currentMode == MANUAL_MODE && !manualButton.get()) break;
            color = colorSens.read();
            switch (color)
            {
            case RED:
                changeState(TURRET_ROUTINE);
                viscosity_value = 50;
                break;
            case GREEN:
                changeState(TURRET_ROUTINE);
                viscosity_value = 30;
                break;
            case BLUE:
                changeState(TURRET_ROUTINE);
                viscosity_value = 10;
                break;
            }
            break;

        case TURRET_ROUTINE:
            if (currentMode == MANUAL_MODE && !manualButton.get()) break;
            Stp_motor.calibrate(1000000);
            changeState(BOTTLE_ARRIVE);
            break;

        case BOTTLE_ARRIVE:
            if (currentMode == MANUAL_MODE && !manualButton.get()) break;
            if (Bottle_Sensor.get() == 1)
            {
                Stp_motor.begin();
                if (Stp_motor.getPosition() <= threshold)
                {
                    Stp_motor.stop();
                    changeState(VISCOSITY_READING);
                }
            }
            break;

        case VISCOSITY_READING:
            if (currentMode == MANUAL_MODE && !manualButton.get()) break;
            Viscosimeter.setSpeed(40.0);
            current_time = esp_timer_get_time();
            if(adjustment_flag == true){
                Viscosimeter.mix();
                adjustment_flag = false;
            }
            if (current_time - prev_time >= 5000000) {
                current_time = prev_time;
                for (int i = 0; i < viscosity_value; i++) {
                    visc_values[i] = Viscosimeter.read();
                }
            }
            for (int i = 0; i < viscosity_value; i++) {
                sampled_visc += visc_values[i];
            }
            sampled_visc /= viscosity_value;
            adjustment_flag = false;
            changeState(VISCOSITY_ADJUSTMENT);
            break;

        case VISCOSITY_ADJUSTMENT:
            if (currentMode == MANUAL_MODE && !manualButton.get()) break;
            if (sampled_visc > viscosity_value) {
                pump1.pumpIn(trigger_pin, 1000);
                adjustment_flag = true;
                changeState(VISCOSITY_READING);
            } else if (sampled_visc < viscosity_value) {
                buzzer.set(1);
            } else if (sampled_visc == viscosity_value) {
                buzzer.set(0);
                changeState(CLEANING_STATE);
            }
            break;

        case CLEANING_STATE:
            if (currentMode == MANUAL_MODE && !manualButton.get()) break;
            Stp_motor.clean();
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
