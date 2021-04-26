if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE "Debug")
endif ()

if (CMAKE_BUILD_TYPE STREQUAL "Release")
  message(STATUS "build_type = Release")
else ()
  message(STATUS "build_type = Debug")
  add_definitions(-DDEBUG=1)
endif()

