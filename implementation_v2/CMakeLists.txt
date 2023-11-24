add_executable(main_v2 main_v2.c 
                    infrared.h
                    magnometer.h
                    motor.h
                    ultrasonic_sensor.h
                    wifi.h)

target_link_libraries(main_v2 pico_stdlib 
                        hardware_i2c 
                        hardware_pwm 
                        hardware_timer
                        hardware_gpio
                        hardware_dma
                        pico_lwip_iperf
                        pico_cyw43_arch_lwip_threadsafe_background)

target_include_directories(main_v2 PRIVATE
${CMAKE_CURRENT_LIST_DIR}
${CMAKE_CURRENT_LIST_DIR}/../.. # for our common lwipopts
)

pico_enable_stdio_usb(main_v2 1)

pico_enable_stdio_uart(main_v2 1)

pico_add_extra_outputs(main_v2)

example_auto_set_url(main_v2)