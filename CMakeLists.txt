cmake_minimum_required(VERSION 2.6)

project( Tiggit )

# Add 'adv' for wxHyperlinkCtrl
find_package(wxWidgets COMPONENTS core base REQUIRED)
find_package(CURL REQUIRED)
find_package(Boost COMPONENTS filesystem system REQUIRED)

include_directories("libs/jsoncpp/include/" ${Boost_INCLUDE_DIRS} ${CURL_INCLUDE_DIRS})

set(JS libs/jsoncpp/src)
set(JSON ${JS}/json_reader.cpp ${JS}/json_writer.cpp ${JS}/json_value.cpp)

set(ZZip_LIBRARIES zzip)

set(MLIBS ${CURL_LIBRARIES} ${wxWidgets_LIBRARIES})
set(LIBS ${MLIBS} ${Boost_LIBRARIES} ${ZZip_LIBRARIES})

set(CPP tiggit.cpp wx/dialogs.cpp dirfinder/dirfinder.cpp ${JSON})

# This automagically sets up everything for us
include( ${wxWidgets_USE_FILE} )

if( APPLE )
  set( MACOSX_BUNDLE_ICON_FILE icon.icns )
  set( MACOSX_BUNDLE_SHORT_VERSION_STRING 0.1-alpha0 )
  set( MACOSX_BUNDLE_VERSION 0.1-alpha0  )
  set( MACOSX_BUNDLE_LONG_VERSION_STRING Alpha Version )
  set( CMAKE_OSX_ARCHITECTURES i386 ) # i386 only.

  set( PROGNAME Tiggit )
  add_executable( ${PROGNAME} MACOSX_BUNDLE ${CPP})
  add_custom_command( TARGET ${PROGNAME} POST_BUILD
      COMMAND mkdir 
      ARGS    ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/${PROGNAME}.app/Contents/Resources
      COMMAND cp 
      ARGS    ${CMAKE_CURRENT_SOURCE_DIR}/${MACOSX_BUNDLE_ICON_FILE} 
              ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_CFG_INTDIR}/${PROGNAME}.app/Contents/Resources 
  )
  target_link_libraries(${PROGNAME} ${LIBS})
else( APPLE )
  add_executable(tiggit ${CPP})
  target_link_libraries(tiggit ${LIBS})

  add_executable(tigmaker tigmaker.cpp ${JSON})
  target_link_libraries(tigmaker ${LIBS})
endif( APPLE )
