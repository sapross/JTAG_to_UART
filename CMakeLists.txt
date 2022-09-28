cmake_minimum_required(VERSION 3.8 FATAL_ERROR)
project(PAD01_SCAN LANGUAGES CXX) 

## Get external content
include(FetchContent)
FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG       v2.13.7
  )
FetchContent_MakeAvailable(Catch2)

## set global compiler options
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -O3")
set(CMAKE_POSITION_INDEPENDENT_CODE ON)


## define targets

add_library(lib STATIC
  include/dummy.hpp
  include/dummy.cpp
  )
target_include_directories(lib PUBLIC include)

# add_executable(test
#   test/test-main.cpp
#   test/test.cpp
#   )

# target_compile_features(test PRIVATE cxx_std_20)
# target_link_libraries(test PUBLIC lib Catch2)
# target_include_directories(test PRIVATE common)

add_executable(jtaguart
  src/device.cpp
  )


target_compile_features(jtaguart PRIVATE cxx_std_20)
target_link_libraries(jtaguart PUBLIC lib)
target_include_directories(jtaguart PRIVATE common)


# define target linkage


