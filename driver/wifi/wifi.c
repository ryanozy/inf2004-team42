#include <string.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "lwip/pbuf.h"
#include "lwip/tcp.h"
#include "wifi.h"

int main() {
    stdio_init_all();

    // Initialize the Wi-Fi driver
    if (wifi_init()) {
        cyw43_arch_deinit();
        return 1;
    }

    // Initialize the TCP server and open the connection
    TCP_SERVER_T *state = create_tcp_server();
    if (!state) {
        cyw43_arch_deinit();
        return 1;
    }

    while (1) {
        // The main loop
        cyw43_arch_poll(); // Poll for Wi-Fi driver or lwIP work
        sleep_ms(1000);
    }

    return 0;
}
