cmake_minimum_required(VERSION 2.6)

include_directories("../")
include_directories("../../")
include_directories("../../libs")

find_package(Boost COMPONENTS filesystem system REQUIRED)

set(MDIR ../../libs/mangle/stream)
set(UDIR ../../unpack)
set(BASE ${UDIR}/dirwriter.cpp ${UDIR}/unpack_base.cpp)

add_executable(dummy1_test dummy1_test.cpp ${BASE})
target_link_libraries(dummy1_test ${Boost_LIBRARIES})
