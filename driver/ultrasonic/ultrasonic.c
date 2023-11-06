#include "ultrasonic_sensor.h" // Include the ultrasonic sensor header file
#include <stdio.h> // Include standard input-output library
#include "pico/stdlib.h" // Include the Pico standard library
#include "hardware/gpio.h" // Include the GPIO hardware library
#include "hardware/timer.h" // Include the timer hardware library

#define TRIGGER_PIN 15 // Define the GPIO pin for the ultrasonic sensor trigger
#define ECHO_PIN 14 // Define the GPIO pin for the ultrasonic sensor echo

int main() {
    ultrasonic_init(TRIGGER_PIN, ECHO_PIN); // Initialize ultrasonic sensor

    while (1) {
        float pulse_duration = getPulseDuration(); // Measure the duration of the echo pulse
        float distance_cm = calculateDistance(pulse_duration); // Calculate distance in centimeters
        printf("Distance: %.3f cm\n", distance_cm); // Print the distance with three decimal places
        sleep_ms(1000); // Wait for 1 second before measuring again
    }

    return 0; // Return 0 to indicate successful execution
}
