#include "run_windows.hpp"

#ifdef _WIN32
#include <windows.h>
#include <stdexcept>

static std::string getWinError(int errCode, const std::string &prepend = "")
{
  std::string res;
  if(errCode)
    {
      LPTSTR winError = NULL;
      FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                    FORMAT_MESSAGE_FROM_SYSTEM |
                    FORMAT_MESSAGE_IGNORE_INSERTS,
                    NULL, errCode,
                    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
                    (LPTSTR)&winError, 0, NULL);
      if( winError )
        {
          res = prepend + winError;
          LocalFree(winError);
        }
    }
  return res;
}

void Launcher::win32_run(const std::string &command, const std::string &workdir)
{
  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  ZeroMemory(&pi, sizeof(pi));
  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);

  bool ok =
    CreateProcess(NULL, (char*)command.c_str(), NULL, NULL, false,
                  DETACHED_PROCESS, NULL,
                  (workdir==""?NULL:workdir.c_str()),
                  &si, &pi);

  if(!ok)
    {
      int code = GetLastError();
      std::string err = "Error executing " + command;
      err += getWinError(code, ": ");
      throw std::runtime_error(err);
    }

  // We don't need to keep track of the process
  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);
}
#endif
