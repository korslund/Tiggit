cmake_minimum_required(VERSION 2.6)

find_package(Boost COMPONENTS filesystem system thread REQUIRED)
find_package(CURL REQUIRED)

include_directories("../")
include_directories("../../")
include_directories("../../libs/")
include_directories("../../libs/spread/")
include_directories("../../libs/spread/libs/jsoncpp/include/")
include_directories(${Boost_INCLUDE_DIRS})

add_subdirectory("../../libs/spread/" "_spread/")

set(LIBS ${Boost_LIBRARIES} ${CURL_LIBRARIES})

set(TIG ../..)
set(LIBDIR ${TIG}/libs)
set(MDIR ${LIBDIR}/mangle)

set(MIDIR ${TIG}/misc)
set(TADIR ${TIG}/tigauth)

set(MANGLE ${MDIR}/stream/clients/io_stream.cpp)
set(AUTH ${TADIR}/api_urlmaker.cpp ${TADIR}/api_response.cpp ${TADIR}/tigauth.cpp)

add_executable(auth_cli auth_cli.cpp ${MIDIR}/fetch.cpp ${AUTH} ${MANGLE})
target_link_libraries(auth_cli Spread ${LIBS})

add_executable(url_test url_test.cpp ${TADIR}/api_urlmaker.cpp)
target_link_libraries(url_test)

add_executable(response_test response_test.cpp ${TADIR}/api_response.cpp ${MANGLE})
target_link_libraries(response_test Spread ${LIBS})
