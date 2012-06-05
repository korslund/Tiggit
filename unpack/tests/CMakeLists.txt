cmake_minimum_required(VERSION 2.6)

include_directories("../")
include_directories("../../")
include_directories("../../libs")

find_package(Boost COMPONENTS filesystem system REQUIRED)
set(ZZip_LIBRARIES zzip)

set(MDIR ../../libs/mangle/stream)
set(UDIR ../../unpack)
set(BASE ${UDIR}/dirwriter.cpp ${UDIR}/base.cpp)
set(ZIP ${BASE} ${UDIR}/unpack_zip.cpp)
set(AUTO ${UDIR}/auto.cpp ${ZIP})

set(BLIBS ${Boost_LIBRARIES})
set(LIBS ${BLIBS} ${ZZip_LIBRARIES})

add_executable(dummy1_test dummy1_test.cpp ${BASE})
target_link_libraries(dummy1_test ${BLIBS})

add_executable(zip_test zip_test.cpp ${ZIP})
target_link_libraries(zip_test ${LIBS})

add_executable(auto_test auto_test.cpp ${AUTO})
target_link_libraries(auto_test ${LIBS})
