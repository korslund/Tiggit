cmake_minimum_required(VERSION 2.6)

include_directories("../")
include_directories("../../")
include_directories("../../libs")

#find_package(Boost COMPONENTS filesystem system REQUIRED)
#set(ZZip_LIBRARIES zzip)
find_package(wxWidgets COMPONENTS core base REQUIRED)

# Set up wxWidgets
include( ${wxWidgets_USE_FILE} )

#set(MDIR ../../libs/mangle/stream)
#set(UDIR ../../unpack)
set(WDIR ../../wx)

#set(BASE ${UDIR}/dirwriter.cpp ${UDIR}/base.cpp)
#set(ZIP ${BASE} ${UDIR}/unpack_zip.cpp)
#set(AUTO ${UDIR}/auto.cpp ${ZIP})
set(WX ${WDIR}/frame.cpp ${WDIR}/tabbase.cpp ${WDIR}/gametab.cpp ${WDIR}/image_viewer.cpp ${WDIR}/gamelist.cpp ${WDIR}/listbase.cpp ${WDIR}/newstab.cpp ${WDIR}/dialogs.cpp)

#set(BLIBS ${Boost_LIBRARIES})
#set(LIBS ${BLIBS} ${ZZip_LIBRARIES})
set(WLIBS ${wxWidgets_LIBRARIES})

add_executable(wx1_test wx1_test.cpp ${WX})
target_link_libraries(wx1_test ${WLIBS})
