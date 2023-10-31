#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "motor.h"

// Encoder Pins
#define ENCODEROUT_PIN 2
#define ENCODEROUT_PIN2 3

// Distance between wheel notch in cm
#define DISTANCE_BETWEEN_NOTCHES_CM 1.0525

// PID Constants
#define KP 0.1
#define KI 10
#define KD 1

double integral_left = 0;
double prev_error_left = 0;
double derivative_left = 0;

double integral_right = 0;
double prev_error_right = 0;
double derivative_right = 0;

uint32_t time_of_prev_notch[2] = {0, 0};
__long_double_t distance[2] = {0, 0};
double speed[2] = {0, 0};
__long_double_t prev_distance[2] = {0, 0};

int desired_speed_cm_s = 36;

void calculate_speed(uint32_t time_of_notch, int encoder_number);
void gpio_callback(uint gpio, uint32_t events);
bool check_wheel_moving(struct repeating_timer *t);
bool pid(struct repeating_timer *t);

/**
 * @brief Main function to calculate speed of wheels.
 * 
 * This function is called on a interrupt when the encoder notch is detected. (Rising Edge)
 * It calculates the speed of the wheel and distance travelled, updating the global variables.
 * 
 */
void calculate_speed(uint32_t time_of_notch, int encoder_number)
{
    // If Encoder Pin is 2, index is 0
    // If Encoder Pin is 3, index is 1
    int index = encoder_number == ENCODEROUT_PIN ? 0 : 1;

    if (time_of_prev_notch[index] == 0)
    {
        time_of_prev_notch[index] = time_of_notch;
    }
    else
    {
        double time_between_notches_s = (time_of_notch - time_of_prev_notch[index]) / 1000000.0;
        speed[index] = (DISTANCE_BETWEEN_NOTCHES_CM) / time_between_notches_s;
        distance[index] += DISTANCE_BETWEEN_NOTCHES_CM;

        time_of_prev_notch[index] = time_of_notch;
    }
}

/**
 * @brief Callback function for GPIO interrupts.
 * 
 * Called when the encoder notch is detected. (Rising Edge)
 */
void gpio_callback(uint gpio, uint32_t events)
{
    if ((gpio == ENCODEROUT_PIN || gpio == ENCODEROUT_PIN2) && events == GPIO_IRQ_EDGE_RISE)
    {
        calculate_speed(time_us_32(), gpio);
    }
}

/**
 * @brief Repeated timer callback function
 * 
 * Called every 50ms to check if wheel is moving
 * Set speed to 0 if wheel is not moving
 */
bool check_wheel_moving(struct repeating_timer *t)
{
    // Check if distance not changed
    if (prev_distance[0] == distance[0])
    {
        speed[0] = 0.0;
    }

    if (prev_distance[1] == distance[1])
    {
        speed[1] = 0.0;
    }

    prev_distance[0] = distance[0];
    prev_distance[1] = distance[1];

    return true;
}

/**
 * @brief PID function
 * 
 * This function is called every 50ms to calculate the PID values for the left and right motors.
 * The PID values are then used to set the speed of the motors.
 * This function ensures motors are moving at the desired speed.
 */
bool pid(struct repeating_timer *t)
{
    double error_left = desired_speed_cm_s - speed[0];
    integral_left += error_left;
    derivative_left = error_left - prev_error_left;
    prev_error_left = error_left;

    double error_right = desired_speed_cm_s - speed[1];
    integral_right += error_right;
    derivative_right = error_right - prev_error_right;
    prev_error_right = error_right;

    double left_motor_speed = KP * error_left + KI * integral_left + KD * derivative_left;
    double right_motor_speed = KP * error_right + KI * integral_right + KD * derivative_right;

    pwm_set_gpio_level(15, left_motor_speed);
    pwm_set_gpio_level(10, right_motor_speed);

    return true;
}

int main()
{
    stdio_init_all();

    // Set GP2 and GP3 as input
    gpio_init(ENCODEROUT_PIN);
    gpio_init(ENCODEROUT_PIN2);

    gpio_set_dir(ENCODEROUT_PIN, GPIO_IN);
    gpio_set_dir(ENCODEROUT_PIN2, GPIO_IN);

    // Init Motor
    start_motor(14, 13, 12, 11);

    // Init PWM
    start_motor_pwm(15, 10);

    move_forward(14, 13, 12, 11);

    gpio_set_irq_enabled_with_callback(ENCODEROUT_PIN, GPIO_IRQ_EDGE_RISE, true, &gpio_callback);
    gpio_set_irq_enabled_with_callback(ENCODEROUT_PIN2, GPIO_IRQ_EDGE_RISE, true, &gpio_callback);

    // add repeating timer to check if wheel is moving
    struct repeating_timer speedtimer;
    add_repeating_timer_ms(-50, &check_wheel_moving, NULL, &speedtimer);

    struct repeating_timer pidtimer;
    add_repeating_timer_ms(-50, &pid, NULL, &pidtimer);

    while (true)
    {
        printf("Left Motor Speed: %lfcm/s\nRight Motor Speed: %lfcm/s\n", speed[0], speed[1]);

        sleep_ms(300);

        // Clear screen
        printf("\033[2J");

    }

    return 0;
}