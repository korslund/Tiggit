cmake_minimum_required(VERSION 2.6)

include_directories("../")

find_package(wxWidgets COMPONENTS core base REQUIRED)
find_package(Boost COMPONENTS filesystem system REQUIRED)

set(LIBS ${wxWidgets_LIBRARIES} ${Boost_LIBRARIES})
set(CPP ../dirfinder.cpp)

# This automagically sets up everything for us
include( ${wxWidgets_USE_FILE} )

add_executable(finder_test finder_test.cpp ${CPP})
target_link_libraries(finder_test ${LIBS})

add_executable(wx_test wx_test.cpp ${CPP})
target_link_libraries(wx_test ${LIBS})
