set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

cmake_minimum_required(VERSION 3.1)

cmake_policy(SET CMP0054 NEW)

set_property(GLOBAL PROPERTY PROJ_ROOT "${CMAKE_CURRENT_LIST_DIR}")
get_property(proj_root GLOBAL PROPERTY PROJ_ROOT)
message(STATUS "Global property PROJ_ROOT is set to ${proj_root}")

include(${proj_root}/cmake/macros.cmake)
include(${proj_root}/cmake/toolchain.cmake)
include(${proj_root}/cmake/buildtype.cmake)

project(zgmos C ASM)

add_compile_flags(LD
        -nostartfiles
        -static
        -Wl,--gc-sections
        -Wl,-static
        -Wl,--start-group
        -Wl,--whole-archive
        -Wl,--no-whole-archive
        -Wl,--end-group
        -Wl,-EL
        -T ${proj_root}/lds/linker.ld
        )

# C Flags Settings
add_compile_flags(BOTH
        -mcmodel=medany
        -mabi=lp64d
        -march=rv64gc
        -nostdlib
        -ffreestanding
        -fno-common
        -ffunction-sections
        -fdata-sections
        -fstrict-volatile-bitfields
        -fno-zero-initialized-in-bss
        -ffast-math
        -fno-math-errno
        -fsingle-precision-constant
        -O0
        -ggdb
        -Wall
        -Werror=all)

add_compile_flags(C -std=gnu11 -Wno-pointer-to-int-cast)
include(${proj_root}/cmake/fix-9985.cmake)

add_executable(${PROJECT_NAME} "./kernel/entry.s" "./kernel/main.c")
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE C)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} --output-format=binary --strip-all ${CMAKE_BINARY_DIR}/${PROJECT_NAME} ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.bin
        DEPENDS ${PROJECT_NAME}
        COMMENT "Generating .bin file ...")
