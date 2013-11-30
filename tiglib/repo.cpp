#include "repo.hpp"
#include "server_api.hpp"
#include "repo_locator.hpp"
#include "misc/lockfile.hpp"
#include "misc/fetch.hpp"
#include "gameinfo/stats_json.hpp"
#include <spread/job/thread.hpp>
#include <spread/spread.hpp>
#include <spread/hash/hash.hpp>
#include <spread/tasks/download.hpp>
#include <boost/filesystem.hpp>
#include <stdio.h>

using namespace TigLib;
using namespace Spread;

namespace bf = boost::filesystem;

//#define PRINT_DEBUG
#ifdef PRINT_DEBUG
#include <iostream>
#define PRINT(a) std::cout << a << "\n"
#else
#define PRINT(a)
#endif

struct Repo::_Internal
{
  Misc::LockFile lock;
  TigLib::GameData data;
  SpreadLib spread;
  std::string tmp;

  _Internal(const std::string &spreadDir, const std::string &tmpDir)
    : spread(spreadDir, tmpDir), tmp(tmpDir)
  {
    DownloadTask::userAgent = "Tiggit/1.0 - see http://tiggit.net";
  }

  ~_Internal()
  { bf::remove_all(tmp); }
};

bool Repo::isLocked() const
{ return ptr && ptr->lock.isLocked(); }

const InfoLookup &Repo::getList() const
{
  assert(ptr);
  return ptr->data.lookup;
}

List::ListBase &Repo::baseList()
{
  assert(ptr);
  return ptr->data.allList;
}

bool Repo::findRepo(const std::string &where)
{
  PRINT("findRepo(" << where << ")");
  dir = where;

  if(dir == "")
    dir = TigLibInt::getStoredPath();

  PRINT("  dir=" << dir);

  // Nothing was found. Tell the user.
  if(dir == "")
    return false;

  setDirs();

  // Store path for later
  if(!TigLibInt::setStoredPath(dir))
    return false;

  return true;
}

bool Repo::setStoredPath(const std::string &newPath)
{
  return TigLibInt::setStoredPath(newPath);
}

void Repo::setDirs()
{
  PRINT("  setDirs dir=" << dir);
  assert(dir != "");
  tigFile = getPath("spread/channels/tiggit.net/tigdata.json");
  shotDir = getPath("shots_300x260/");
  statsFile = getPath("stats.json");
  newsFile = getPath("news.json");
  spreadDir = getPath("spread/");

  std::string tmpDir = getPath("spread/tmp/");

  PRINT("  spreadDir=" << spreadDir << " tmpDir=" << tmpDir);
  ptr.reset(new _Internal(spreadDir, tmpDir));
}

void Repo::setRepo(const std::string &where)
{
  dir = where;
  if(dir == "") dir = ".";

  setDirs();
}

std::string Repo::findLegacyDir() { return TigLibInt::findLegacyRepo(); }

Spread::SpreadLib &Repo::getSpread() const
{
  assert(ptr);
  return ptr->spread;
}

std::string Repo::defaultPath() { return TigLibInt::getDefaultPath(); }

bool Repo::initRepo(bool forceLock)
{
  assert(ptr);
  assert(dir != "");

  bf::create_directories(dir);

  // Lock the repo before we start writing to it
  if(!ptr->lock.lock(getPath("lock"), forceLock))
    return false;

  // Open config files
  conf.load(getPath("tiglib.conf"));
  news.load(getPath("tiglib_news.conf"));
  rates.load(getPath("tiglib_rates.conf"));

  // Load config options
  lastTime = conf.getInt64("last_time", -1);

  return true;
}

void Repo::convertOldInstallConf() const
{
  // Convert old install list, if present
  std::string instFile = getPath("tiglib_installed.conf");
  std::string oldF = instFile+".old", newF = instFile+".new";
  if(!bf::exists(instFile) && !bf::exists(oldF) && !bf::exists(newF))
    return;

  PRINT("Converting old install list conf");
  PRINT("  reading " << instFile);

  Misc::JConfig inst(instFile);
  std::vector<std::string> names = inst.getNames();
  for(int i=0; i<names.size(); i++)
    {
      const std::string &id = names[i];
      std::string path = inst.get(id);
      PRINT("  FOUND: " << id << " at " << path);
      if(path != "" && id.size() > 11 && id.substr(0,11) == "tiggit.net/")
        try { ptr->spread.setLegacyPack("tiggit.net", id, path); }
        catch(std::exception &e)
          { PRINT("    ERROR: " << e.what()); }
    }
  bf::remove(instFile);
  bf::remove(oldF);
  bf::remove(newF);
  PRINT("  legacy conversion done");
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

std::string Repo::getPath(const std::string &fname) const
{
  return (bf::path(dir) / fname).string();
}

std::string Repo::fetchPath(const std::string &url,
                            const std::string &fname)
{
  std::string outfile = getPath(fname);
  if(!offline)
    Fetch::fetchFile(url, outfile);
  return outfile;
}

struct FetchJob : Job
{
  SpreadLib &spread;
  bool shots;
  std::string spreadRepo, shotsPath, statsFile, newsFile, fetchURL;

  FetchJob(SpreadLib &_spread, bool _shots, const std::string &_spreadRepo,
           const std::string &_shotsPath, const std::string &_statsFile,
           const std::string &_newsFile, const std::string &_fetchURL)
    : spread(_spread), shots(_shots), spreadRepo(_spreadRepo),
      shotsPath(_shotsPath), statsFile(_statsFile), newsFile(_newsFile),
      fetchURL(_fetchURL) {}

  void doJob()
  {
    using namespace std;

    JobInfoPtr client = spread.updateFromURL("tiggit.net", fetchURL);
    if(waitClient(client)) return;

    if(shots)
      {
        std::string dest = (bf::path(shotsPath)/"tiggit.net").string();
        client = spread.installPack("tiggit.net", "shots300x260", dest);
        if(client && waitClient(client)) return;
      }

    /* Fetch the stats file (download counts etc), if the current file
       is older than 24 hours.

       We ignore errors, as stats are just cosmetics and not
       critically necessary.
    */
    try { Fetch::fetchIfOlder(ServerAPI::statsURL(), statsFile, 24*60); }
    catch(...) {}

    // Ditto for the news file
    try { Fetch::fetchFile(ServerAPI::newsURL(), newsFile); }
    catch(...) {}

    setDone();
  }
};

bool Repo::hasNewData() const { return ptr->spread.wasUpdated("tiggit.net"); }

JobInfoPtr Repo::fetchFiles(bool includeShots, bool async)
{
  // If we're in offline mode, skip this entire function
  if(offline) return JobInfoPtr();

  assert(isLocked());

  std::string fetchURL = ServerAPI::spreadURL_SR0();

  // Get the Spread channel URL from the config instead, if it is set.
  fetchURL = conf.get("fetch_url", fetchURL);

  // Create and run the fetch job
  return Thread::run(new FetchJob(ptr->spread, includeShots, spreadDir,
                                  shotDir, statsFile, newsFile, fetchURL),
                     async);
}

std::string Repo::getGameDir(const std::string &idname) const
{
  const PackStatus *is = ptr->spread.getPackStatus("tiggit.net", idname);
  if(is) return is->where;
  return "";
}

uint64_t Repo::getGameSize(const std::string &idname) const
{
  return ptr->spread.getPackInfo("tiggit.net", idname).installSize;
}

std::string Repo::getGameVersion(const std::string &idname) const
{
  return ptr->spread.getPackInfo("tiggit.net", idname).version;
}

void Repo::loadStats()
{
  // Always ignore errors, stats aren't critically important
  try { Stats::fromJson(ptr->data.data, statsFile); }
  catch(...) {}
}

void Repo::doneLoading()
{
  ptr->data.allList.done();
}

void Repo::loadData()
{
  PRINT("loadData()");

  assert(isLocked());

  // TODO: This kills everything. Later we might add the possiblity to
  // selectively add and remove channels.
  ptr->data.clear();
  ptr->data.data.clear();

  ptr->data.data.addChannel("tiggit.net", tigFile);
  convertOldInstallConf();
  loadStats();
  ptr->data.createLiveData(this);
  PRINT("loadData() done");
}

// Get screenshot path for a game.
std::string Repo::getScreenshot(const std::string &idname) const
{
  return (bf::path(shotDir) / idname).string() + ".png";
}

struct InstallJob : Job
{
  std::string sendOnDone;
  JobInfoPtr client;

  void doJob()
  {
    client->wait(info);
    if(checkStatus() || !client->isSuccess())
      {
        setError("Install failed");
        return;
      }

    // Notify the server that the game was downloaded
    Fetch::fetchString(sendOnDone, true);

    setDone();
  }
};

void Repo::getStatusList(StatusList &list) const
{
  PackStatusList orig;
  ptr->spread.getStatusList(orig);
  list.resize(orig.size());
  PackStatusList::const_iterator it = orig.begin();
  for(int i=0; i<list.size(); i++)
    {
      GameStatus &s = list[i];
      const Spread::PackStatus *is = *it++;
      assert(is->info.channel == "tiggit.net");
      s.id = is->info.package;
      s.curVer = is->info.version;
      s.isRemoved = false;
      // Not good to use exceptions for flow control, but this should
      // be a rare occurance. Fix it later.
      try { s.newVer = ptr->spread.getPackInfo(is->info.channel, is->info.package).version; }
      catch(...) { s.isRemoved = true; }
      s.where = is->where;
      s.isUpdated = is->needsUpdate;
    }
}

// Start installing or upgrading a game
JobInfoPtr Repo::startInstall(const std::string &idname, const std::string &urlname,
                              std::string where, bool async)
{
  // Ignore offline mode for game installs, since they are user
  // initiated events.
  where = bf::absolute(where).string();
  JobInfoPtr client = ptr->spread.installPack("tiggit.net", idname, where, NULL, true, true, false); // TODO: Change last to 'true' when we enable user-asks

  if(client)
    {
      InstallJob *job = new InstallJob;
      job->client = client;
      job->sendOnDone = ServerAPI::dlCountURL(urlname);
      Thread::run(job, async);
    }
  return client;
}

/* We had one unfortunate user who managed to set his repo path to his
   main Documents folder. Attempting to move it afterwards (using this
   for cleanup) did not end well; it deleted all his files and he was
   NOT particularly happy.

   We've replaced this with the much safer RepoKillJob below, but keep
   the code around in case you need it for something else.

struct RemoveJob : Job
{
  RemoveJob(const std::string &_what) : what(_what) {}

  std::string what;

  void doJob()
  {
    setBusy("Removing " + what);
    bf::remove_all(what);
    setDone();
  }
};

JobInfoPtr Repo::killPath(const std::string &dir, bool async)
{
  assert(dir != "");
  return Thread::run(new RemoveJob(dir), async);
}
*/

struct RepoKillJob : Job
{
  RepoKillJob(const std::string &_what) : what(_what) {}

  std::string what;

  void doJob()
  {
    setBusy("Removing data from " + what);

    bf::path p = what;

    bf::remove_all(p/"gamedata");
    bf::remove_all(p/"promo");
    bf::remove_all(p/"run");
    bf::remove_all(p/"shots_300x260");
    bf::remove_all(p/"spread");
    bf::remove(p/"news.json");
    bf::remove(p/"stats.json");
    bf::remove(p/"launch.log");
    bf::remove(p/"threads.log");
    bf::remove(p/"tiglib.conf");
    bf::remove(p/"tiglib_news.conf");
    bf::remove(p/"tiglib_rates.conf");
    bf::remove(p/"wxtiggit.conf");
    bf::remove(p/"wxtiggit.conf");
    bf::remove(p/"launch.log.old");
    bf::remove(p/"threads.log.old");
    bf::remove(p/"tiglib.conf.old");
    bf::remove(p/"tiglib_news.conf.old");
    bf::remove(p/"tiglib_rates.conf.old");
    bf::remove(p/"wxtiggit.conf.old");
    bf::remove(p/"wxtiggit.conf.old");

    setDone();
  }
};

JobInfoPtr Repo::killRepo(const std::string &dir, bool async)
{
  assert(dir != "");
  return Thread::run(new RepoKillJob(dir), async);
}

JobInfoPtr Repo::startUninstall(const std::string &idname, bool async)
{
  return ptr->spread.uninstallPack("tiggit.net", idname, getGameDir(idname), async);
}
