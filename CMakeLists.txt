cmake_minimum_required(VERSION 2.6)

find_package(Boost COMPONENTS filesystem system thread REQUIRED)
find_package(CURL REQUIRED)

find_package(wxWidgets COMPONENTS core base REQUIRED)
include(${wxWidgets_USE_FILE})

include_directories("./")
include_directories("libs/")
include_directories("libs/jsoncpp/include/")
include_directories(${Boost_INCLUDE_DIRS})

set(LIBS ${Boost_LIBRARIES} zzip ${CURL_LIBRARIES})
set(WLIBS ${wxWidgets_LIBRARIES})

set(LIBDIR libs/)
set(MDIR ${LIBDIR}/mangle)
set(MIDIR misc)
set(JS ${LIBDIR}/jsoncpp/src)
set(RJDIR readjson)
set(GIDIR gameinfo)
set(JDIR job)
set(UDIR unpack)
set(TDIR tasks)
set(LDIR list)
set(TLDIR tiglib)
set(WDIR wx)
set(AWDIR app_wx)

set(JOB ${JDIR}/thread.cpp ${JDIR}/job.cpp)
set(MISC ${MIDIR}/dirfinder.cpp ${MIDIR}/lockfile.cpp)
set(MANGLE ${MDIR}/stream/clients/io_stream.cpp)
set(JSON ${JS}/json_reader.cpp ${JS}/json_writer.cpp ${JS}/json_value.cpp)
set(READJSON ${RJDIR}/readjson.cpp)
set(GAMEINFO ${GIDIR}/json_tigdata.cpp ${GIDIR}/binary_tigdata.cpp ${GIDIR}/tigloader.cpp)
set(TASKS ${TDIR}/curl.cpp ${TDIR}/download.cpp ${TDIR}/unpack.cpp)
set(UNPACK ${UDIR}/dirwriter.cpp ${UDIR}/base.cpp ${UDIR}/unpack_zip.cpp ${UDIR}/auto.cpp)
set(LIST ${LDIR}/sortlist.cpp ${LDIR}/listbase.cpp ${LDIR}/picklist.cpp ${LDIR}/parentbase.cpp)
set(TIGLIB ${TLDIR}/gamedata.cpp ${TLDIR}/gamelister.cpp ${TLDIR}/sorters.cpp)
set(WX ${WDIR}/frame.cpp ${WDIR}/tabbase.cpp ${WDIR}/gametab.cpp ${WDIR}/image_viewer.cpp ${WDIR}/gamelist.cpp ${WDIR}/listbase.cpp ${WDIR}/newstab.cpp)
set(APPWX ${AWDIR}/wxtiggit_main.cpp ${AWDIR}/gamedata.cpp)

# All CPP files, excluding wxWidgets-users
set(ALL ${JOB} ${MISC} ${MANGLE} ${JSON} ${READJSON} ${GAMEINFO} ${TASKS} ${UNPACK} ${LIST} ${TIGLIB})

# Only wxWidgets CPP files
set(ALL_WX ${WX} ${APPWX})

add_executable(tiggit ${ALL} ${ALL_WX})
target_link_libraries(tiggit ${LIBS} ${WLIBS})
