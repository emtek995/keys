cmake_minimum_required(VERSION 3.13)

include_directories(${LLVM_INCLUDE_DIRS})

include(pico_sdk_import.cmake)

project(keys C CXX ASM)
set(CMAKE_C_STANDARD 17)
set(CMAKE_CXX_STANDARD 20)
pico_sdk_init()

add_executable(keys
  main.c
)

pico_enable_stdio_usb(keys 1)
pico_enable_stdio_uart(keys 1)

pico_add_extra_outputs(keys)

target_link_libraries(keys pico_stdlib)
