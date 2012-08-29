#include "repo.hpp"
#include "fetch.hpp"
#include "server_api.hpp"
#include "repo_locator.hpp"
#include "misc/lockfile.hpp"
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
  dir = where;

  if(dir == "")
    dir = TigLibInt::getStoredPath();

  // Nothing was found. Tell the user.
  if(dir == "")
    return false;

  setDirs();

  // Store path for later
  if(!TigLibInt::setStoredPath(dir))
    return false;

  return true;
}

void Repo::setDirs()
{
  assert(dir != "");
  tigFile = getPath("spread/channels/tiggit.net/tigdata.json");
  shotDir = getPath("shots_300x260/");
  statsFile = getPath("stats.json");
  spreadDir = getPath("spread/");

  ptr.reset(new _Internal(spreadDir, getPath("spread/tmp/")));
}

void Repo::setRepo(const std::string &where)
{
  dir = where;
  if(dir == "") dir = ".";

  setDirs();
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

  // Make sure the repo format is up-to-date
  /*
    TODO: There is no longer any "upgrading" or repositories. This
    will instead be replaced by an import function that moves old data
    into the new repo format.
   */
  //TigLibInt::upgradeRepo(dir);

  // Open config files
  conf.load(getPath("tiglib.conf"));
  inst.load(getPath("tiglib_installed.conf"));
  news.load(getPath("tiglib_news.conf"));
  rates.load(getPath("tiglib_rates.conf"));

  // Load config options
  lastTime = conf.getInt64("last_time", 0xffffffffffff);

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

#include <iostream>

struct FetchJob : Job
{
  SpreadLib &spread;
  bool shots;
  std::string spreadRepo, shotsPath, statsFile;

  FetchJob(SpreadLib &_spread, bool _shots, const std::string &_spreadRepo,
           const std::string &_shotsPath, const std::string &_statsFile)
    : spread(_spread), shots(_shots), spreadRepo(_spreadRepo),
      shotsPath(_shotsPath), statsFile(_statsFile) {}

  void doJob()
  {
    using namespace std;

    JobInfoPtr client = spread.updateFromURL("tiggit.net", ServerAPI::spreadURL_SR0());
    if(waitClient(client)) return;
    if(shots)
      {
        std::string dest = (bf::path(shotsPath)/"tiggit.net").string();
        client = spread.install("tiggit.net", "shots300x260", dest);
        if(waitClient(client)) return;
      }

    /* Fetch the stats file (download counts etc), if the current file
       is older than 24 hours.
    */
    try { Fetch::fetchIfOlder(ServerAPI::statsURL(), statsFile, 24*60); }
    /*
       Ignore errors. Stats are just cosmetics, not critically
       important.
    */
    catch(...) {}

    setDone();
  }
};

JobInfoPtr Repo::fetchFiles(bool includeShots, bool async)
{
  // If we're in offline mode, skip this entire function
  if(offline) return JobInfoPtr();

  assert(isLocked());

  // Create and run the fetch job
  return Thread::run(new FetchJob(ptr->spread, includeShots, spreadDir,
                                  shotDir, statsFile), async);
}

void Repo::loadData()
{
  assert(isLocked());
  ptr->data.data.addChannel("tiggit.net", tigFile);

  // Load stats, but ignore errors
  try { Stats::fromJson(ptr->data.data, statsFile); }
  catch(...) {}

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
  std::string idname;
  Misc::JConfig *inst;

  void doJob()
  {
    if(waitClient(client)) return;

    // Set config status
    inst->setInt(idname, 2);

    // Notify the server that the game was downloaded
    Fetch::fetchString(sendOnDone, true);

    setDone();
  }
};

// Start installing or upgrading a game
JobInfoPtr Repo::startInstall(const std::string &idname, const std::string &urlname,
                              bool async)
{
  // Ignore offline mode for game installs, since they are user-initiated.

  JobInfoPtr client = ptr->spread.install("tiggit.net", idname, getInstDir(idname));
  InstallJob *job = new InstallJob;
  job->client = client;
  job->sendOnDone = ServerAPI::dlCountURL(urlname);
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

// Start uninstalling a game
JobInfoPtr Repo::startUninstall(const std::string &idname, bool async)
{
  // Mark the game as uninstalled immediately
  inst.setInt(idname, 0);

  // Kill the installation directory
  return Thread::run(new RemoveJob(getInstDir(idname)), async);
}
