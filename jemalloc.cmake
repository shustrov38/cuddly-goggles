message(STATUS "Fetching jemalloc...")

find_program(AUTORECORDFOUND autoreconf)
if(NOT AUTORECORDFOUND)
    message(FATAL_ERROR "autoreconf not found! Install autotools:\n sudo apt-get install autoconf automake libtool")
endif()

set(JEMALLOC_PREFIX ${CMAKE_BINARY_DIR}/_deps/jemalloc)

set(JEMALLOC_BIN_DIR ${JEMALLOC_PREFIX}/bin)
set(JEMALLOC_LIB_DIR ${JEMALLOC_PREFIX}/lib)
set(JEMALLOC_INCLUDE_DIR ${JEMALLOC_PREFIX}/include)

set(JEMALLOC_CONFIGURE_OPTS 
    --prefix=${JEMALLOC_PREFIX} 
    --disable-cxx
    --disable-doc
)

ExternalProject_Add(
    jemalloc_external
    GIT_REPOSITORY "https://github.com/jemalloc/jemalloc.git"
    GIT_TAG "5.3.0"
    GIT_SHALLOW ON
    BUILD_IN_SOURCE ON
    UPDATE_COMMAND ""
    PATCH_COMMAND ""
    CONFIGURE_COMMAND
        COMMAND <SOURCE_DIR>/autogen.sh ${JEMALLOC_CONFIGURE_OPTS}
    BUILD_COMMAND make -j
    INSTALL_COMMAND make install
    LOG_CONFIGURE ON
    LOG_BUILD ON
    LOG_INSTALL OFF
)

add_library(jemalloc SHARED IMPORTED)
add_dependencies(jemalloc jemalloc_external)

set_target_properties(jemalloc PROPERTIES
    IMPORTED_LOCATION ${JEMALLOC_LIB_DIR}/libjemalloc.so
    INTERFACE_INCLUDE_DIRECTORIES ${JEMALLOC_INCLUDE_DIR}
)