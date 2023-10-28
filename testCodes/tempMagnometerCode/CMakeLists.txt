add_executable(temp temp.c)

target_link_libraries(temp pico_stdlib hardware_i2c)

pico_enable_stdio_usb(temp 1)
pico_enable_stdio_uart(temp 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(temp)

# add url via pico_set_program_url
example_auto_set_url(temp)