set(CMAKE_TRY_COMPILE_TARGET_TYPE "STATIC_LIBRARY")

cmake_minimum_required(VERSION 3.1)

cmake_policy(SET CMP0054 NEW)

set_property(GLOBAL PROPERTY PROJ_ROOT "${CMAKE_CURRENT_LIST_DIR}")
get_property(proj_root GLOBAL PROPERTY PROJ_ROOT)
message(STATUS "Global property PROJ_ROOT is set to ${proj_root}")

set(CMAKE_EXPORT_COMPILE_COMMANDS 1)
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
        -T ${proj_root}/lds/${LINKER_SCRIPT}
        )

# C Flags Settings
add_compile_flags(BOTH
        -mcmodel=medany
        -mabi=lp64f
        -march=rv64imafc
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
        -I${proj_root}/driver
        -I${proj_root}/kernel
        -ggdb
        -Wall)

add_compile_flags(C -std=gnu11 -Wno-pointer-to-int-cast)

if( QEMU ) 
  message(STATUS "QEMU is set, pass it to gcc")
  add_compile_flags(BOTH -DQEMU)
endif()

if( K210 )
  message(STATUS "K210 is set, pass it to gcc")
  add_compile_flags(BOTH -DK210)
endif()

include(${proj_root}/cmake/fix-9985.cmake)

file(GLOB source_files "${proj_root}/kernel/*.c" "${proj_root}/kernel/*.s" "${proj_root}/driver/*.c" "${proj_root}/hal/*.c" "${proj_root}/driver/kendryte/*.c")

add_executable(${PROJECT_NAME} ${source_files})
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE C)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_OBJCOPY} --output-format=binary --strip-all ${CMAKE_BINARY_DIR}/${PROJECT_NAME} ${CMAKE_BINARY_DIR}/${PROJECT_NAME}.bin
        DEPENDS ${PROJECT_NAME}
        COMMENT "Generating .bin file ...")

message(STATUS "Toolchain Infomation:")
message(STATUS "C compiler   : ${CMAKE_C_COMPILER}")
message(STATUS "CXX compiler : ${CMAKE_CXX_COMPILER}")
message(STATUS "LINKER       : ${CMAKE_LINKER}")
message(STATUS "C fLags      : ${CMAKE_C_FLAGS}")
message(STATUS "CXX flags    : ${CMAKE_CXX_FLAGS}")
