#include <windows.h>
#include <boost/filesystem.hpp>
#include <string>

TCHAR pathbuf[MAX_PATH];

std::string getBinPath(const std::string &appName)
{
  SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, pathbuf);

  boost::filesystem::path base = pathbuf;
  return (base / appName / "bin").string();
}

std::string getExe()
{
  GetModuleFileName(NULL, pathbuf, MAX_PATH);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, INT nCmdShow)
{
  std::string exe = getExe();
  std::string dir = getBinPath("tiggit");
  MessageBox(0, dir.c_str(), exe.c_str(), MB_ICONINFORMATION);
}
