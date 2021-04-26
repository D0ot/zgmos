message(STATUS "Checking RISCV toolchain")

if(NOT TOOLCHAIN)
  message(FATAL_ERROR "TOOLCHAIN not set, abort")
else()
  find_path(TOOLCHAIN_TMP NAMES "riscv64-unknown-elf-gcc" PATHS ${TOOLCHAIN})
  if("${TOOLCHAIN_TMP}" STREQUAL "TOOLCHAIN_TMP-NOTFOUND")
    message(FATAL_ERROR "TOOLCHAIN set, but riscv64-unknown-elf-gcc is not found under ${TOOLCHAIN}")
  else()
    message(STATUS "Using toolchain under ${TOOLCHAIN}")
  endif()
endif()

set(CMAKE_C_COMPILER "${TOOLCHAIN}/riscv64-unknown-elf-gcc")
set(CMAKE_CXX_COMPILER "${TOOLCHAIN}/riscv64-unknown-elf-g++")
set(CMAKE_LINKER "${TOOLCHAIN}/riscv64-unknown-elf-ld")
set(CMAKE_AR "${TOOLCHAIN}/riscv64-unknown-elf-ar")
set(CMAKE_OBJCOPY "${TOOLCHAIN}/riscv64-unknown-elf-objcopy")
set(CMAKE_SIZE "${TOOLCHAIN}/riscv64-unknown-elf-size")
set(CMAKE_OBJDUMP "${TOOLCHAIN}/riscv64-unknown-elf-objdump")
set(CMAKE_RANLIB "${TOOLCHAIN}/riscv64-unknown-elf-ranlib")

execute_process(COMMAND ${CMAKE_CXX_COMPILER} --print-file-name=crt0.o OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE crt0_obj)
execute_process(COMMAND ${CMAKE_CXX_COMPILER} --print-file-name=crtbegin.o OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE crtbegin_obj)
execute_process(COMMAND ${CMAKE_CXX_COMPILER} --print-file-name=crtend.o OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE crtend_obj)
execute_process(COMMAND ${CMAKE_CXX_COMPILER} --print-file-name=crti.o OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE crti_obj)
execute_process(COMMAND ${CMAKE_CXX_COMPILER} --print-file-name=crtn.o OUTPUT_STRIP_TRAILING_WHITESPACE OUTPUT_VARIABLE crtn_obj)


