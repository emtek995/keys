cmake_minimum_required(VERSION 3.13)

include(pico_sdk_import.cmake)

project(keys C CXX ASM)
set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)
set(PICO_BOARD pico_w)
set(PICO_CYW43_ARCH_DEFAULT_COUNTRY_CODE CYW43_COUNTRY_USA)
pico_sdk_init()

add_executable(keys
  main.c
)

pico_add_extra_outputs(keys)
pico_btstack_make_gatt_header(keys PRIVATE "${CMAKE_CURRENT_LIST_DIR}/keys.gatt")

target_link_libraries(keys
  pico_multicore
  pico_printf
  pico_stdlib
  pico_stdio_usb
  pico_btstack_ble
  pico_cyw43_arch
  pico_cyw43_arch_threadsafe_background
  pico_lwip_nosys
  pico_btstack_cyw43
)

target_include_directories(keys PRIVATE
  ${CMAKE_CURRENT_LIST_DIR}
)

