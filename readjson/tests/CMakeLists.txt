cmake_minimum_required(VERSION 2.6)

include_directories("../")
include_directories("../../libs/")
include_directories("../../libs/jsoncpp/include/")

set(LIBDIR ../../libs)
set(MDIR ${LIBDIR}/mangle)

set(MANGLE ${MDIR}/stream/clients/io_stream.cpp)
set(JS ${LIBDIR}/jsoncpp/src)
set(JSON ${JS}/json_reader.cpp ${JS}/json_writer.cpp ${JS}/json_value.cpp)
set(READJSON ${MANGLE} ${JSON} ../readjson.cpp)

add_executable(file_test file_test.cpp ${READJSON})
