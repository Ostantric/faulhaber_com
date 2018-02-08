find_package(Boost COMPONENTS program_options REQUIRED)
find_package(PkgConfig REQUIRED)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

include_directories(
    ${Boost_INCLUDE_DIR}
    src/
)

add_executable(faulhaber_tests #ignore
    tests/test_faulhaber_low_level.cc
)

target_link_libraries(faulhaber_tests
    ${GTEST_BOTH_LIBRARIES}
    Threads::Threads
)

gtest_add_tests(faulhaber_tests "" AUTO)

# Executable for testing the spii plus client against the SPiiPlus MMI Application Studio simulator.
# We just run this manually for now.
add_executable(faulhaber
    src/faulhaber/debug_tool.cpp
    src/faulhaber/faulhaber_low_level_SerialPort.c
)

target_link_libraries(faulhaber
    ${Boost_LIBRARIES}
    ${sew_common_libraries}
)
