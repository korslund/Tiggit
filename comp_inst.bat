g++ win32_setup.cpp -o setup.exe -Wl,--subsystem,windows -I../mingw-div/include/ -L../boost -lboost_filesystem -lboost_system -lole32 -luuid

