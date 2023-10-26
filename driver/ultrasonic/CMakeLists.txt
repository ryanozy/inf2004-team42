add_executable(cm ultrasonic.c ultrasonic_sensor.h)

# pull in common dependencies
target_link_libraries(cm
    pico_stdlib
    hardware_gpio
    hardware_dma
    hardware_timer
)


# enable usb output, enable uart output
pico_enable_stdio_usb(cm 1)
pico_enable_stdio_uart(cm 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(cm)

# add url via pico_set_program_url
example_auto_set_url(cm)
