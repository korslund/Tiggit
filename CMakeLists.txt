cmake_minimum_required(VERSION 2.6)

find_package(wxWidgets COMPONENTS core base REQUIRED)

# This automagically sets up everything for us
include( ${wxWidgets_USE_FILE} )

add_executable(tiggit tiggit.cpp)
target_link_libraries(tiggit ${wxWidgets_LIBRARIES})
