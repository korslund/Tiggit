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

// Used to notify the server about broken URLs
struct CallbackURL
{
  void operator()(const Hash &hash, const std::string &url) const
  {
    Fetch::fetchString(ServerAPI::brokenURL(hash.toString(), url), true);
  }
};

struct Repo::_Internal
{
  Misc::LockFile lock;
  TigLib::GameData data;
  SpreadLib spread;
  std::string tmp;

  // Used by fetchFiles()
  bool newData;

  _Internal(const std::string &spreadDir, const std::string &tmpDir)
    : spread(spreadDir, tmpDir), tmp(tmpDir)
  {
    CallbackURL cb;
    spread.setURLCallback(cb);

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
  inst.load(getPath("tiglib_installed.conf"));
  news.load(getPath("tiglib_news.conf"));
  rates.load(getPath("tiglib_rates.conf"));

  // Load config options
  lastTime = conf.getInt64("last_time", -1);

  return true;
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
  bool shots, *newData;
  std::string spreadRepo, shotsPath, statsFile, newsFile;

  FetchJob(SpreadLib &_spread, bool _shots, const std::string &_spreadRepo,
           const std::string &_shotsPath, const std::string &_statsFile,
           const std::string &_newsFile, bool *_newData)
    : spread(_spread), shots(_shots), newData(_newData), spreadRepo(_spreadRepo),
      shotsPath(_shotsPath), statsFile(_statsFile), newsFile(_newsFile) {}

  void doJob()
  {
    using namespace std;

    JobInfoPtr client = spread.updateFromURL("tiggit.net", ServerAPI::spreadURL_SR0());
    if(waitClient(client)) return;

    // Set newData depending on whether data was updated
    *newData = spread.wasUpdated("tiggit.net");

    if(shots)
      {
        std::string dest = (bf::path(shotsPath)/"tiggit.net").string();
        client = spread.install("tiggit.net", "shots300x260", dest);
        if(waitClient(client)) return;
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

bool Repo::hasNewData() const { return ptr->newData; }

JobInfoPtr Repo::fetchFiles(bool includeShots, bool async)
{
  ptr->newData = false;

  // If we're in offline mode, skip this entire function
  if(offline) return JobInfoPtr();

  assert(isLocked());

  // Create and run the fetch job
  return Thread::run(new FetchJob(ptr->spread, includeShots, spreadDir,
                                  shotDir, statsFile, newsFile,
                                  &ptr->newData), async);
}

std::string Repo::getGameDir(const std::string &idname)
{
  if(inst.has(idname))
    {
      std::string val = inst.get(idname);

      // Check for old int values
      std::string newVal = val;
      if(val == "0") newVal = "";
      else if(val == "2") newVal = getDefGameDir(idname);

      // Update the stored value if it was altered
      if(newVal != val)
        inst.set(idname, newVal);

      return newVal;
    }
  return "";
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
  assert(isLocked());

  // TODO: This kills everything. Later we might add the possiblity to
  // selectively add and remove channels.
  ptr->data.clear();
  ptr->data.data.clear();

  ptr->data.data.addChannel("tiggit.net", tigFile);
  loadStats();
  ptr->data.createLiveData(this);
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
  std::string idname, where;
  Misc::JConfig *inst;

  void doJob()
  {
    if(waitClient(client)) return;

    // Set config status
    inst->set(idname, where);

    // Notify the server that the game was downloaded
    Fetch::fetchString(sendOnDone, true);

    setDone();
  }
};

// Start installing or upgrading a game
JobInfoPtr Repo::startInstall(const std::string &idname, const std::string &urlname,
                              std::string where, bool async)
{
  // Ignore offline mode for game installs, since they are user
  // initiated events.
  where = bf::absolute(where).string();
  JobInfoPtr client = ptr->spread.install("tiggit.net", idname, where);
  InstallJob *job = new InstallJob;
  job->client = client;
  job->sendOnDone = ServerAPI::dlCountURL(urlname);
  job->where = where;
  job->idname = idname;
  job->inst = &inst;
  return Thread::run(job,async);
}

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

// Start uninstalling a game
JobInfoPtr Repo::startUninstall(const std::string &idname, bool async)
{
  // Get the game's install dir
  std::string dir = getGameDir(idname);
  if(dir == "") return JobInfoPtr();

  // Mark the game as uninstalled immediately
  inst.set(idname, "");

  // Kill the installation directory
  return killPath(dir, async);
}
