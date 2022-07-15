cmake_minimum_required(VERSION 2.6)

find_package(Boost COMPONENTS filesystem system thread REQUIRED)
find_package(CURL REQUIRED)
find_package(ZLIB REQUIRED)

find_package(wxWidgets COMPONENTS core base adv REQUIRED)
include(${wxWidgets_USE_FILE})

add_definitions(-DNEED_LOCKGUARD=1)
include_directories("./")
include_directories("libs/")
include_directories("libs/spread/")
include_directories("libs/spread/libs/jsoncpp/include/")
include_directories(${Boost_INCLUDE_DIRS})

add_subdirectory("libs/spread/")
add_subdirectory("libs/unpackcpp/")

set(LIBS Spread UnpackCpp ${Boost_LIBRARIES} ${ZLIB_LIBRARIES} ${CURL_LIBRARIES})
set(WLIBS ${wxWidgets_LIBRARIES})

set(LIBDIR libs)
set(MDIR ${LIBDIR}/mangle)
set(TIG .)

set(MIDIR ${TIG}/misc)
set(LADIR ${TIG}/launcher)
set(GIDIR ${TIG}/gameinfo)
set(LDIR ${TIG}/list)
set(TLDIR ${TIG}/tiglib)
set(WDIR ${TIG}/wx)
set(AWDIR ${TIG}/app_wx)

set(MISC ${MIDIR}/dirfinder.cpp ${MIDIR}/lockfile.cpp ${MIDIR}/freespace.cpp ${MIDIR}/fetch.cpp ${MIDIR}/curl_post.cpp)
set(MANGLE ${MDIR}/stream/clients/io_stream.cpp)
set(GAMEINFO ${GIDIR}/stats_json.cpp ${GIDIR}/tigloader.cpp)
set(LIST ${LDIR}/sortlist.cpp ${LDIR}/listbase.cpp ${LDIR}/picklist.cpp ${LDIR}/parentbase.cpp)
set(TIGLIB ${TLDIR}/gamedata.cpp ${TLDIR}/gamelister.cpp ${TLDIR}/sorters.cpp ${TLDIR}/repo.cpp ${TLDIR}/liveinfo.cpp ${TLDIR}/news.cpp ${TLDIR}/repo_locator.cpp)
set(LAUNCH ${LADIR}/run.cpp ${LADIR}/run_windows.cpp)

set(WX ${WDIR}/frame.cpp ${WDIR}/tabbase.cpp ${WDIR}/gametab.cpp ${WDIR}/image_viewer.cpp ${WDIR}/gamelist.cpp ${WDIR}/listbase.cpp ${WDIR}/newstab.cpp ${WDIR}/progress_holder.cpp ${WDIR}/dialogs.cpp)
set(APPWX ${AWDIR}/wxtiggit_main.cpp ${AWDIR}/gamedata.cpp ${AWDIR}/gameinf.cpp ${AWDIR}/gamelist.cpp ${AWDIR}/notifier.cpp ${AWDIR}/jobprogress.cpp ${AWDIR}/appupdate.cpp ${AWDIR}/importer_gui.cpp ${AWDIR}/importer_backend.cpp)

# All CPP files, excluding wxWidgets-dependent files
set(ALL ${MISC} ${MANGLE} ${GAMEINFO} ${LIST} ${TIGLIB} ${LAUNCH})

# Only wxWidgets CPP files
set(ALL_WX ${WX} ${APPWX})

add_executable(tiggit ${ALL} ${ALL_WX})
target_link_libraries(tiggit ${LIBS} ${WLIBS})
#add_executable(tiggit-cli ${ALL} cli/cli_main.cpp)
#target_link_libraries(tiggit-cli ${LIBS})
