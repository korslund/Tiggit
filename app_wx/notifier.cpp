#include "notifier.hpp"
#include "gamedata.hpp"
#include "wx/boxes.hpp"
#include <assert.h>

using namespace wxTigApp;
using namespace Spread;

//#define PRINT_DEBUG

#ifdef PRINT_DEBUG
#include <iostream>
#define PRINT(a) std::cout << a << "\n"
#else
#define PRINT(a)
#endif

bool StatusNotifier::hasJobs()
{
  return watchList.size() != 0;
}

void StatusNotifier::watchMe(GameInf *p)
{
  assert(p);

  const std::string &idname = p->info.ent->idname;
  JobInfoPtr info = p->info.getStatus();

  if(!info) return;

  PRINT("Watching " << idname);
  watchList[idname] = info;
  statusChanged();
}

static GameInf* getFromId(const TigLib::Repo &repo, const std::string &idname)
{
  using namespace TigLib;

  const InfoLookup &look = repo.getList();
  InfoLookup::const_iterator it = look.find(idname);
  if(it == look.end()) return NULL;

  return (GameInf*)(it->second->extra);
}

void StatusNotifier::cleanup()
{
  // Disable the loop
  data = NULL;

  // True if there was anything to abort
  bool abort = false;

  // Abort the update job, if any
  if(updateJob && !updateJob->isFinished())
    {
      updateJob->abort();
      abort = true;
    }

  // Abort any other job
  WatchList::iterator it;
  for(it = watchList.begin(); it != watchList.end(); it++)
    {
      JobInfoPtr job = it->second;
      if(job && !job->isFinished())
        {
          job->abort();
          abort = true;
        }
    }

  // Give jobs some time to finish. This is run after the main window
  // closes, so a delay before exiting won't disturb the user.
  if(abort)
    wxSleep(1);
}

void StatusNotifier::reassignJobs()
{
  if(!data) return;

  PRINT("Reassigning " << watchList.size() << " jobs");

  WatchList::iterator it;
  for(it = watchList.begin(); it != watchList.end(); it++)
    {
      GameInf *inf = getFromId(data->repo, it->first);

      // Ignore unknown jobs
      if(!inf) continue;

      JobInfoPtr job = it->second;

      if(job != inf->info.getStatus())
        inf->info.setStatus(job);
    }
}

void StatusNotifier::tick()
{
  // If the data pointer hasn't been set yet, we aren't ready to do
  // anything. So just exit.
  if(!data) return;

  // Check if we're updating the entire dataset first
  if(updateJob)
    if(updateJob->isFinished())
      {
        if(updateJob->isSuccess())
          {
            PRINT("Update job successful");
            data->updateReady();
          }
        else
          {
            /* TODO: If the job failed, but the base Spread update
               succeeded, then the update will not be attempted again
               until the next release. This means we might be sitting on
               a partially updated repo and it won't be fixed.
            */

            PRINT("Update job FAILED: " << updateJob->getMessage());
          }

        updateJob.reset();
      }
    else
      {
        // Update is running
        data->updateRunning(updateJob->getCurrent(), updateJob->getTotal());
      }

  // How much do we need to update
  bool soft = false;
  bool hard = false;

  for(WatchList::iterator it = watchList.begin(); it != watchList.end();)
    {
      // If there are any elements being installed at all, always
      // update the displays.
      soft = true;

      // Increase the iterator here since we might erase it from the
      // list below, invalidating the current position.
      WatchList::iterator itold = it++;

      // Get the GameInf pointer
      GameInf* inf = getFromId(data->repo, itold->first);
      if(!inf) continue;

      // Get the stored job
      JobInfoPtr job = itold->second;

      // Check that we have the right job attached
      assert(job == inf->info.getStatus());

      // If we are no longer working, update main status and remove
      // ourselves from the list
      if(job->isFinished())
        {
          watchList.erase(itold);
          hard = true;

          // Report errors to the user
          if(job->isError())
            Boxes::error(job->getMessage());
        }

      // Update the object status
      inf->updateStatus();
    }

  /* A 'hard' update means totally refresh the 'installed' list, and
     tell all tabs to update game data - screenshot, button
     information etc - in case the currently selected game has changed
     status.
  */
  if(hard)
    statusChanged();

  /* A 'soft' update just refreshes the list views. It's only meant to
     update the percentages when downloading/installing.
   */
  else if(soft)
    data->updateDisplayStatus();
}

void StatusNotifier::statusChanged()
{
  if(data)
    data->installStatusChanged();
}

StatusNotifier wxTigApp::notify;

// This makes sure the notifier runs regularly
struct MyTimer : wxTimer
{
  MyTimer(int ms)
  {
    Start(ms);
  }

  void Notify()
  {
    notify.tick();
  }
};

static MyTimer timer(700);
