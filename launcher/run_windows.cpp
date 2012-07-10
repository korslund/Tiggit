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
      if(FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
                       NULL, errCode, 0, (LPTSTR)&winError, 0, NULL) == 0)
        res = prepend + winError;
      if( winError ) LocalFree(winError);
    }
  return res;
}

static std::string getWinError(const std::string &prepend = "")
{
  int errCode = GetLastError();
  return getWinError(errCode, prepend);
}

void Launcher::win32_run(const std::string &command, const std::string &workdir)
{
  STARTUPINFO si;
  PROCESS_INFORMATION pi;

  ZeroMemory(&pi, sizeof(pi));
  ZeroMemory(&si, sizeof(si));
  si.cb = sizeof(si);

  bool ok =
    CreateProcess(NULL, command.c_str(), NULL, NULL, false,
                  DETACHED_PROCESS, NULL,
                  (workdir==""?NULL:workdir.c_str()),
                  &si, &pi);

  if(!ok)
    {
      std::string err = "Error executing " + command;
      err += getWinError(": ");
      throw std::runtime_error(err);
    }

  // We don't need to keep track of the process
  CloseHandle(pi.hProcess);
  CloseHandle(pi.hThread);
}
#endif
