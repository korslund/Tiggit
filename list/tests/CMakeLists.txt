cmake_minimum_required(VERSION 2.6)

include_directories("../")

set(LDIR ../../list)
set(LIST ${LDIR}/sortlist.cpp ${LDIR}/listbase.cpp ${LDIR}/picklist.cpp ${LDIR}/parentbase.cpp)

add_executable(list_test list_test.cpp ${LIST})
