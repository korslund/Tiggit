cmake_minimum_required(VERSION 2.6)

include_directories("../")
include_directories("../../")

find_package(Boost COMPONENTS thread REQUIRED)

set(JDIR ../../job)
set(THREAD ${JDIR}/thread.cpp ${JDIR}/job.cpp)

add_executable(thread1_test thread1_test.cpp ${THREAD})
target_link_libraries(thread1_test ${Boost_LIBRARIES})
