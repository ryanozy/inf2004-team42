#include "ultrasonic_sensor.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/dma.h"
#include "hardware/timer.h"

#define TRIGGER_PIN 15
#define ECHO_PIN 14

//static const uint32_t MAX_DISTANCE_CM = 400; // Maximum distance in centimeters
int timeout = 26100;

void ultrasonic_init() {
    gpio_init(TRIGGER_PIN);
    gpio_init(ECHO_PIN);
    gpio_set_dir(TRIGGER_PIN, GPIO_OUT);
    gpio_set_dir(ECHO_PIN, GPIO_IN);

    // Initialize the Pico SDK
    stdio_init_all();
}

uint64_t getPulse()
{
    gpio_put(TRIGGER_PIN, 1);
    sleep_us(10);
    gpio_put(TRIGGER_PIN, 0);

    uint64_t width = 0;

    while (gpio_get(ECHO_PIN) == 0) tight_loop_contents();
    absolute_time_t startTime = get_absolute_time();
    while (gpio_get(ECHO_PIN) == 1) 
    {
        width++;
        sleep_us(1);
        if (width > timeout) {
            printf("width, %lld\n", width);
            return 0;
        }
    }
    absolute_time_t endTime = get_absolute_time();
    //printf("absolute time difference: %lld\n", absolute_time_diff_us(startTime, endTime));
    
    return absolute_time_diff_us(startTime, endTime);
}

uint64_t getCm(uint64_t pulseLength)
{
    return pulseLength / 29 / 2;
}


int main() {
    ultrasonic_init(); // Initialize ultrasonic sensor

    while (1) {
        uint32_t pulse_duration = getPulse(); // Measure the pulse duration
        uint32_t distance_cm = getCm(pulse_duration); // Measure distance in centimeters
        printf("Distance: %d cm\n", distance_cm); // Print the distance
        sleep_ms(1000); // Wait for 1 second before measuring again
    }

    return 0;
}