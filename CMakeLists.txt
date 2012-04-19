cmake_minimum_required(VERSION 2.6)

find_package(wxWidgets COMPONENTS core base REQUIRED)
find_package(CURL REQUIRED)
find_package(Boost COMPONENTS filesystem system REQUIRED)

include_directories("libs/jsoncpp/include/")

set(JS libs/jsoncpp/src)
set(JSON ${JS}/json_reader.cpp ${JS}/json_writer.cpp ${JS}/json_value.cpp)

set(ZZip_LIBRARIES zzip)

set(MLIBS ${CURL_LIBRARIES} ${wxWidgets_LIBRARIES})
set(LIBS ${MLIBS} ${Boost_LIBRARIES} ${ZZip_LIBRARIES})

# This automagically sets up everything for us
include( ${wxWidgets_USE_FILE} )

add_executable(tiggit tiggit.cpp ${JSON})
target_link_libraries(tiggit ${LIBS})

add_executable(tigmaker tigmaker.cpp ${JSON})
target_link_libraries(tigmaker ${MLIBS})

add_executable(fix_json fix_json.cpp ${JSON})
