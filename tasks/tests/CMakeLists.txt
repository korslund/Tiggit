cmake_minimum_required(VERSION 2.6)

include_directories("../")
include_directories("../../")
include_directories("../../libs")

find_package(Boost COMPONENTS thread REQUIRED)
find_package(CURL REQUIRED)

set(JDIR ../../job)
set(TDIR ../../tasks)
set(MDIR ../../libs/mangle/stream)
set(JOB ${JDIR}/job.cpp)
set(CURL ${TDIR}/curl.cpp ${MDIR}/servers/outfile_stream.hpp)
set(DL ${CURL} ${TDIR}/download.cpp ${JOB})

add_executable(dl_test dl_test.cpp ${DL})
target_link_libraries(dl_test ${Boost_LIBRARIES} ${CURL_LIBRARIES})
