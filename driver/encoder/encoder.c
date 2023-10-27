#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "motor.h"

#define ENCODEROUT_PIN 2
#define ENCODEROUT_PIN2 3
#define DISTANCE_BETWEEN_NOTCHES_CM 1.0525

uint32_t time_of_prev_notch = 0;
uint32_t time_of_prev_notch2 = 0;

__long_double_t distance = 0;
__long_double_t distance2 = 0;

void calculate_speed(uint32_t time_of_notch, int encoder_number);
void gpio_callback(uint gpio, uint32_t events);

void calculate_speed(uint32_t time_of_notch, int encoder_number)
{

    if (time_of_prev_notch == 0)
    {
        time_of_prev_notch = time_of_notch;
    } else if (time_of_prev_notch2 == 0)
    {
        time_of_prev_notch2 = time_of_notch;
    } else if (encoder_number == ENCODEROUT_PIN)
    {
        printf("Time of notch: %d\n", time_of_notch);
        printf("Time of prev notch: %d\n", time_of_prev_notch);

        float time_between_notches_ms = (time_of_notch - time_of_prev_notch) / 1000;
        printf("Time between notches: %2f ms\n", time_between_notches_ms);

        double speed = (DISTANCE_BETWEEN_NOTCHES_CM) / time_between_notches_ms;
        distance += DISTANCE_BETWEEN_NOTCHES_CM;
        printf("Speed: %lf cm/ms\n", speed);
        printf("Distance: %lf cm\n", distance);
        
        time_of_prev_notch = time_of_notch;
    } else if (encoder_number == ENCODEROUT_PIN2)
    {

        printf("Time of notch: %d\n", time_of_notch);
        printf("Time of prev notch: %d\n", time_of_prev_notch2);

        float time_between_notches_ms = (time_of_notch - time_of_prev_notch2) / 1000;
        printf("Time between notches: %2f us\n", time_between_notches_ms);

        double speed = (DISTANCE_BETWEEN_NOTCHES_CM) / time_between_notches_ms;
        distance2 += DISTANCE_BETWEEN_NOTCHES_CM;
        printf("Speed: %lf cm/ms\n", speed);
        printf("Distance: %lf cm\n", distance2);
        
        time_of_prev_notch2 = time_of_notch;
    }
}

/*
GPIO interrupt callback function
Called when the START pseudo-button is pressed or released
*/
void gpio_callback(uint gpio, uint32_t events)
{

    // Check if the START pseudo-button (GP15) was released (FALLING EDGE)
    if ((gpio == ENCODEROUT_PIN || gpio == ENCODEROUT_PIN2) && events == GPIO_IRQ_EDGE_RISE)
    {
        calculate_speed(time_us_32(), gpio);
    }

}

int main()
{
    stdio_init_all();

    // Set GP2 as input
    gpio_init(ENCODEROUT_PIN);

    gpio_set_dir(ENCODEROUT_PIN, GPIO_IN);

    // Init Motor
    start_motor(14, 15, 16, 17);

    // Init PWM
    start_motor_pwm(1, 0);

    move_forward(14, 15, 16, 17);

    gpio_set_irq_enabled_with_callback(ENCODEROUT_PIN, GPIO_IRQ_EDGE_RISE, true, &gpio_callback);

    while (true)
    {
    }

    return 0;
}