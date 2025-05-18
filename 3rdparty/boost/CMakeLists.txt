message(STATUS "Fetching Boost...")

set(BOOST_COMPONENTS
    program_options polygon geometry graph timer heap pool
)

set(BOOST_INCLUDE_LIBRARIES ${BOOST_COMPONENTS})
set(BOOST_ENABLE_CMAKE ON)

fetchcontent_declare(
  boost
  GIT_REPOSITORY "https://github.com/boostorg/boost.git"
  GIT_TAG "boost-1.85.0"
  GIT_PROGRESS ON
  GIT_SHALLOW ON
  OVERRIDE_FIND_PACKAGE ON # needed to find correct Boost
  EXCLUDE_FROM_ALL ON # compile only what you need
)

fetchcontent_makeavailable(boost)
