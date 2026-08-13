cmake_minimum_required(VERSION 3.16)

project(TheGripSim LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Your local prebuilt SDL3 package
set(SDL3_DIR "${CMAKE_CURRENT_SOURCE_DIR}/SDL3/cmake")

find_package(SDL3 REQUIRED CONFIG)

add_executable(sim
    main.c
    display.c
    machine.c
)

target_link_libraries(sim PRIVATE SDL3::SDL3)

target_compile_options(sim PRIVATE
    -Wall
    -Wextra
    -Wpedantic
)