#include "repo_locator.hpp"

#include "misc/dirfinder.hpp"
#include <boost/filesystem.hpp>
#include <spread/misc/readjson.hpp>
#include <spread/misc/jconfig.hpp>

static Misc::DirFinder dfinder("tiggit.net", "tiggit");

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

     Once we have disabled virtualization for this app by including
     a manifest, those paths will stop working. We will therefore
     have to go hunting for that data on our own.
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
  assert(0 && "NOT IN USE IN THIS VERSION");

  {
    std::string dir;
    // Check first if there's a stored repository path. If so, use that.
    if(dfinder.getStoredPath(dir))
      return dir;
  }

  path exeDir = finder.getExePath();
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
  p = dfinder.getAppData();
  if(hasRepo(p)) return p.string();

  // Last ditch effort: Check for a repository in the data/ folder
  // relative to the exe file.
  p = exeDir / "data";
  if(hasRepo(p)) return p.string();

  // Nothing found
  return "";
}

// Upgrade a legacy repository to the new format. Returns true if an
// upgrade was necessary.
static bool doUpgradeRepo(const path &where)
{
  assert(0 && "NOT UPDATED YET");

  using namespace Misc;

  // New configuration files
  JConfig conf, inst, news;
  conf.load((where/"tiglib.conf").string());
  inst.load((where/"tiglib_installed.conf").string());
  news.load((where/"tiglib_news.conf").string());
  path rateConf = where/"tiglib_rates.conf";

  // Is there an old config file?
  path oldcfg = where/"config";
  if(exists(oldcfg))
    try
      {
        // Open the old config file to convert the values
        JConfig in(oldcfg.string());

        // Convert wxTiggit-specific options
        if(in.has("vote_count"))
          {
            path tmp = where/"wxtiggit.conf";
            if(!exists(tmp))
              {
                JConfig out(tmp.string());
                out.setBool("show_votes", in.getBool("vote_count"));
              }
          }

        // Move last_time over to the new config.
        if(in.has("last_time") && !conf.has("last_time"))
          {
            uint32_t oldTime = in.getInt("last_time");
            conf.setInt64("last_time", oldTime);
          }

        // Kill the old file
        remove(oldcfg);

        /* Convert any other data we can find as well.
        */

        // Rename the ratings file
        oldcfg = where/"ratings.json";
        if(exists(oldcfg) && !exists(rateConf))
          rename(oldcfg, rateConf);

        // Convert list of read news
        oldcfg = where/"readnews.json";
        if(exists(oldcfg))
          {
            // Convert JSON array to config values
            Json::Value root = ReadJson::readJson(oldcfg.string());
            for(int i=0; i<root.size(); i++)
              {
                // The old values were ints, now we are using strings.
                int val = root[i].asInt();
                char buf[10];
                snprintf(buf, 10, "%d", val);
                std::string key(buf);
                news.setBool(key, true);
              }

            remove(oldcfg);
          }

        // Convert the list of installed games
        oldcfg = where/"installed.json";
        if(exists(oldcfg))
          {
            JConfig old(oldcfg.string());
            std::vector<std::string> list = old.getNames();

            for(int i=0; i<list.size(); i++)
              if(!inst.has(list[i]))
                inst.setInt(list[i], 2);

            remove(oldcfg);
          }

        // Find and rename screenshot images (add .png to filenames)
        directory_iterator iter(where/"cache/shot300x260/tiggit.net/"), end;
        for(; iter != end; ++iter)
          {
            path p = iter->path();
            if(!is_regular_file(p)) continue;

            // Check file ending
            std::string name = p.string();
            if(name.size() > 4 && name.substr(name.size()-4, 1) == ".")
              continue;

            // Rename to .png
            rename(name, name+".png");
          }

        // In some really old repos, the games may be installed into
        // "data/" instead of "games/". If so, rename it.
        if(exists(where/"data/") && !exists(where/"games"))
          rename(where/"data", where/"games");
      }
    catch(...) {}
  else return false;

  return true;
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

std::string findLegacyRepo()
{
  assert(0);

#ifdef _WIN32
  return findLegacyDir();
#else
  return "";
#endif
}

bool TigLibInt::upgradeRepo(const std::string &where)
{
  assert(0);

#ifdef _WIN32
  if(where != "")
    return doUpgradeRepo(where);
#endif
  return false;
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
