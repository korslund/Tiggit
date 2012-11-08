cmake_minimum_required(VERSION 2.6)

include_directories("../")
include_directories("../../")
include_directories("../../libs/")
include_directories("../../libs/jsoncpp/include/")

find_package(wxWidgets COMPONENTS core base REQUIRED)
find_package(Boost COMPONENTS filesystem system REQUIRED)

set(LIBS ${Boost_LIBRARIES})
set(WLIBS ${wxWidgets_LIBRARIES} ${LIBS})

set(MIDIR ../../misc)
set(FIND ${MIDIR}/dirfinder.cpp)
set(LOCK ${MIDIR}/lockfile.cpp)
set(FREE ${MIDIR}/freespace.cpp)

set(LIBDIR ../../libs)
set(MDIR ${LIBDIR}/mangle)

set(SPDIR ${LIBDIR}/spread)

set(MANGLE ${MDIR}/stream/clients/io_stream.cpp)
set(JS ${SPDIR}/lib/jsoncpp/src)
set(JSON ${JS}/json_reader.cpp ${JS}/json_writer.cpp ${JS}/json_value.cpp)
set(READJSON ${MANGLE} ${JSON} ${SPDIR}/json/readjson.cpp)
set(C85 ${SPDIR}/misc/comp85.cpp)

# This automagically sets up everything for us
include( ${wxWidgets_USE_FILE} )

add_executable(finder_test finder_test.cpp ${FIND})
target_link_libraries(finder_test ${LIBS})

add_executable(wx_test wx_test.cpp ${FIND})
target_link_libraries(wx_test ${WLIBS})

add_executable(lock_test lock_test.cpp ${LOCK})
target_link_libraries(lock_test ${LIBS})

add_executable(freespace freespace.cpp ${FREE})
target_link_libraries(freespace ${LIBS})
