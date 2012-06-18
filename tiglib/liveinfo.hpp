#ifndef __TIGLIB_LIVEINFO_HPP_
#define __TIGLIB_LIVEINFO_HPP_

#include "gameinfo/tigentry.hpp"
#include "job/job.hpp"

namespace TigLib
{
  /* Callback functor called when a screenshot download is ready. It
     may either be invoked immediately from requestShot(), or from a
     worker thread. For asynchronous requests, make sure the reciever
     is thread safe!
   */
  struct ShotIsReady
  {
    virtual void shotIsReady(const std::string &idname,
                             const std::string &file) = 0;
  };

  /* This represents the 'live' counterpart to the game information in
     gameinfo/.

     Unlike TigData::TigEntry, which contains static, pre-loaded data,
     this structure is used to host dynamically updated information
     such as the local user's install status and rating information,
     and functionality such as screenshot fetching.
   */
  class Repo;
  struct LiveInfo
  {
    LiveInfo(const TigData::TigEntry *e,
             Repo *_repo)
      : ent(e), extra(NULL), repo(_repo) {}

    // Link to static game info
    const TigData::TigEntry *ent;

    // Extra data, used for custom application structs
    void *extra;

    /* Request a screenshot. Will invoke the given callback when the
       screenshot is ready.

       If async==true, a download will start in the background if a
       cached file is not available. A pointer to the attached JobInfo
       is returned. If async==false, the function will not return
       until the screenshot is available.

       In any case, you may use JobInfo::isBusy, ::isSuccess(),
       ::isError etc on the result to inquire about status.

       Calling the function multiple times may or may not invoke the
       callback multiple times. The behavior depends on thread status
       and is thus undefined.
    */
    Jobify::JobInfoPtr requestShot(ShotIsReady*, bool async = true);

  private:
    Jobify::JobInfoPtr screenJob;
    Repo *repo;
  };
}

#endif
