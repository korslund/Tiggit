cmake_minimum_required(VERSION 2.6)

find_package(Boost COMPONENTS filesystem system REQUIRED)

include_directories("../")
include_directories("../../")
include_directories("../../libs/")
include_directories(${Boost_INCLUDE_DIRS})

set(LIBDIR ../../libs)
set(SPDIR ${LIBDIR}/spread)
set(SLIBDIR ${SPDIR}/libs)

set(MDIR ${LIBDIR}/mangle)

set(SMIDIR ${SPDIR}/misc)

set(GIDIR ../../gameinfo)
set(TIGDATA ${GIDIR}/binary_tigfile.cpp)
set(TIGLOADER ${GIDIR}/tigloader.cpp ${TIGDATA})

set(BLIBS ${Boost_LIBRARIES})

add_executable(binary_test binary_test.cpp ${TIGDATA})
add_executable(tigloader_test tigloader_test.cpp ${TIGLOADER})
target_link_libraries(tigloader_test ${BLIBS})
