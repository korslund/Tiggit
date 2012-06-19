#include "repo.hpp"
#include "fetch.hpp"
#include "misc/dirfinder.hpp"
#include <boost/filesystem.hpp>

using namespace TigLib;

struct MyFetch : GameInfo::URLManager
{
  void getUrl(const std::string &url, const std::string &outfile)
  { fetchFile(url, outfile); }
};

static std::string getHomeDir()
{
  Misc::DirFinder find("tiggit.net", "tiggit");

  std::string res;
  if(find.getStoredPath(res))
    return res;

  if(!find.getStandardPath(res))
    return "";

  find.setStoredPath(res);
  return res;
}

// Locate stored, standard or legacy repository paths, in that
// order. May return empty string if no existing or usable path was
// found.
static std::string locateRepo()
{
  /* TODO: This function should be responsible for finding legacy
     repository locations. Kill getHomeDir and integrate the code from
     old/repo.hpp instead.
  */
  return getHomeDir();
}

bool Repo::findRepo(const std::string &where)
{
  dir = where;

  if(dir == "")
    dir = locateRepo();

  if(dir == "")
    return false;

  listFile = getPath("all_games.json");
  tigDir = getPath("tigfiles/");
  return true;
}

bool Repo::initRepo(bool forceLock)
{
  if(!lock.lock(getPath("lock"), forceLock))
    return false;

  using namespace boost::filesystem;
  using namespace Misc;

  // Open new config file
  conf.load(getPath("tiglib.conf"));
  inst.load(getPath("tiglib_installed.conf"));
  news.load(getPath("tiglib_news.conf"));
  rates.load(getPath("tiglib_rates.conf"));

  // Is there an old config file?
  std::string oldcfg = getPath("config");
  if(exists(oldcfg))
    try
      {
        // Convert it to something usable
        JConfig in(oldcfg);

        // Convert wxTiggit-specific options
        if(in.has("vote_count"))
          {
            std::string tmp = getPath("wxtiggit.conf");
            if(!exists(tmp))
              {
                JConfig out(tmp);
                out.setBool("show_votes", in.getBool("vote_count"));
              }
          }

        // Move last_time over to the new config.
        if(in.has("last_time") && !conf.has("last_time"))
          {
            int64_t oldTime = in.getInt("last_time");
            setLastTime(oldTime);
          }

        // Kill the old file
        remove(oldcfg);

        // Convert the list of installed games
        oldcfg = getPath("installed.json");
        if(exists(oldcfg))
          {
            JConfig old(oldcfg);
            std::vector<std::string> list = old.getNames();

            for(int i=0; i<list.size(); i++)
              if(!inst.has(list[i]))
                setInstallStatus(list[i], 2);

            remove(oldcfg);
          }

        // Find and rename screenshot images (add .png to filenames)
        directory_iterator iter(getPath("cache/shot300x260/tiggit.net/")), end;
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

        /* TODO:
           - if there is a data/ but no games/, rename it
        */
      }
    catch(...) {}

  // Load config options
  conf.getData("last_time", &lastTime, 8);

  return true;
}

void Repo::setInstallStatus(const std::string &idname, int status)
{
  // This may be call from worker threads, but this is ok because the
  // JConfig setters are thread safe.
  inst.setInt(idname, status);
}

void Repo::setLastTime(int64_t val)
{
  // Store as binary data, since 64 bit int support in general is
  // a bit dodgy.
  conf.setData("last_time", &val, 8);
}

std::string Repo::getPath(const std::string &fname)
{
  using namespace boost::filesystem;
  return (path(dir)/fname).string();
}

void Repo::fetchFiles()
{
  assert(lock.isLocked());
  fetchIfOlder("http://tiggit.net/api/all_games.json",
               listFile, 60);
}

void Repo::loadData()
{
  assert(lock.isLocked());
  MyFetch fetch;
  data.data.addChannel(listFile, tigDir, &fetch);
  data.createLiveData(this);
}
