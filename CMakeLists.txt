cmake_minimum_required(VERSION 3.5.0)
project(planar_graph_coloring VERSION 0.1.0 LANGUAGES C CXX)

option(USE_JEMALLOC "Build with jemalloc memory allocator" ON)

set(DEBUG ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

add_subdirectory(3rdparty)
add_subdirectory(src)

file(GLOB_RECURSE python_scripts ${CMAKE_CURRENT_LIST_DIR}/scripts/*.py)
foreach(script_path ${python_scripts})
    get_filename_component(script_name ${script_path} NAME)
    configure_file(${script_path} ${CMAKE_CURRENT_BINARY_DIR}/${script_name} COPYONLY)
endforeach(script_path ${python_scripts})

file(GLOB_RECURSE datasets ${CMAKE_CURRENT_LIST_DIR}/datasets/*.col)
foreach(dataset_path ${datasets})
    get_filename_component(dataset_name ${dataset_path} NAME)
    configure_file(${dataset_path} ${CMAKE_CURRENT_BINARY_DIR}/${dataset_name} COPYONLY)
endforeach(dataset_path ${datasets})
