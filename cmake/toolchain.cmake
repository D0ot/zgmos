message(STATUS "Checking RISCV toolchain")

if(NOT TOOLCHAIN)
  message(STATUS "TOOLCHAIN not set, searching in PATH...")
  find_program(FIND_RESULT "riscv64-unknown-elf-gcc")
  if("${FIND_RESULT}" STREQUAL "FIND_RESULT-NOTFOUND")
    message(FATAL_ERROR "TOOLCHAIN not set, searching in PATH failed, abort")
  else()
    get_filename_component(TOOLCHAIN "${FIND_RESULT}" DIRECTORY)
    message(STATUS "Using toolchain under ${TOOLCHAIN}")
  endif()
else()
  find_path(TOOLCHAIN_TMP NAMES "riscv64-unknown-elf-gcc" PATHS ${TOOLCHAIN})
  if("${TOOLCHAIN_TMP}" STREQUAL "TOOLCHAIN_TMP-NOTFOUND")
    message(FATAL_ERROR "TOOLCHAIN set, but riscv64-unknown-elf-gcc is not found under ${TOOLCHAIN}")
  else()
    message(STATUS "Using toolchain under ${TOOLCHAIN}")
  endif()
endif()

global_set(CMAKE_C_COMPILER "${TOOLCHAIN}/riscv64-unknown-elf-gcc")
global_set(CMAKE_CXX_COMPILER "${TOOLCHAIN}/riscv64-unknown-elf-g++")
global_set(CMAKE_LINKER "${TOOLCHAIN}/riscv64-unknown-elf-ld")
global_set(CMAKE_AR "${TOOLCHAIN}/riscv64-unknown-elf-ar")
global_set(CMAKE_OBJCOPY "${TOOLCHAIN}/riscv64-unknown-elf-objcopy")
global_set(CMAKE_SIZE "${TOOLCHAIN}/riscv64-unknown-elf-size")
global_set(CMAKE_OBJDUMP "${TOOLCHAIN}/riscv64-unknown-elf-objdump")
global_set(CMAKE_RANLIB "${TOOLCHAIN}/riscv64-unknown-elf-ranlib")

execute_process(COMMAND ${CMAKE_CXX_COMPILER} --print-file-name=crt0.o OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE crt0_obj)
execute_process(COMMAND ${CMAKE_CXX_COMPILER} --print-file-name=crtbegin.o OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE crtbegin_obj)
execute_process(COMMAND ${CMAKE_CXX_COMPILER} --print-file-name=crtend.o OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE crtend_obj)
execute_process(COMMAND ${CMAKE_CXX_COMPILER} --print-file-name=crti.o OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE crti_obj)
execute_process(COMMAND ${CMAKE_CXX_COMPILER} --print-file-name=crtn.o OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE crtn_obj)


