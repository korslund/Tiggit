#include "repo_locator.hpp"

#include "misc/dirfinder.hpp"
#include <boost/filesystem.hpp>
#include <spread/misc/readjson.hpp>
#include <spread/misc/jconfig.hpp>

static Misc::DirFinder dfinder("tiggit.net", "tiggit-data");

using namespace boost::filesystem;

/* We don't expect to find legacy repositories on any other platform
   than windows. This helps us narrow down a few bits of code.
*/
#ifdef _WIN32

/* Check a directory if it has an old-style repository. May change
   'dir'.
*/
static bool hasRepo(path &dir)
{
  // A repo is found if it has a config file.
  path file = dir / "config";
  if(exists(file)) return true;

  /* Legacy installations may have depended on UAC virtualization to
     store data in places normally not allowed.

     Since we have disabled virtualization for this app by including a
     manifest, those paths will stop working. We will therefore have
     to go hunting for that data on our own.
  */

  // Get the relative version of the path, eg.:
  // C:\Program Files\Tiggit\   =>   Program Files\Tiggit\
  dir = dir.relative_path();

  // Add the VirtualStore directory. New result would now be eg.:
  // C:\Users\myname\AppData\Local\VirtualStore\Program Files\Tiggit\
  dir = path(dfinder.getAppData()) / "VirtualStore" / dir;

  // Return true if we found a repository
  file = dir / "config";
  if(exists(file)) return true;

  // Nothing was found
  return false;
}

// Find and return legacy repository path, if any
static std::string findLegacyDir()
{
  {
    // Check first if there's a stored repository path under the old
    // name (tiggit instead of tiggit-data).
    Misc::DirFinder find("tiggit.net", "tiggit");
    std::string dr;
    if(find.getStoredPath(dr))
      {
	path dir = dr;
	if(hasRepo(dir))
	  return dir.string();
      }
  }

  path exeDir = dfinder.getExePath();
  exeDir = exeDir.parent_path();

  // Check for a paths.json co-located with the exe.
  {
    path pathfile = exeDir / "paths.json";
    if(exists(pathfile))
      {
        try
          {
            Json::Value root = ReadJson::readJson(pathfile.string());
            path p = root["repo_dir"].asString();

            if(p != "")
              {
                // The path in the pathfile may be relative. If so,
                // convert it to absolute.
                if(!p.has_root_path())
                  p = absolute(p, exeDir);

                if(hasRepo(p)) return p.string();
              }
          }
        catch(...) {}
      }
  }

  // Check standard appdata location
  path p = dfinder.getAppData();
  p /= "tiggit";
  if(hasRepo(p)) return p.string();

  // Last ditch effort: Check for a repository in the data/ folder
  // relative to the exe file.
  p = exeDir / "data";
  if(hasRepo(p)) return p.string();

  // Nothing found
  return "";
}

#endif

// -------------------- PUBLIC FUNCTIONS ---------------------

std::string TigLibInt::getStoredPath()
{
  std::string dir;
  if(dfinder.getStoredPath(dir))
    return dir;
  return "";
}

std::string TigLibInt::findLegacyRepo()
{
#ifdef _WIN32
  return findLegacyDir();
#else
  return "";
#endif
}

/* Set stored path, will be retrieved the next time findRepo is
   called.
 */
bool TigLibInt::setStoredPath(const std::string &dir)
{
  return dfinder.setStoredPath(dir);
}

/* Get OS-dependent default path. Mostly intended as a default value
   to present to the user when findRepo() returns nothing.
 */
std::string TigLibInt::getDefaultPath()
{
  std::string res;
  if(dfinder.getStandardPath(res))
    return res;
  return "";
}
