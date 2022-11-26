cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
project(PAD01_SCAN LANGUAGES CXX) 

## Get external content
include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG       v3.1.1
  )
FetchContent_MakeAvailable(Catch2)

FetchContent_Declare(
  Lyra
  GIT_REPOSITORY https://github.com/bfgroup/Lyra.git
  GIT_TAG       1.6.1
  )
FetchContent_MakeAvailable(Lyra)


## set global compiler options
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O3 -pthread")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)


## define targets

add_library(lib STATIC
  include/UARTdevice.hpp
  include/UARTdevice.cpp
  include/Adapter.hpp
  include/Adapter.cpp
  include/JTAGDevice.hpp
  include/JTAGDevice.cpp
  include/BitBangHandler.hpp
  include/BitBangHandler.cpp
  include/TCPServer.hpp
  include/TCPServer.cpp
  )
target_include_directories(lib PUBLIC include)

## Tests for JTAG-to-UART project.
add_executable(test
  test/test-main.cpp
  test/test.cpp
  )

target_compile_features(test PRIVATE cxx_std_20)
target_link_libraries(test PUBLIC lib PRIVATE Catch2::Catch2WithMain)
target_include_directories(test PRIVATE common)

## Test for UART-Interface configured to echo.
add_executable(hw_test
  test/hw_test-main.cpp
  test/hw_test.cpp
  )

target_compile_features(hw_test PRIVATE cxx_std_20)
target_link_libraries(hw_test PUBLIC lib PRIVATE Catch2::Catch2)
target_include_directories(hw_test PRIVATE common)

## JTAG-TO-UART main executable.
add_executable(jtaguart
  src/main.cpp
  )

target_compile_features(jtaguart PRIVATE cxx_std_20)
target_link_libraries(jtaguart PUBLIC lib PRIVATE bfg::lyra)
target_include_directories(jtaguart PRIVATE common)



# define target linkage


