//#include "include/main.hpp"

#include <cstdio>

#include <pico/cyw43_arch.h>
#include <pico/stdlib.h>
#include <pico/time.h>

#include "include/bluetooth.hpp"


static const int LED_PIN = CYW43_WL_GPIO_LED_PIN;

static auto hardware_init() noexcept -> void {
    stdio_init_all();
    cyw43_arch_init();
}

extern "C" [[noreturn]] int main() noexcept {
    hardware_init();

    [[maybe_unused]] auto bluetooth = new Bluetooth();


    while (true) {
        printf("blink ");
        cyw43_arch_gpio_put(LED_PIN, false);
        sleep_ms(250);
        cyw43_arch_gpio_put(LED_PIN, true);
        sleep_ms(250);
    }
}
