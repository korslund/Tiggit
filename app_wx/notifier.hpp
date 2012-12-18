#ifndef __WXAPP_NOTIFIER_HPP_
#define __WXAPP_NOTIFIER_HPP_

#include <map>
#include <spread/job/jobinfo.hpp>

/* This is a pretty simple and unelegant notification distributor. We
   can refine it later.
 */

namespace wxTigApp
{
  struct GameData;
  struct GameInf;

  struct StatusNotifier
  {
    GameData *data;

    typedef std::map<std::string, Spread::JobInfoPtr> WatchList;
    WatchList watchList;

    // JobInfo used for the background update thread. When this job
    // finishes, we notify the main loader system.
    Spread::JobInfoPtr updateJob;

    StatusNotifier() : data(0) {}

    // Invoked at program exit
    void cleanup();

    // Invoked regularly to inspect the watchList
    void tick();

    /* Reassign existing jobs to the corresponding new LiveInfo /
       GameInf structures.

       This is useful after a data reload, when the entire tree of
       data structures has been replaced in memory, but there might
       still be lingering install jobs in progress. Running this as
       soon as the GameInf structures have been set up will assign
       those existing jobs (previously added here through watchMe())
       correctly to the new structures.
    */
    void reassignJobs();

    // Returns true if there are any currently running install jobs
    bool hasJobs();

    // Add an item to the watch list
    void watchMe(GameInf *p);

    // Notify the main data object that an item has changed status
    void statusChanged();
  };

  extern StatusNotifier notify;
}

#endif
