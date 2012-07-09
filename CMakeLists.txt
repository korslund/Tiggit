cmake_minimum_required(VERSION 2.6)

find_package(Boost COMPONENTS filesystem system thread REQUIRED)
find_package(CURL REQUIRED)

find_package(wxWidgets COMPONENTS core base REQUIRED)
include(${wxWidgets_USE_FILE})

include_directories("./")
include_directories("libs/")
include_directories("libs/spread/")
include_directories("libs/spread/libs/jsoncpp/include/")
include_directories(${Boost_INCLUDE_DIRS})

set(LIBS ${Boost_LIBRARIES} zzip ${CURL_LIBRARIES})
set(WLIBS ${wxWidgets_LIBRARIES})

set(LIBDIR libs)
set(SPDIR ${LIBDIR}/spread)
set(MDIR ${LIBDIR}/mangle)
set(SLDIR ${SPDIR}/libs)

set(MIDIR misc)
set(SMDIR ${SPDIR}/misc)

set(JS ${SLDIR}/jsoncpp/src)
set(GIDIR gameinfo)
set(JDIR ${SPDIR}/job)
set(UDIR ${SPDIR}/unpack)
set(TDIR ${SPDIR}/tasks)
set(LDIR list)
set(TLDIR tiglib)
set(WDIR wx)
set(AWDIR app_wx)

set(JOB ${JDIR}/thread.cpp ${JDIR}/job.cpp)
set(MISC ${MIDIR}/dirfinder.cpp ${MIDIR}/lockfile.cpp)
set(SPMISC ${SMDIR}/jconfig.cpp ${SMDIR}/comp85.cpp ${SMDIR}/readjson.cpp)
set(MANGLE ${MDIR}/stream/clients/io_stream.cpp)
set(JSON ${JS}/json_reader.cpp ${JS}/json_writer.cpp ${JS}/json_value.cpp)
set(GAMEINFO ${GIDIR}/json_tigdata.cpp ${GIDIR}/binary_tigdata.cpp ${GIDIR}/tigloader.cpp)
set(TASKS ${TDIR}/curl.cpp ${TDIR}/download.cpp ${TDIR}/unpack.cpp ${TDIR}/multi.cpp ${TDIR}/fileop.cpp ${TDIR}/notify.cpp ${TDIR}/install.cpp)
set(UNPACK ${UDIR}/dirwriter.cpp ${UDIR}/base.cpp ${UDIR}/unpack_zip.cpp ${UDIR}/auto.cpp)
set(LIST ${LDIR}/sortlist.cpp ${LDIR}/listbase.cpp ${LDIR}/picklist.cpp ${LDIR}/parentbase.cpp)
set(TIGLIB ${TLDIR}/gamedata.cpp ${TLDIR}/gamelister.cpp ${TLDIR}/sorters.cpp ${TLDIR}/repo.cpp ${TLDIR}/fetch.cpp ${TLDIR}/liveinfo.cpp ${TLDIR}/news.cpp ${TLDIR}/repo_locator.cpp)

set(WX ${WDIR}/frame.cpp ${WDIR}/tabbase.cpp ${WDIR}/gametab.cpp ${WDIR}/image_viewer.cpp ${WDIR}/gamelist.cpp ${WDIR}/listbase.cpp ${WDIR}/newstab.cpp ${WDIR}/progress_holder.cpp ${WDIR}/dialogs.cpp)
set(APPWX ${AWDIR}/wxtiggit_main.cpp ${AWDIR}/gamedata.cpp ${AWDIR}/gameinf.cpp ${AWDIR}/gamelist.cpp ${AWDIR}/notifier.cpp ${AWDIR}/jobprogress.cpp)

# All CPP files, excluding wxWidgets-dependent files
set(ALL ${JOB} ${MISC} ${SPMISC} ${MANGLE} ${JSON} ${GAMEINFO} ${TASKS} ${UNPACK} ${LIST} ${TIGLIB})

# Only wxWidgets CPP files
set(ALL_WX ${WX} ${APPWX})

add_executable(tiggit ${ALL} ${ALL_WX})
target_link_libraries(tiggit ${LIBS} ${WLIBS})

#add_executable(tiggit-cli ${ALL} cli/cli_main.cpp)
#target_link_libraries(tiggit-cli ${LIBS})
