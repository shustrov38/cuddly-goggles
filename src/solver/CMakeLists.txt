cmake_minimum_required(VERSION 3.5.0)
project(solver VERSION 0.1.0 LANGUAGES C CXX)

include(${CMAKE_CURRENT_LIST_DIR}/../../Common.cmake)

file(GLOB_RECURSE solver_src *.cpp *.h ../common/*.cpp)

add_executable(solver ${solver_src})

if(DEBUG)
    target_compile_definitions(solver PUBLIC -DDEBUG)
endif(DEBUG)

find_package(OpenMP REQUIRED)

target_include_directories(solver PUBLIC ${CMAKE_CURRENT_LIST_DIR}/../common/)
target_link_libraries(solver PRIVATE 
    Boost::program_options
    Boost::graph
    Boost::heap
    Boost::timer
    tqdm-cpp
    OpenMP::OpenMP_CXX
)