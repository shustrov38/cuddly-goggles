cmake_minimum_required(VERSION 3.5.0)
project(solver VERSION 0.1.0 LANGUAGES C CXX)

include(${CMAKE_CURRENT_LIST_DIR}/../../Common.cmake)

find_package(Boost REQUIRED)

file(GLOB_RECURSE solver_src *.cpp *.h ../common/*.cpp)

add_executable(solver ${solver_src})

target_include_directories(solver PUBLIC ${CMAKE_CURRENT_LIST_DIR}/../common/)
target_link_libraries(solver PRIVATE 
    Boost::program_options
    Boost::graph
    Boost::heap
    Boost::timer
)