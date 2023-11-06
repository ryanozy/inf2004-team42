add_executable(wifi wifi.c wifi.h)

target_compile_definitions(wifi PRIVATE
WIFI_SSID=\"${WIFI_SSID}\"
WIFI_PASSWORD=\"${WIFI_PASSWORD}\"
)

target_include_directories(wifi PRIVATE
${CMAKE_CURRENT_LIST_DIR}
${CMAKE_CURRENT_LIST_DIR}/../.. # for our common lwipopts
)

target_link_libraries(wifi pico_stdlib pico_lwip_iperf pico_cyw43_arch_lwip_poll)

pico_add_extra_outputs(wifi)

pico_enable_stdio_usb(wifi 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(wifi)

# add url via pico_set_program_url
example_auto_set_url(wifi)