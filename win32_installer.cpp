#include <windows.h>
#include <shlobj.h>
#include <shellapi.h>
#include <boost/filesystem.hpp>
#include <string>
#include <stdexcept>
#include <fstream>
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

void fail(const std::string &msg)
{
  throw std::runtime_error(msg);
}

struct SizeInfo
{
  int sign, start, size;
};

// Extract payload from an exe file to the given destination folder
void extractPayload(const std::string &exe,
                    const std::string &dest)
{
  std::ifstream inf(exe.c_str(), std::ios::binary);
  if(!inf)
    fail("Failed to open " + exe);

  // Read the payload descriptor
  SizeInfo si;
  inf.seekg(-sizeof(SizeInfo), std::iosbase::end);
  inf.read((char*)&si, sizeof(SizeInfo));

  if(si.sign != 0x13737FAB)
    fail("Missing payload in " + exe);

  // Seek to the data
  inf.clear();
  inf.seekg(si.start)

  // Set up the destination file
  std::ofstream of(dest.c_str(), std::ios::binary);

  // Read and dump file
  char buf[4096];
  while(si.size > 0)
    {
      inf.read(buff, sizeof(buf));
      size_t count = inf.gcount();
      of.write(buf, count);
      si.size -= count;
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, INT nCmdShow)
{
  fs::path bin = getBinPath("tiggit");
  std::string exe = getExe();

  // Make sure bin/ exists
  fs::create_directories(bin);

  // Destination zip file
  std::string zip = (bin / "tmp.zip").string();

  try
    {
      // Extract embedded ZIP file
      extractPayload(exe, zip);

      // Unpack the contents into the bin/ folder
      UnZip z;
      z.unpack(zip, bin.string());
    }
  catch(std::exception &e)
    {
      MessageBox(NULL, e.what(), "Error", MB_ICONERROR);
      return 1;
    }

  // Kill the zip file
  fs::remove(zip);

  // Run the main program
  ShellExecute(NULL, "open", (bin/"tiggit.exe").string().c_str(),
               NULL, NULL, SW_SHOW);

  return 0;
}
