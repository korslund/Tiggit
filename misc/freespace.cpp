#include "freespace.hpp"
#include <stdexcept>
#include <boost/filesystem.hpp>

namespace bf = boost::filesystem;

static void fail(const std::string &msg)
{
  throw std::runtime_error(msg);
}

#ifdef _WIN32
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

static void throwError(std::string err)
{
  int code = GetLastError();
  err += getWinError(code, ": ");
  fail(err);
}

#else
#include <sys/vfs.h>
#include <sys/stat.h>
#endif

void Misc::getDiskSpace(const std::string &filePath, int64_t &free, int64_t &total)
{
  bf::path dir = bf::absolute(filePath);

  if(!bf::exists(dir))
    fail("File or directory not found: " + dir.string());

  if(!bf::is_directory(dir))
    dir = dir.parent_path();

#ifdef _WIN32
  // Make sure path is \ terminated (HACK)
  std::string dirStr = (dir/"tmp").parent_path().string() + "\\";

  ULARGE_INTEGER Lfree,Ltotal;
  if(!::GetDiskFreeSpaceEx(dirStr.c_str(), &Lfree, &Ltotal, NULL))
    throwError("Unable to calculate disk space for " + filePath);

  free = Lfree.QuadPart;
  total = Ltotal.QuadPart;

#else //linux

  struct stat stst;
  struct statfs stfs;

  std::string dirStr = dir.string();
  if ( ::stat(dirStr.c_str(),&stst) == -1 ||
       ::statfs(dirStr.c_str(),&stfs) == -1 )
    fail("Unable to stat " + dirStr);

  free = stfs.f_bavail * stst.st_blksize;
  total = stfs.f_blocks * stst.st_blksize;
#endif // _WIN32
}
