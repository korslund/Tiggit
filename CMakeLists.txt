cmake_minimum_required(VERSION 2.6)

find_package(wxWidgets COMPONENTS core base adv html REQUIRED)

# This automagically sets up everything for us
include( ${wxWidgets_USE_FILE} )

add_executable(wxtest1_flexgrid wxtest1_flexgrid.cpp)
target_link_libraries(wxtest1_flexgrid ${wxWidgets_LIBRARIES})

add_executable(wxtest2_ctrl_list wxtest2_ctrl_list.cpp)
target_link_libraries(wxtest2_ctrl_list ${wxWidgets_LIBRARIES})

add_executable(wxtest3_html wxtest3_html.cpp)
target_link_libraries(wxtest3_html ${wxWidgets_LIBRARIES})

add_executable(wxtest4_list wxtest4_list.cpp)
target_link_libraries(wxtest4_list ${wxWidgets_LIBRARIES})
