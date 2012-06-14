cmake_minimum_required(VERSION 2.6)

find_package(Boost COMPONENTS filesystem system thread REQUIRED)
find_package(CURL REQUIRED)

# This automagically sets up everything for us
#find_package(wxWidgets COMPONENTS core base REQUIRED)
#include( ${wxWidgets_USE_FILE} )
#set(WLIBS ${wxWidgets_LIBRARIES} ${LIBS})

include_directories("./")
include_directories("libs/")
include_directories("libs/jsoncpp/include/")
include_directories(${Boost_INCLUDE_DIRS})

set(LIBS ${Boost_LIBRARIES} zzip ${CURL_LIBRARIES})

set(LIBDIR libs/)
set(MDIR ${LIBDIR}/mangle)
set(MIDIR misc)
set(JS ${LIBDIR}/jsoncpp/src)
set(RJDIR readjson)
set(GIDIR gameinfo)
set(JDIR job)
set(UDIR unpack)
set(TDIR tasks)

set(JOB ${JDIR}/thread.cpp ${JDIR}/job.cpp)
set(MISC ${MIDIR}/dirfinder.cpp ${MIDIR}/lockfile.cpp)
set(MANGLE ${MDIR}/stream/clients/io_stream.cpp)
set(JSON ${JS}/json_reader.cpp ${JS}/json_writer.cpp ${JS}/json_value.cpp)
set(READJSON ${RJDIR}/readjson.cpp)
set(GAMEINFO ${GIDIR}/json_tigdata.cpp ${GIDIR}/binary_tigdata.cpp ${GIDIR}/tigloader.cpp)
set(TASKS ${TDIR}/curl.cpp ${TDIR}/download.cpp ${TDIR}/unpack.cpp)
set(UNPACK ${UDIR}/dirwriter.cpp ${UDIR}/base.cpp ${UDIR}/unpack_zip.cpp ${UDIR}/auto.cpp)

set(ALL ${JOB} ${MISC} ${MANGLE} ${JSON} ${READJSON} ${GAMEINFO} ${TASKS} ${UNPACK})

add_executable(proto1_test proto1_test.cpp ${ALL})
target_link_libraries(proto1_test ${LIBS})
