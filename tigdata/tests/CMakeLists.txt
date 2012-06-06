cmake_minimum_required(VERSION 2.6)

include_directories("../")
include_directories("../../")
include_directories("../../libs/")
include_directories("../../libs/jsoncpp/include/")

set(LIBDIR ../../libs)
set(MDIR ${LIBDIR}/mangle)

set(JS ${LIBDIR}/jsoncpp/src)
set(MANGLE ${MDIR}/stream/clients/io_stream.cpp)
set(JSON ${JS}/json_reader.cpp ${JS}/json_writer.cpp ${JS}/json_value.cpp)
set(READJSON ${MANGLE} ${JSON} ../../readjson/readjson.cpp)
set(TIGDATA ../../tigdata/json_tigdata.cpp)

add_executable(json_test json_test.cpp ${READJSON} ${TIGDATA})
