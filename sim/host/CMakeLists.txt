cmake_minimum_required(VERSION 3.16)

project(TheGripSim LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# ============================================================
# SDL3 - PREBUILT PACKAGE
# ============================================================

set(SDL3_DIR "${CMAKE_CURRENT_SOURCE_DIR}/SDL3/cmake")
find_package(SDL3 REQUIRED CONFIG)

# ============================================================
# libx86emu
# ============================================================

file(GLOB X86EMU_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/libx86emu/*.c"
)

add_library(x86emu STATIC
    ${X86EMU_SOURCES}
)

target_include_directories(x86emu PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/libx86emu/include"
)

# ============================================================
# Grip simulator
# ============================================================

add_executable(sim
    main.c
    display.c
    machine.c
    cpu.c
)

target_link_libraries(sim PRIVATE
    SDL3::SDL3
    x86emu
)

# Compiler warnings
if(MSVC)
    target_compile_options(sim PRIVATE /W4)

    target_compile_definitions(sim PRIVATE
        _CRT_SECURE_NO_WARNINGS
    )

    target_compile_definitions(x86emu PRIVATE
        _CRT_SECURE_NO_WARNINGS
    )
else()
    target_compile_options(sim PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()