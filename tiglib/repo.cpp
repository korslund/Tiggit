#include "repo.hpp"
#include "fetch.hpp"
#include <stdio.h>
#include "tasks/install.hpp"
#include "job/thread.hpp"
#include "server_api.hpp"
#include "repo_locator.hpp"

#define CACHE_VERSION 1

using namespace TigLib;

bool Repo::findRepo(const std::string &where)
{
  dir = where;

  if(dir == "")
    dir = TigLibInt::findRepo();

  // Nothing was found. Tell the user.
  if(dir == "")
    return false;

  listFile = getPath("all_games.json");
  tigDir = getPath("tigfiles/");

  // Store path for later
  if(!TigLibInt::setStoredPath(dir))
    return false;

  return true;
}

std::string Repo::defaultPath() { return TigLibInt::getDefaultPath(); }

bool Repo::initRepo(bool forceLock)
{
  // Lock the repo before we start writing to it
  if(!lock.lock(getPath("lock"), forceLock))
    return false;

  // Make sure the repo format is up-to-date
  TigLibInt::upgradeRepo(dir);

  // Open config files
  conf.load(getPath("tiglib.conf"));
  inst.load(getPath("tiglib_installed.conf"));
  news.load(getPath("tiglib_news.conf"));
  rates.load(getPath("tiglib_rates.conf"));

  // Load config options
  lastTime = conf.getInt64("last_time", 0xffffffffffff);

  return true;
}

void Repo::downloadFinished(const std::string &idname,
                            const std::string &urlname)
{
  // Set config status
  setInstallStatus(idname, 2);

  // Respect offline mode even after a download has succeeded. The
  // user may have a dodgy connection or something.
  if(offline) return;

  std::string url = ServerAPI::dlCountURL(urlname);
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

  // Ignore votes in offline mode
  if(offline) return;

  rates.setInt(id, rate);

  // Send it off to the server
  std::string url = ServerAPI::rateURL(urlname, rate);
  Fetch::fetchString(url, true);
}

std::string Repo::getPath(const std::string &fname)
{
  return dir + "/" + fname;
}

std::string Repo::fetchPath(const std::string &url,
                            const std::string &fname)
{
  std::string outfile = getPath(fname);
  if(!offline)
    Fetch::fetchFile(url, outfile);
  return outfile;
}

Jobify::JobInfoPtr Repo::fetchFiles()
{
  using namespace Tasks;
  using namespace Jobify;

  JobInfoPtr info;

  // If we're in offline mode, skip this entire function
  if(offline) return info;

  assert(lock.isLocked());
  Fetch::fetchIfOlder(ServerAPI::listURL(), listFile, 60);

  // Check if we need to download the cache
  if(conf.getInt("cache_version") < CACHE_VERSION)
    {
      conf.setInt("cache_version", CACHE_VERSION);
      Job *job = new InstallTask
        (ServerAPI::cacheURL(),
         getPath("incoming/cache.zip"),
         dir);
      info = job->getInfo();
      Jobify::Thread::run(job);
    }

  return info;
}

struct MyFetch : GameInfo::URLManager
{
  bool offline;

  void getUrl(const std::string &url, const std::string &outfile)
  { if(!offline) Fetch::fetchFile(url, outfile); }
};

void Repo::loadData()
{
  assert(isLocked());
  MyFetch fetch;
  fetch.offline = offline;
  data.data.addChannel(listFile, tigDir, &fetch);
  data.createLiveData(this);
}
