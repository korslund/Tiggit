cmake_minimum_required(VERSION 2.6)

find_package(wxWidgets COMPONENTS core base adv REQUIRED)

# This automagically sets up everything for us
include( ${wxWidgets_USE_FILE} )

add_executable(wxtest1_flexgrid wxtest1_flexgrid.cpp)
target_link_libraries(wxtest1_flexgrid ${wxWidgets_LIBRARIES})

add_executable(wxtest2_ctrl_list wxtest2_ctrl_list.cpp)
target_link_libraries(wxtest2_ctrl_list ${wxWidgets_LIBRARIES})
