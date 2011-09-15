#include <windows.h>
#include <shlobj.h>
#include <shellapi.h>
#include <atlbase.h>

#include <boost/filesystem.hpp>
#include <string>
#include <stdexcept>
#include <fstream>

#include "unzip.hpp"

namespace fs = boost::filesystem;

TCHAR pathbuf[MAX_PATH];
WCHAR wbuf[MAX_PATH];

fs::path getPathCSIDL(int csidl)
{
  SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, pathbuf);
  return fs::path(pathbuf);
}

fs::path getBinPath(const std::string &appName)
{
  return getPathCSIDL(CSIDL_LOCAL_APPDATA) / appName / "bin";
}

// Note: pointer only valid until next run
WCHAR *toWide(const std::string &str)
{
  MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, wbuf, MAX_PATH);
  return wbuf;
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
  inf.seekg(0, std::ios::end);
  inf.seekg((int)inf.tellg()-sizeof(SizeInfo));
  inf.read((char*)&si, sizeof(SizeInfo));

  if(si.sign != 0x13737FAB)
    fail("Missing payload in " + exe);

  // Seek to the data
  inf.clear();
  inf.seekg(si.start);

  // Set up the destination file
  std::ofstream of(dest.c_str(), std::ios::binary);

  // Read and dump file
  char buf[4096];
  while(si.size > 0)
    {
      inf.read(buf, sizeof(buf));
      size_t count = inf.gcount();
      of.write(buf, count);
      si.size -= count;
    }
}

void createLinks(const std::string name, const std::string &exe)
{
  CCoInitialize init;
  CComPtr<IShellLink> spsl;
  spsl.CoCreateInstance(CLSID_ShellLink);
  spsl->SetPath(exe.c_str());
  spsl->SetDescription(name.c_str());

  CComQIPtr<IPersistFile> pf(spsl);

  // Use this for links you don't want to highlight, ie. everything
  // except the main program link
  /*
  PROPVARIANT pvar;
  pvar.vt = VT_BOOL;
  pvar.boolVal = VARIANT_TRUE;
  pf->SetValue(PKEY_AppUserModel_ExcludeFromShowInNewInstall, pvar);
  */

  std::string lname = name + ".lnk";

  // Save desktop link
  fs::path link = getPathCSIDL(CSIDL_DESKTOPDIRECTORY) / lname;
  pf.Save(toWide(link.string()), TRUE);

  // Create start menu directory
  link = getPathCSIDL(CSIDL_PROGRAMS) / name;
  fs::create_directories(link);

  // Save the start menu link
  link /= lname;
  pf.Save(toWide(link.string()), TRUE);
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, INT nCmdShow)
{
  fs::path bin = getBinPath("tiggit");
  std::string exe = getExe();
  std::string dest_exe = (bin/"tiggit.exe").string();

  // Make sure bin/ exists
  fs::create_directories(bin);

  // Destination zip file
  std::string zip = (bin / "tmp.zip").string();

  int failed = 0;
  try
    {
      // Extract embedded ZIP file
      extractPayload(exe, zip);

      // Unpack the contents into the bin/ folder
      UnZip z;
      z.unpack(zip, bin.string());

      // Create shortcuts
      createLinks("Open Game Loader", dest_exe);
    }
  catch(std::exception &e)
    {
      MessageBox(NULL, e.what(), "Error", MB_ICONERROR);
      failed = 1;
    }

  // Kill the zip file
  fs::remove(zip);

  if(failed == 0)
    {
      // Run the installed program
      ShellExecute(NULL, "open", dest_exe.c_str(),
                   NULL, NULL, SW_SHOW);
    }

  return failed;
}
