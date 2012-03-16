cmake_minimum_required(VERSION 2.6)

find_package(CURL REQUIRED)
find_package(Boost COMPONENTS filesystem system REQUIRED)

include_directories("../libs/jsoncpp/include/")
include_directories("../")

set(JS ../libs/jsoncpp/src)
set(JSON ${JS}/json_reader.cpp ${JS}/json_writer.cpp ${JS}/json_value.cpp)

set(LIBS ${CURL_LIBRARIES} ${Boost_LIBRARIES})

add_executable(gen_webdata gen_webdata.cpp ${JSON})
target_link_libraries(gen_webdata ${LIBS})

