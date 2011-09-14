g++ win32_installer.cpp -o setup_stub.exe -Wl,--subsystem,windows -I../mingw-div/include/ -L../boost -lboost_filesystem -lboost_system -I../zziplib -I../zlib -L../zziplib/zzip -lzzip -L../zlib-1.2.5 -lz
g++ win32_makeinst.cpp -o make_inst.exe
