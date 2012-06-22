#include "notifier.hpp"
#include "gamedata.hpp"
#include "wx/boxes.hpp"

using namespace wxTigApp;

void StatusNotifier::tick()
{
  if(!data) return;

  std::set<GameInf*>::iterator it, itold;

  // How much do we need to update
  bool soft = false;
  bool hard = false;

  for(it = watchList.begin(); it != watchList.end();)
    {
      // If there are any elements being installed, update the
      // displays.
      soft = true;

      itold = it++;
      GameInf *inf = *itold;

      // If we are no longer working, update main status and remove
      // ourselves from the list
      if(!inf->isWorking())
        {
          watchList.erase(itold);
          hard = true;

          // Report errors to the user
          Jobify::JobInfoPtr info = inf->info.getStatus();
          if(info->isError())
            Boxes::error(info->message);
        }

      // Update the object status
      inf->updateStatus();
    }

  /* A 'hard' update means totally refresh the 'installed' list, and
     tell all tabs to update game data - screenshot, butten
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
