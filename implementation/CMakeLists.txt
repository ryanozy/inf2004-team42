add_executable(main main.c 
                    infrared.h
                    magnometer.h
                    motor.h
                    ultrasonic_sensor.h
                    wifi.h)

target_link_libraries(main pico_stdlib 
                        hardware_i2c 
                        hardware_pwm 
                        hardware_timer
                        hardware_gpio
                        hardware_dma
                        pico_lwip_iperf
                        pico_cyw43_arch_lwip_threadsafe_background)

target_include_directories(main PRIVATE
${CMAKE_CURRENT_LIST_DIR}
${CMAKE_CURRENT_LIST_DIR}/../.. # for our common lwipopts
)

pico_enable_stdio_usb(main 1)

pico_enable_stdio_uart(main 1)

pico_add_extra_outputs(main)

example_auto_set_url(main)