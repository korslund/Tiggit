cmake_minimum_required(VERSION 2.6)

find_package(wxWidgets COMPONENTS core base REQUIRED)
find_package(CURL REQUIRED)
find_package(Boost COMPONENTS filesystem REQUIRED)

include_directories("libs/jsoncpp/include/")

set(JS libs/jsoncpp/src)
set(JSON ${JS}/json_reader.cpp ${JS}/json_writer.cpp ${JS}/json_value.cpp)

set(ZZip_LIBRARIES zzip)

set(LIBS ${CURL_LIBRARIES} ${Boost_LIBRARIES} ${wxWidgets_LIBRARIES} ${ZZip_LIBRARIES})

# This automagically sets up everything for us
include( ${wxWidgets_USE_FILE} )

add_executable(tiggit tiggit.cpp ${JSON})
target_link_libraries(tiggit ${LIBS})
