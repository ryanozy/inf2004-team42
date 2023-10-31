add_subdirectory(encoder)
add_subdirectory(irline)
add_subdirectory(magnometer)
add_subdirectory(motor)
add_subdirectory(ultrasonic)
add_subdirectory(wifi)
add_subdirectory(pid_motor)

add_executable(driver driverDemo.c)

target_link_libraries(driver pico_stdlib pico_lwip_iperf pico_cyw43_arch_lwip_poll hardware_timer hardware_pwm hardware_gpio)

pico_enable_stdio_usb(driver 1)

pico_add_extra_outputs(driver)

