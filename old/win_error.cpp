#include <string.h>
#include <windows.h>

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

static std::string getWinError(const std::string &prepend = "")
{
  int errCode = GetLastError();
  return getWinError(errCode, prepend);
}
