#include "appupdate.hpp"

#include <spread/job/thread.hpp>
#include <spread/spread.hpp>
#include "misc/dirfinder.hpp"
#include "version.hpp"
#include <fstream>
#include <ctime>
#include <boost/filesystem.hpp>
#include <mangle/stream/servers/file_stream.hpp>
#include <mangle/stream/servers/outfile_stream.hpp>
#include "launcher/run.hpp"

using namespace wxTigApp;
using namespace Spread;
namespace bf = boost::filesystem;
namespace MS = Mangle::Stream;

/* Mechanism used to alternate between two program directories. This
   is necessary for auto-updates on Windows because you can't update
   the current app while it's running.
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
  }

  void doSwitch()
  {
    if(cur == '1') cur = '2';
    else cur = '1';
    write();
  }
};

struct Logger
{
  std::ofstream log;

  Logger(const std::string &file)
  {
    if(bf::exists(file))
      {
        std::string old = file + ".old";
        if(bf::exists(old))
          bf::remove(old);
        bf::rename(file, old);
      }
    log.open(file.c_str());
  }

  void operator()(const std::string &msg)
  {
    char buf[100];
    time_t now = std::time(NULL);
    std::strftime(buf, 100, "%Y-%m-%d %H:%M:%S", gmtime(&now));
    log << buf << ":   " << msg << "\n";
    // Flush after each line in case of crashes
    log.flush();
  }
};

// Returns true if the given path is the same as the current exe. Only
// works on Windows, always returns false on other platforms.
static bool isCurrentExe(const std::string &newExe)
{
#ifdef _WIN32
  std::string thisExe = Misc::DirFinder::getExePath();
  if(thisExe == newExePath ||
     (bf::exists(newExePath) && bf::equivalent(thisExe, newExePath)))
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
    Logger log(repo->getPath("update.log"));

    bool &hasNew = *hasNewUpdate;
    std::string &newExe = *newExePath;
    std::string &newVer = *newVersion;

    hasNew = false;
    newExe = "";
    newVer = "";

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
        std::string package = "wxtiggit-win32-1";

        // Install into the non-current directory
        std::string dest = swt.getOther();
        newExe = swt.getOther("tiggit.exe");

        log("Installing " + package + " into " + dest);
        log("  (Current EXE is: " + Misc::DirFinder::getExePath() + ")");

        if(isCurrentExe(newExe))
          {
            setError("Cannot overwrite running executable: " + newExe);
            log("ERROR: " + info->getMessage());
            return;
          }

        client = repo->getSpread().install("tiggit.net", package, dest);
        if(waitClient(client))
          {
            if(info->isAbort()) log("App update aborted");
            else log("App update failed: " + info->getMessage());
            return;
          }
        log("Wrote new EXE: " + newExe);

        /* TODO: Load version file, compare version with this version,
           set hasNew accordingly. Log everything.
         */

        if(hasNew)
          {
            log("NEW version detected. Restart required.");
            swt.doSwitch();
          }
        else log("Existing version detected.");
      }
    else log("No new data available");

    log("Update job finished successfully");
    setDone();
  }
};

Spread::JobInfoPtr AppUpdater::startJob()
{
  if(repo.offline) return JobInfoPtr();

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
  // This is meand to be run at program startup, so we assume no
  // values have been set yet
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
  assert(newExePath != "");
  return launch();
}

bool AppUpdater::launch()
{
  Logger log(repo.getPath("launch.log"));
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

#ifdef _WIN32
  log("Current EXE: " + Misc::DirFinder::getExePath());

  // Are we already running the EXE we are supposed to launch?
  if(isCurrentExe(newExePath))
    {
      log("We are already running the latest executable");
      return false;
    }

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
      log << "ERROR: " << e.what() << endl;
    }
#else
  log("Only implemented on Windows");
#endif
  return false;
}
