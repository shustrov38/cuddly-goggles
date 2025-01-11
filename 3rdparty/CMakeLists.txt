include(FetchContent)

message(STATUS "Preparing 3rd-party libs...")
set(FETCHCONTENT_QUIET OFF)

add_subdirectory(boost)
