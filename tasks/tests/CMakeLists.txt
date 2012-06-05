cmake_minimum_required(VERSION 2.6)

include_directories("../")
include_directories("../../")
include_directories("../../libs")

find_package(Boost COMPONENTS filesystem system REQUIRED)
find_package(CURL REQUIRED)
set(ZZip_LIBRARIES zzip)

set(MDIR ../../libs/mangle/stream)
set(UDIR ../../unpack)
set(JDIR ../../job)
set(TDIR ../../tasks)
set(JOB ${JDIR}/job.cpp)
set(CURL ${TDIR}/curl.cpp ${MDIR}/servers/outfile_stream.hpp)
set(DL ${CURL} ${TDIR}/download.cpp ${JOB})

set(UBASE ${UDIR}/dirwriter.cpp ${UDIR}/base.cpp)
set(ZIP ${UBASE} ${UDIR}/unpack_zip.cpp)
set(AUTO ${UDIR}/auto.cpp ${ZIP})
set(UNP ${AUTO} ${TDIR}/unpack.cpp ${JOB})

add_executable(dl_test dl_test.cpp ${DL})
target_link_libraries(dl_test ${Boost_LIBRARIES} ${CURL_LIBRARIES})

add_executable(zip_test zip_test.cpp ${UNP})
target_link_libraries(zip_test ${Boost_LIBRARIES} ${ZZip_LIBRARIES})
