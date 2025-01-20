cmake_minimum_required(VERSION 3.10.0)

include_guard(DIRECTORY)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_FLAGS "-O3 -march=native -Wall -Wextra") # -O0 -g -fsatitize=address

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-enum-float-conversion")
#              boost.Polygon dependency ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-maybe-uninitialized")
#      boost.Polygon.Voronoi dependency ^^^^^^^^^^^^^^^^^^^^^^^^

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-empty-body")
#      boost.Heap dependency            ^^^^^^^^^^^^^^^