#include "liveinfo.hpp"
#include "repo.hpp"
#include "server_api.hpp"
#include "launcher/run.hpp"
#include <boost/filesystem.hpp>
#include <assert.h>

namespace bs = boost::filesystem;
using namespace TigLib;

LiveInfo::LiveInfo(const TigData::TigEntry *e, Repo *_repo)
  : ent(e), extra(NULL), repo(_repo), myRate(-2)
{
  // Mark newly added games as 'new'.
  sNew = ent->addTime > repo->getLastTime();
}

bool LiveInfo::isInstalled() const
{
  // We use JobInfo status to signal an installed game, even if it was
  // not installed in this session.
  return installJob && installJob->isSuccess();
}

bool LiveInfo::isUninstalled() const
{
  /* Three cases count as 'uninstalled':
     - the JobInfo struct has not been set up yet (!installJob)
     - it has been set up, but has not been attached to any job, or
       reset() was called (!isCreated())
     - a previous install attempt failed or was aborted
       (isNonSuccess())
   */
  return !installJob || !installJob->isCreated() || installJob->isNonSuccess();
}

bool LiveInfo::isWorking() const
{
  return installJob && installJob->isCreated() && !installJob->isFinished();
}

void LiveInfo::setupInfo()
{
  if(!installJob)
    installJob.reset(new Spread::JobInfo);
}

void LiveInfo::markAsInstalled()
{
  assert(isUninstalled());
  setupInfo();
  installJob->setDone();
}

std::string LiveInfo::getInstallDir() const
{
  assert(isInstalled());
  return repo->getInstDir(ent->idname);
}

int LiveInfo::getMyRating()
{
  if(myRate == -2)
    myRate = repo->getRating(ent->idname);

  return myRate;
}

void LiveInfo::setMyRating(int i)
{
  assert(i >= 0 && i <= 5);

  getMyRating();
  if(myRate != -1)
    return;

  myRate = i;
  repo->setRating(ent->idname, ent->urlname, myRate);
}

Spread::JobInfoPtr LiveInfo::install(bool async)
{
  if(isUninstalled())
    // Ask the repository to start the install process
    installJob = repo->startInstall(ent->idname, ent->urlname, async);

  return installJob;
}

/* TODO: This is an ad-hoc solution. We will improve it later, and
   allow upgrading to be counted as a separate mode (ie. uninstalled,
   installing, installed, upgrading).

   Right now this is meant to be used modally, meaning that no other
   LiveInfo functions should be called while the update is in
   progress.
 */
Spread::JobInfoPtr LiveInfo::update(bool async)
{
  return repo->startInstall(ent->idname, ent->urlname, async);
}

Spread::JobInfoPtr LiveInfo::uninstall(bool async)
{
  if(!isInstalled())
    {
      if(isWorking())
        abort();

      installJob->reset();
      return Spread::JobInfoPtr();
    }
  installJob->reset();

  // Tell the repository to uninstall this game
  return repo->startUninstall(ent->idname, async);
}

void LiveInfo::launch() const
{
  assert(isInstalled());

  // Figure out what to run
  bs::path exe = getInstallDir();
  exe /= ent->launch;

  // Use executable location as working directory
  bs::path work = exe.parent_path();

  Launcher::run(exe.string(), work.string());
}

std::string LiveInfo::getScreenshot() const
{
  const std::string &shot = repo->getScreenshot(ent->idname);
  if(bs::exists(shot))
    return shot;
  return "";
}
