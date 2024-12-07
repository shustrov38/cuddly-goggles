cmake_minimum_required(VERSION 3.5.0)
project(generator VERSION 0.1.0 LANGUAGES C CXX)

include(${CMAKE_CURRENT_LIST_DIR}/../../Common.cmake)

file(GLOB_RECURSE generator_src *.cpp *.h ../common/*.cpp)

add_executable(generator ${generator_src})

target_include_directories(generator PUBLIC ${CMAKE_CURRENT_LIST_DIR}/../../3rdparty/)
target_include_directories(generator PUBLIC ${CMAKE_CURRENT_LIST_DIR}/../common/)