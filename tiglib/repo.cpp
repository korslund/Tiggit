#include "repo.hpp"
#include "fetch.hpp"
#include "misc/dirfinder.hpp"
#include <boost/filesystem.hpp>
#include "readjson/readjson.hpp"
#include <stdio.h>
#include "tasks/install.hpp"
#include "job/thread.hpp"

#define CACHE_VERSION 1

using namespace TigLib;

struct MyFetch : GameInfo::URLManager
{
  void getUrl(const std::string &url, const std::string &outfile)
  { Fetch::fetchFile(url, outfile); }
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

  // Open config files
  conf.load(getPath("tiglib.conf"));
  inst.load(getPath("tiglib_installed.conf"));
  news.load(getPath("tiglib_news.conf"));
  // Opened further down:
  std::string rateConf = getPath("tiglib_rates.conf");

  // Is there an old config file?
  std::string oldcfg = getPath("config");
  if(exists(oldcfg))
    try
      {
        // Open the old config file to convert the values
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

        /* If the old 'config' file existed, chances are that this is
           an old repository. We should convert any other old data we
           can find as well.
         */

        // Rename the ratings file
        oldcfg = getPath("ratings.json");
        if(exists(oldcfg) && !exists(rateConf))
          rename(oldcfg, rateConf);

        // Convert list of read news
        oldcfg = getPath("readnews.json");
        if(exists(oldcfg))
          {
            // Convert JSON array to config value
            Json::Value root = ReadJson::readJson(oldcfg);
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

        // In some really old repos, the games may be installed into
        // "data/" instead of "games/". If so, rename it.
        if(exists(getPath("data/")) && !exists(getPath("games/")))
          rename(getPath("data"), getPath("games"));
      }
    catch(...) {}

  // Load ratings file
  rates.load(rateConf);

  // Load config options
  lastTime = conf.getInt64("last_time", 0xffffffffffff);

  return true;
}

void Repo::downloadFinished(const std::string &idname,
                            const std::string &urlname)
{
  // Set config status
  setInstallStatus(idname, 2);

  // Tell the server
  std::string url = "http://tiggit.net/api/count/" + urlname + "&download";
  Fetch::fetchString(url, true);
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

int Repo::getRating(const std::string &id)
{
  int res = rates.getInt(id, -1);
  if(res < 0 || res > 5)
    res = -1;
  return res;
}

void Repo::setRating(const std::string &id, const std::string &urlname,
                     int rate)
{
  if(rate < 0 || rate > 5)
    return;

  // No point in voting more than once, the server will filter it out.
  if(rates.has(id))
    return;

  rates.setInt(id, rate);

  // Send it off to the server
  std::string url = "http://tiggit.net/api/count/" + urlname + "&rate=";
  char rateCh = '0' + rate;
  url += rateCh;
  Fetch::fetchString(url, true);
}

std::string Repo::getPath(const std::string &fname)
{
  using namespace boost::filesystem;
  return (path(dir)/fname).string();
}

std::string Repo::fetchPath(const std::string &url,
                            const std::string &fname)
{
  std::string outfile = getPath(fname);
  Fetch::fetchFile(url, outfile);
  return outfile;
}

Jobify::JobInfoPtr Repo::fetchFiles()
{
  using namespace Tasks;
  using namespace Jobify;

  JobInfoPtr info;

  assert(lock.isLocked());
  Fetch::fetchIfOlder("http://tiggit.net/api/all_games.json",
                      listFile, 60);

  // Check if we need to download the cache
  if(conf.getInt("cache_version") < CACHE_VERSION)
    {
      conf.setInt("cache_version", CACHE_VERSION);
      Job *job = new InstallTask
        ("http://sourceforge.net/projects/tiggit/files/client_data/cache.zip",
         getPath("incoming/cache.zip"),
         getPath(""));
      info = job->getInfo();
      Jobify::Thread::run(job);
    }

  return info;
}

void Repo::loadData()
{
  assert(lock.isLocked());
  MyFetch fetch;
  data.data.addChannel(listFile, tigDir, &fetch);
  data.createLiveData(this);
}
