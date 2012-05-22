#include "dirfinder.hpp"

#include <boost/filesystem.hpp>
#include <stdlib.h>
#include <fstream>

using namespace DirFinder;
using namespace boost::filesystem;

#ifdef _WIN32

#include <windows.h>
#include <shlobj.h>
#include <tchar.h>

TCHAR pathbuf[MAX_PATH];

path getPathCSIDL(int csidl)
{
  SHGetFolderPath(NULL, csidl, NULL, 0, pathbuf);
  return path(pathbuf);
}

std::string getDataDir(int csidl, const std::string &vname, const std::string &aname)
{
  path p = getPathCSIDL(csidl);
  if(vname != "") p /= vname;
  return (p / aname).string();
}

bool Finder::getStandardPath(std::string &dir)
{
  dir = getDataDir(CSIDL_APPDATA, vname, aname);
  return isWritable(dir);
}

bool openRegKey(const std::string &vname, const std::string &aname, HKEY &hkey)
{
  LPCTSTR sk = ("Software\\" + vname + "\\" + aname).c_str();;

  LONG openRes = RegCreateKeyEx(HKEY_CURRENT_USER,
                                sk,
                                0,NULL,0,0,NULL,
                                &hkey,
                                NULL);

  return openRes == ERROR_SUCCESS;
}


static bool os_setStoredPath(const std::string &dir,
                             const std::string &vname,
                             const std::string &aname,
                             const std::string &dname)
{
  HKEY hkey;
  if(!openRegKey(vname,aname,hkey))
    return false;

  DWORD dwSize = dir.size();
  if(RegSetValueEx(hkey, dname.c_str(), 0, REG_SZ,
                   dir.c_str(), dwSize) != ERROR_SUCCESS)
    return false;
 
  RegCloseKey(hkey);
  return true;
}

static std::string os_getStoredPath(const std::string &vname,
                                    const std::string &aname,
                                    const std::string &dname)
{
  HKEY hkey;
  if(!openRegKey(vname,aname,hkey))
    return false;

  DWORD dwType = REG_SZ;
  DWORD dwSize = MAX_PATH;
  if(RegQueryValueEx(hkey, dname.c_str(), 0, &dwType, 
                     pathbuf, &dwSize) != ERROR_SUCCESS)
    return "";

  RegCloseKey(hkey);

  return string(pathbuf, dwSize);
}

#else
#ifdef __unix

static std::string getHome(const std::string &aname)
{
  return std::string(getenv("HOME")) + "/." + aname + "/";
}

static std::string getPathFile(const std::string &aname,
                               const std::string &dname)
{
  return getHome(aname) + dname + ".conf";
}

bool Finder::getStandardPath(std::string &dir)
{
  dir = getHome(aname);
  return isWritable(dir);
}

static std::string os_getStoredPath(const std::string &vname,
                                    const std::string &aname,
                                    const std::string &dname)
{
  std::string file = getPathFile(aname,dname);
  if(exists(file))
    {
      std::ifstream inp(file.c_str());
      if(inp)
        {
          std::string res;
          inp >> res;
          if(inp)
            return res;
        }
    }
  return "";
}

static bool os_setStoredPath(const std::string &dir,
                             const std::string &vname,
                             const std::string &aname,
                             const std::string &dname)
{
  std::ofstream out(getPathFile(aname,dname).c_str());
  if(out)
    {
      out << dir;
      out.flush();
      if(out)
        return true;
    }
  return false;
}

#else
#error "This platform is not supported yet."
#endif
#endif

bool Finder::getStoredPath(std::string &dir)
{
  dir = os_getStoredPath(vname,aname,dname);
  return isWritable(dir);
}

bool Finder::setStoredPath(const std::string &dir)
{
  return isWritable(dir) && os_setStoredPath(dir,vname,aname,dname);
}

bool Finder::isWritable(const std::string &dir)
{
  if(dir == "")
    return false;

  try
    {
      // Create directories
      create_directories(dir);

      // Do write check
      path p = dir;
      p /= "xyz.tmp";
      const char *file = p.string().c_str();
      {
        std::ofstream out(file);
        if(!out) return false;
        out << "abc";
        if(!out) return false;
      }
      {
        std::ifstream inp(file);
        if(!inp) return false;
        std::string tmp;
        inp >> tmp;
        if(!inp || tmp != "abc")
          return false;
      }

      remove(p);
    }
  // All errors means the dir is not writable
  catch(...){ return false; }
  return true;
}
