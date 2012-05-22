rem g++ dirfinder/tests/wx_test.cpp dirfinder/dirfinder.cpp -o wx_test.exe -I../wxWidgets/include -I../wxWidgets/lib/gcc_dll/mswu -I../mingw-div/include/ -L../boost -lboost_filesystem -lboost_system wxbase28u_gcc_custom.dll wxmsw28u_core_gcc_custom.dll -Idirfinder/

g++ dirfinder/tests/finder_test.cpp dirfinder/dirfinder.cpp -o finder_test.exe -I../mingw-div/include/ -L../boost -lboost_filesystem -lboost_system -Idirfinder/

finder_test.exe