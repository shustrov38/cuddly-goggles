message(STATUS "Fetching tqdm-cpp...")

set(TQDM_HPP_PATH ${CMAKE_CURRENT_BINARY_DIR}/tqdm.hpp)

if (NOT EXISTS ${TQDM_HPP_PATH})
    file (
        DOWNLOAD https://raw.githubusercontent.com/mraggi/tqdm-cpp/master/tqdm.hpp
        ${TQDM_HPP_PATH}
    )
endif()

file(SIZE ${TQDM_HPP_PATH} TQDM_HPP_FILE_SIZE)
if(TQDM_HPP_FILE_SIZE EQUAL 0)
    file (REMOVE ${TQDM_HPP_PATH})
    message (
        FATAL_ERROR
        "Couldn't download tqdm-cpp library. Double check the URL."
    )
endif()

add_library(tqdm-cpp INTERFACE)
target_include_directories(tqdm-cpp INTERFACE ${CMAKE_CURRENT_BINARY_DIR})