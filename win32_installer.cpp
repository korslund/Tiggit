#include <windows.h>
#include <shlobj.h>
#include <shellapi.h>
#include <boost/filesystem.hpp>
#include <string>
#include "unzip.hpp"

namespace fs = boost::filesystem;

TCHAR pathbuf[MAX_PATH];

fs::path getBinPath(const std::string &appName)
{
  SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, pathbuf);
  fs::path base = pathbuf;
  return base / appName / "bin";
}

std::string getExe()
{
  GetModuleFileName(NULL, pathbuf, MAX_PATH);
  return std::string(pathbuf);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, INT nCmdShow)
{
  fs::path bin = getBinPath("tiggit");
  std::string exe = getExe();

  // Make sure bin/ exists
  fs::create_directories(bin);

  // TODO: Extract embedded ZIP file and store the filename here
  std::string zip = "tmp.zip";

  // Unpack the contents into the bin/ folder
  try
    {
      Unzip z;
      z.unpack(zip, bin.string());
    }
  catch(std::exception &e)
    {
      MessageBox(NULL, e.what(), "Error", MB_ICONERROR);
      return 1;
    }

  // Kill the zip file
  remove(zip);

  // Run the main program
  ShellExecute(NULL, "open", (bin/"tiggit.exe").string().c_str(),
               NULL, NULL, SW_SHOW);

  return 0;
}
