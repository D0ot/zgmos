cmake_minimum_required(VERSION 3.1)
project(zgmos)

cmake_policy(SET CMP0054 NEW)

set_property(GLOBAL PROPERTY PROJ_ROOT "${CMAKE_CURRENT_LIST_DIR}")
get_property(proj_root GLOBAL PROPERTY PROJ_ROOT)
message(STATUS "Global property PROJ_ROOT is set to ${proj_root}")

include(${proj_root}/cmake/toolchain.cmake)

message(STATUS "${CMAKE_C_COMPILER}")





