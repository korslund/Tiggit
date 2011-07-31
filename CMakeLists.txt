cmake_minimum_required(VERSION 2.6)

find_package(wxWidgets COMPONENTS core base REQUIRED)

include_directories("libs/jsoncpp/include/")

set(JS libs/jsoncpp/src)
set(JSON ${JS}/json_reader.cpp ${JS}/json_writer.cpp ${JS}/json_value.cpp)

# This automagically sets up everything for us
include( ${wxWidgets_USE_FILE} )

add_executable(tiggit tiggit.cpp ${JSON})
target_link_libraries(tiggit ${wxWidgets_LIBRARIES})
