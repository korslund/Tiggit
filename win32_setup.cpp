#include <windows.h>
#include <shlobj.h>
#include <shellapi.h>

#include <boost/filesystem.hpp>
#include <string>
#include <stdexcept>
#include <fstream>

namespace fs = boost::filesystem;

TCHAR pathbuf[MAX_PATH];
WCHAR wbuf[MAX_PATH];

fs::path getPathCSIDL(int csidl)
{
  SHGetFolderPath(NULL, csidl, NULL, 0, pathbuf);
  return fs::path(pathbuf);
}

fs::path getBinPath(const std::string &appName)
{
  return getPathCSIDL(CSIDL_LOCAL_APPDATA) / appName / "bin";
}

fs::path getExe()
{
  GetModuleFileName(NULL, pathbuf, MAX_PATH);
  return fs::path(pathbuf);
}

// Note: pointer only valid until next run
WCHAR *toWide(const std::string &str)
{
  MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, wbuf, MAX_PATH);
  return wbuf;
}

void fail(const std::string &msg)
{
  throw std::runtime_error(msg);
}

void createLinks(const std::string name, const std::string &exe)
{
  CoInitialize(NULL);
  HRESULT res;
  IShellLink *lnk = NULL;

  res = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
                         IID_IShellLink, reinterpret_cast<void**>(&lnk));
  if(!SUCCEEDED(res))
    fail("Couldn't create shortcut links");

  lnk->SetPath(exe.c_str());
  lnk->SetDescription(name.c_str());
  //lnk->SetIconLocation("where", 0);

  IPersistFile *pf = NULL;
  res = lnk->QueryInterface(IID_IPersistFile, reinterpret_cast<void**>(&pf));
  if(!SUCCEEDED(res))
    {
      lnk->Release();
      fail("Couldn't create shortcut links");
    }

  // Use this for links you don't want to highlight, ie. everything
  // except the main program link. May need some rewriting.
  /*
  PROPVARIANT pvar;
  pvar.vt = VT_BOOL;
  pvar.boolVal = VARIANT_TRUE;
  pf->SetValue(PKEY_AppUserModel_ExcludeFromShowInNewInstall, pvar);
  */

  std::string lname = name + ".lnk";

  // Save desktop link
  fs::path link = getPathCSIDL(CSIDL_DESKTOPDIRECTORY) / lname;
  pf->Save(toWide(link.string()), TRUE);

  // Create start menu directory
  link = getPathCSIDL(CSIDL_PROGRAMS) / name;
  fs::create_directories(link);

  // Save the start menu link
  link /= lname;
  pf->Save(toWide(link.string()), TRUE);

  pf->Release();
  lnk->Release();
}

void copy_files(fs::path from, fs::path to)
{
  using namespace boost::filesystem;

  // Copy files over
  directory_iterator iter(from), end;
  for(; iter != end; ++iter)
    {
      path p = iter->path();

      // Only process files
      if(!is_regular_file(p)) continue;

      // Skip setup.exe
      if(p.leaf() == "setup.exe")
        continue;

      // Destination
      path dest = to / p.leaf();

      // Remove destination, if it exists
      if(exists(dest))
        remove(dest);

      // Copy the file
      copy_file(p, dest);
    }
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine, INT nCmdShow)
{
  fs::path bin = getBinPath("tiggit");
  std::string dest_exe = (bin/"tiggit.exe").string();
  std::string name = "Tiggit Game Installer";

  try
    {
      // Make sure bin/ exists
      fs::create_directories(bin);

      // Copy all our sibling files to bin/
      fs::path from = getExe().parent_path();
      copy_files(from, bin);

      // Create shortcuts
      createLinks(name, dest_exe);
    }
  catch(std::exception &e)
    {
      MessageBox(NULL, e.what(), "Error", MB_ICONERROR);
      return 1;
    }

  // Run the installed program
  ShellExecute(NULL, "open", dest_exe.c_str(),
               NULL, NULL, SW_SHOW);

  return 0;
}
