cmake_minimum_required(VERSION 2.6)

include_directories("../")

set(TIG ../../)

set(LADIR ${TIG}/launcher)

set(LAUNCH ${LADIR}/run.cpp ${LADIR}/run_windows.cpp)

add_executable(run_test run_test.cpp ${LAUNCH})
