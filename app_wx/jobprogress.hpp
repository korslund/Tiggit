#ifndef __WXAPP_JOBPROG_HPP_
#define __WXAPP_JOBPROG_HPP_

#include "../wx/progress_holder.hpp"
#include <spread/job/jobinfo.hpp>

namespace wxTigApp
{
  struct JobProgress : wxTiggit::ProgressHolder
  {
    Spread::JobInfoPtr info;

    JobProgress(Spread::JobInfoPtr _info)
      : info(_info) {}

    // Returns true on success, false on failure or abort.
    bool start(const std::string &msg);
  };
}
#endif
