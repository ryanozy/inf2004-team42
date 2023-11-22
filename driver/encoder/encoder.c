#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "motor.h"

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

    // Move forward
    move_forward(14, 13, 12, 11);

    // Enable interrupts on rising edge
    gpio_set_irq_enabled_with_callback(ENCODEROUT_PIN, GPIO_IRQ_EDGE_RISE, true, &gpio_callback);
    gpio_set_irq_enabled_with_callback(ENCODEROUT_PIN2, GPIO_IRQ_EDGE_RISE, true, &gpio_callback);

    // add repeating timer to check if wheel is moving
    struct repeating_timer timer;
    add_repeating_timer_ms(-50, &check_wheel_moving, NULL, &timer);

    while (true)
    {
        printf("Left Motor Speed: %lfcm/s\nRight Motor Speed: %lfcm/s\n", speed[0], speed[1]);

        sleep_ms(1000);

        move_backward(14, 13, 12, 11);

        sleep_ms(1000);

        turn_left(14, 13, 12, 11);

        sleep_ms(1000);

        turn_right(14, 13, 12, 11);

        sleep_ms(1000);

        move_forward(14, 13, 12, 11);
        
        // For Loop for the speed control
        for (int i = 0; i < 10; i++)
        {
            set_speed(i * 10, 15, 10);
            sleep_ms(1000);
        }

        // Clear screen
        printf("\033[2J");

    }

    return 0;
}