#include "appupdate.hpp"

#include <spread/job/thread.hpp>
#include <spread/spread.hpp>
#include <spread/misc/readjson.hpp>
#include <spread/tasks/download.hpp>
#include "misc/dirfinder.hpp"
#include "misc/fetch.hpp"
#include "version.hpp"
#include <fstream>
#include <ctime>
#include <boost/filesystem.hpp>
#include <mangle/stream/servers/file_stream.hpp>
#include <mangle/stream/servers/outfile_stream.hpp>
#include "launcher/run.hpp"
#include <spread/misc/logger.hpp>

//#define PRINT_DEBUG

#ifdef PRINT_DEBUG
#include <iostream>
#define PRINT(a) std::cout << a << "\n"
#else
#define PRINT(a)
#endif

using namespace wxTigApp;
using namespace Spread;
namespace bf = boost::filesystem;
namespace MS = Mangle::Stream;

/* Mechanism used to alternate between two program directories. This
   is necessary for auto-updates on Windows because you can't update
   the current exe while it is running.
 */
struct Switch
{
  bf::path baseDir;
  char cur;

  std::string curFile, dir1, dir2;

  Switch(const std::string &base)
    : baseDir(base)
  {
    curFile = (baseDir / "current").string();
    dir1 =  (baseDir / "1").string();
    dir2 =  (baseDir / "2").string();

    bf::create_directories(dir1);
    bf::create_directories(dir2);

    if(bf::exists(curFile))
      {
        MS::FileStream::Read(curFile, &cur, 1);
        if(cur != '1' && cur != '2')
          cur = '2';
      }
    else cur = '2';

    PRINT("Loaded switch " << base << ", cur=" << cur);
  }

  ~Switch() { if(!bf::exists(curFile)) write(); }

  std::string getCurrent() { return cur == '1' ? dir1 : dir2; }
  std::string getOther() { return cur == '1' ? dir2 : dir1; }

  std::string getCurrent(const std::string &file)
  { return (bf::path(getCurrent())/file).string(); }

  std::string getOther(const std::string &file)
  { return (bf::path(getOther())/file).string(); }

  void write()
  {
    assert(cur == '1' || cur == '2');
    MS::OutFileStream::Write(curFile, &cur, 1);
    PRINT("Wrote switch value " << cur);
  }

  void doSwitch()
  {
    PRINT("doSwitch()");
    if(cur == '1') cur = '2';
    else cur = '1';
    write();
  }
};

// Returns true if the given path is the same as the current exe. Only
// works on Windows, always returns false on other platforms.
static bool isCurrentExe(const std::string &newExe)
{
#ifdef _WIN32
  std::string thisExe = Misc::DirFinder::getExePath();
  if(thisExe == newExe ||
     (bf::exists(newExe) && bf::equivalent(thisExe, newExe)))
    return true;
#endif
  return false;
}

struct UpdateJob : Job
{
  TigLib::Repo *repo;
  bool *hasNewUpdate;
  std::string *newExePath, *newVersion;

  void doJob()
  {
    Misc::Logger log(repo->getPath("update.log"));
#ifdef PRINT_DEBUG
    log.print = true;
#endif

    bool &hasNew = *hasNewUpdate;
    std::string &newExe = *newExePath;
    std::string &newVer = *newVersion;

    hasNew = false;
    newExe = "";
    newVer = "";

    /* Do a cache cleanup run - this removes dead entries from our
       cache, potentially speeding up load time.
     */
    log("Doing routine cache cleanup");
    repo->getSpread().verifyCache();

    // Do the main data update first
    log("Starting update of repository " + repo->getPath(""));
    JobInfoPtr client = repo->fetchFiles();
    if(waitClient(client))
      {
        if(info->isAbort()) log("TigLib update aborted");
        else log("TigLib update failed: " + info->getMessage());
        return;
      }

    /* If there was no new update of the dataset, don't bother
       checking for a new app. Any new app release must necessarily be
       packaged as part of a data update.
    */
    if(repo->hasNewData())
      {
        log("New data was available");

        Switch swt(repo->getPath("run"));
        std::string package = "wxtiggit1-win32";

        // Install into the non-current directory
        std::string dest = swt.getOther();
        newExe = swt.getOther("tiggit.exe");

        log("New EXE: " + newExe);
        log("Current EXE: " + Misc::DirFinder::getExePath());

        if(isCurrentExe(newExe))
          {
            setError("Cannot overwrite running executable: " + newExe);
            log("ERROR: " + info->getMessage());
            return;
          }

        log("Current version: " + std::string(TIGGIT_VERSION));
        newVer = repo->getSpread().getPackInfo("tiggit.net", package).version;
        log("New version: " + newVer);

        // Figure out if we've got a new version
        hasNew = (newVer != "" && newVer != TIGGIT_VERSION);

        if(hasNew)
          {
            log("NEW version detected!");
            log("Installing tiggit.net/" + package + " into " + dest);
            client = repo->getSpread().installPack("tiggit.net", package, dest);
            if(client && waitClient(client))
              {
                if(info->isAbort()) log("Install aborted");
                else log("Install failed: " + info->getMessage());
                return;
              }

            log("Install complete. Switching to new directory.");
            swt.doSwitch();
          }
        else log("Existing version detected.");
      }
    else log("No new data available");

    try
      {
        std::string promoFile = repo->getPath("promo/index.json");
        Fetch::fetchIfOlder("http://tiggit.net/client/promo/index.json", promoFile, 120);

        Json::Value val = ReadJson::readJson(promoFile);

        std::vector<std::string> names = val.getMemberNames();
        for(int i=0; i<names.size(); i++)
          {
            const std::string &code = names[i];
            std::string imgFile = repo->getPath("promo/" + code + ".png");

            if(code != "" && !bf::exists(imgFile))
              {
                std::string imgUrl = val[code]["img_url"].asString();
                if(imgUrl != "")
                  {
                    DownloadTask dl(imgUrl, imgFile);
                    if(runClient(dl, false, false)) return;
                  }
              }
          }
      }
    catch(...) {}

    log("Update job finished successfully");
    setDone();
  }
};

void AppUpdater::cacheLocalExeDir()
{
#ifdef _WIN32
  SpreadLib &spread = repo.getSpread();

  // Get the directory containing the current running exe
  bf::path exePath = Misc::DirFinder::getExePath();
  exePath = exePath.parent_path();

  // Traverse it and cache all the files in it
  bf::directory_iterator iter(exePath), end;
  for(; iter != end; ++iter)
    {
      bf::path file = iter->path();

      // Ignore errors, this is just an optimization anyway.
      try { spread.cacheFile(file.string()); }
      catch(...) {}
    }
#endif
}

Spread::JobInfoPtr AppUpdater::startJob()
{
  if(repo.offline) return JobInfoPtr();

  PRINT("AppUpdater::startJob()");

  assert(!current || current->isFinished());

  UpdateJob *job = new UpdateJob;
  job->repo = &repo;
  job->hasNewUpdate = &hasNewUpdate;
  job->newExePath = &newExePath;
  job->newVersion = &newVersion;

  current = Thread::run(job);
  return current;
}

bool AppUpdater::launchCorrectExe()
{
  // This is ment to be run at program startup, so we assume no values
  // have been set yet
  assert(!hasNewUpdate);
  assert(!current);
  assert(newExePath == "");

  Switch swt(repo.getPath("run"));
  newExePath = swt.getCurrent("tiggit.exe");

  return launch();
}

bool AppUpdater::launchNew()
{
  assert(current && current->isSuccess());
  assert(hasNewUpdate);
  return launch();
}

bool AppUpdater::launch()
{
  assert(newExePath != "");

  Misc::Logger log(repo.getPath("launch.log"));
  log("launch() newExePath=" + newExePath);

  if(!bf::exists(newExePath))
    {
      /* This isn't really a critical error, it will happen on new
         installs before we have had a chance to launch the first
         update run.
       */
      log("ERROR: executable does not exist");
      return false;
    }

  log("Current EXE: " + Misc::DirFinder::getExePath());

  // Are we already running the EXE we are supposed to launch?
  if(isCurrentExe(newExePath))
    {
      log("We are already running the latest executable");
      return false;
    }

  // Is there an override file?
  {
    std::string override =
      (bf::path(Misc::DirFinder::getExePath()).parent_path() / "override").string();
    log("Checking for override file: " + override);
    if(bf::exists(override))
      {
        log("Override found, aborting.");
        return false;
      }
  }

#ifdef _WIN32
  // Everything seems to be in order. Run the exe.
  try
    {
      std::string exeDir = bf::path(newExePath).parent_path().string();
      log("Running " + newExePath + ", working directory " + exeDir);
      Launcher::run(newExePath, exeDir);
      return true;
    }
  catch(std::exception &e)
    {
      log("ERROR: " + std::string(e.what()));
    }
#else
  log("Launching only implemented for Windows");
#endif
  return false;
}
