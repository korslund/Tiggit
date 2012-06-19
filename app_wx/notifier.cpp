#include "notifier.hpp"
#include "gamedata.hpp"

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
        }

      // Update the object status
      inf->updateStatus();
    }

  if(hard)
    statusChanged();
  else if(soft)
    data->updateDisplayStatus();
}

void StatusNotifier::statusChanged()
{
  if(data)
    data->installStatusChanged();
}

StatusNotifier wxTigApp::notify;
