windres --output-format=coff win_test.rc win_test.res
g++ win_test.cpp launcher/run.cpp launcher/run_windows.cpp misc/dirfinder.cpp -o win_test.exe -Wl,--subsystem,windows -I../mingw-div/include/ -L../boost -lboost_filesystem -lboost_system win_test.res
chmod a+r win_test.exe
win_test.exe
cat win_test.log
cat /cygdrive/c/Users/maja/AppData/Local/tiggit.net/test1/run/win_test.log
