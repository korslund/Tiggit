#ifndef __WXAPP_NOTIFIER_HPP_
#define __WXAPP_NOTIFIER_HPP_

#include <set>
#include <spread/job/jobinfo.hpp>

/* This is a pretty simple and unelegant notification distributor. We
   can refine it later.
 */

namespace wxTigApp
{
  struct GameInf;
  struct GameData;

  struct StatusNotifier
  {
    GameData *data;
    std::set<GameInf*> watchList;

    // JobInfo used for the background update thread. When this job
    // finishes, we have to notify the main loader system.
    Spread::JobInfoPtr updateJob;

    StatusNotifier() : data(0) {}

    // Invoked regularly to inspect the watchList
    void tick();

    // Add an item to the watch list
    void watchMe(GameInf *p)
    {
      watchList.insert(p);
      statusChanged();
    }

    // Notify the main data object that an item has changed status
    void statusChanged();
  };

  extern StatusNotifier notify;
}

#endif
