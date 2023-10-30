#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "motor.h"

#define ENCODEROUT_PIN 2
#define ENCODEROUT_PIN2 3
#define DISTANCE_BETWEEN_NOTCHES_CM 1.0525

uint32_t time_of_prev_notch[2] = {0, 0};
__long_double_t distance[2] = {0, 0};
double speed[2] = {0, 0};
__long_double_t prev_distance[2] = {0, 0};

void calculate_speed(uint32_t time_of_notch, int encoder_number);
void gpio_callback(uint gpio, uint32_t events);
bool check_wheel_moving(struct repeating_timer *t);

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

/*
GPIO interrupt callback function
Called when the START pseudo-button is pressed or released
*/
void gpio_callback(uint gpio, uint32_t events)
{
    if ((gpio == ENCODEROUT_PIN || gpio == ENCODEROUT_PIN2) && events == GPIO_IRQ_EDGE_RISE)
    {
        calculate_speed(time_us_32(), gpio);
    }
}

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
    struct repeating_timer timer;
    add_repeating_timer_ms(-50, &check_wheel_moving, NULL, &timer);

    while (true)
    {
        printf("Left Motor Speed: %lfcm/s\nRight Motor Speed: %lfcm/s\n", speed[0], speed[1]);

        sleep_ms(300);

        // Clear screen
        printf("\033[2J");

    }

    return 0;
}