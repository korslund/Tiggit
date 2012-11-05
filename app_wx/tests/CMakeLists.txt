cmake_minimum_required(VERSION 2.6)

find_package(Boost COMPONENTS filesystem system thread REQUIRED)
find_package(CURL REQUIRED)

find_package(wxWidgets COMPONENTS core base REQUIRED)
include(${wxWidgets_USE_FILE})

include_directories("../")
include_directories("../../")
include_directories("../../libs/")
include_directories("../../libs/spread/")
include_directories("../../libs/spread/libs/jsoncpp/include/")
include_directories(${Boost_INCLUDE_DIRS})

add_subdirectory("../../libs/spread/" "_spread/")

set(LIBS ${Boost_LIBRARIES} zzip ${CURL_LIBRARIES})
set(WLIBS ${wxWidgets_LIBRARIES})

set(TIG ../..)
set(LIBDIR ${TIG}/libs)
set(MDIR ${LIBDIR}/mangle)

set(MIDIR ${TIG}/misc)
set(LADIR ${TIG}/launcher)
set(GIDIR ${TIG}/gameinfo)
set(LDIR ${TIG}/list)
set(TLDIR ${TIG}/tiglib)
set(WDIR ${TIG}/wx)
set(AWDIR ${TIG}/app_wx)

set(MANGLE ${MDIR}/stream/clients/io_stream.cpp)

add_executable(import_copy_test import_copy_test.cpp ${MIDIR}/logger.cpp ${AWDIR}/importer.cpp ${MANGLE} ${MIDIR}/freespace.cpp)
target_link_libraries(import_copy_test Spread ${LIBS})
