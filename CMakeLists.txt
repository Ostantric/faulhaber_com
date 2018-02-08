cmake_minimum_required(VERSION 3.1)

project(sew)

if (NOT DEFINED SKIP_TESTS)
    enable_testing()
    find_package(GTest REQUIRED)
    include_directories(${GTEST_INCLUDE_DIRS})
endif(NOT DEFINED SKIP_TESTS)

include("cmake-tools/get_git_source_repo.cmake")

###### Global compiler, linker, etc option setup. ######
# Be wary - anything you add here is propagated
# to all subprojects.
######

# Override at the command line with -DCMAKE_BUILD_TYPE=DEBUG to get debugging
# symboles and no optimization
if (CMAKE_BUILD_TYPE STREQUAL "")
    set(CMAKE_BUILD_TYPE "RELEASE")
endif()

set(sew_cxx_warnings
    "-Wall -Wcast-align -Wpointer-arith -Wshadow -Wsign-compare -Wformat=2\
    -Wno-format-y2k -Wmissing-braces -Wparentheses -Wtrigraphs -Wextra\
    -Wno-int-to-pointer-cast"
)
set(CMAKE_CXX_FLAGS
    "-std=c++11\
    ${CMAKE_CXX_FLAGS}\
    ${sew_cxx_warnings}"
)
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

###### Common resources available across all sewing machine services ######
include_directories("common/")
add_subdirectory("common/")

###### Individual sewing machine services ######
#
# These should not depend on each other -
# only on common and the protobuf definitions
add_subdirectory("faulhaber/")
