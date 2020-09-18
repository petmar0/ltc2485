project(loading)
cmake_minimum_required(VERSION 3.6)
set(CMAKE_MODULE_PATH $ENV{CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

include(Arduino)

enable_arduino_toolchain()

if(TARGET_M4)
  enable_m4_board(${TARGET_BOARD})
endif()

if(TARGET_M0)
  enable_m0()
endif()

file(GLOB sources *.cpp)

add_arduino_library(test "${sources}")

find_package(Wire)
target_link_libraries(test Wire)
target_compile_options(test PUBLIC -Wall -Werror)
add_arduino_firmware(test)
