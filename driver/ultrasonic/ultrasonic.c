#include "ultrasonic_sensor.h" // Include the ultrasonic sensor header file
#include <stdio.h> // Include standard input-output library
#include "pico/stdlib.h" // Include the Pico standard library
#include "hardware/gpio.h" // Include the GPIO hardware library
#include "hardware/timer.h" // Include the timer hardware library

int main() {
    ultrasonic_init(); // Initialize ultrasonic sensor

    while (1) {
        float pulse_duration = getPulseDuration(); // Measure the duration of the echo pulse
        float distance_cm = calculateDistance(pulse_duration); // Calculate distance in centimeters
        printf("Distance: %.3f cm\n", distance_cm); // Print the distance with three decimal places
        sleep_ms(1000); // Wait for 1 second before measuring again
    }

    return 0; // Return 0 to indicate successful execution
}
