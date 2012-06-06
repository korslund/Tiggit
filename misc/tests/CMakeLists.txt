cmake_minimum_required(VERSION 2.6)

include_directories("../")

find_package(wxWidgets COMPONENTS core base REQUIRED)
find_package(Boost COMPONENTS filesystem system REQUIRED)

set(LIBS ${Boost_LIBRARIES})
set(WLIBS ${wxWidgets_LIBRARIES} ${LIBS})

set(MIDIR ../../misc)
set(FIND ${MIDIR}/dirfinder.cpp)
set(LOCK ${MIDIR}/lockfile.cpp)

# This automagically sets up everything for us
include( ${wxWidgets_USE_FILE} )

add_executable(finder_test finder_test.cpp ${FIND})
target_link_libraries(finder_test ${LIBS})

add_executable(wx_test wx_test.cpp ${FIND})
target_link_libraries(wx_test ${WLIBS})

add_executable(lock_test lock_test.cpp ${LOCK})
target_link_libraries(lock_test ${LIBS})
