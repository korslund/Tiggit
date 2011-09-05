/*
  Windows bit-ness checker. Not in use yet. Checks if the client
  system is 32-bit or 64-bit (regardless of whether THIS is a 32 or 64
  bit application!)

  Taken from:
  http://mark.koli.ch/2009/10/reliably-checking-os-bitness-32-or-64-bit-on-windows-with-a-tiny-c-app.html

  A linux version would need to check the kernel some way or
  another. At any rate, we'd normally only run this at startup - in
  fact, just create a platform-independent OS class that checks this
  as well as what OS we have and other pertinent information.
 */

#include "stdafx.h"
#include <iostream>
#include "comutil.h"

#define RESPONSE_32_BIT "32"
#define RESPONSE_64_BIT "64"

using namespace std;

typedef BOOL (WINAPI *IW64PFP)(HANDLE, BOOL *);

int main(int argc, char **argv){

  BOOL res = FALSE;

  // When this application is compiled as a 32-bit app,
  // and run on a native 64-bit system, Windows will run
  // this application under WOW64.  WOW64 is the Windows-
  // on-Windows subsystem that lets native 32-bit applications
  // run in 64-bit land.  This calls the kernel32.dll
  // API to see if this process is running under WOW64.
  // If it is running under WOW64, then that clearly means
  // this 32-bit application is running on a 64-bit OS,
  // and IsWow64Process will return true.
  IW64PFP IW64P = (IW64PFP)GetProcAddress(
            GetModuleHandle(L"kernel32"), "IsWow64Process");

  if(IW64P != NULL){
    IW64P(GetCurrentProcess(), &res);
  }

  cout << ((res) ? RESPONSE_64_BIT : RESPONSE_32_BIT) << endl;

  return 0;
}
