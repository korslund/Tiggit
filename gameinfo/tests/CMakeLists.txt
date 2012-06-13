cmake_minimum_required(VERSION 2.6)

find_package(Boost COMPONENTS filesystem system REQUIRED)

include_directories("../")
include_directories("../../")
include_directories("../../libs/")
include_directories("../../libs/jsoncpp/include/")
include_directories(${Boost_INCLUDE_DIRS})

set(LIBDIR ../../libs)
set(MDIR ${LIBDIR}/mangle)

set(JS ${LIBDIR}/jsoncpp/src)
set(MANGLE ${MDIR}/stream/clients/io_stream.cpp)
set(JSON ${JS}/json_reader.cpp ${JS}/json_writer.cpp ${JS}/json_value.cpp)
set(READJSON ${MANGLE} ${JSON} ../../readjson/readjson.cpp)
set(TIGDATA ../../tigdata/json_tigdata.cpp ../../tigdata/binary_tigdata.cpp ${READJSON})
set(GIDIR ../../gameinfo)
set(GAMEINFO ${GIDIR}/tigloader.cpp ${TIGDATA})

set(BLIBS ${Boost_LIBRARIES})

add_executable(tigloader_test tigloader_test.cpp ${GAMEINFO})
target_link_libraries(tigloader_test ${BLIBS})
