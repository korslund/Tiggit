#ifndef __WXAPP_JOBPROG_HPP_
#define __WXAPP_JOBPROG_HPP_

#include "wx/progress_holder.hpp"
#include "job/jobinfo.hpp"

namespace wxTigApp
{
  struct JobProgress : wxTiggit::ProgressHolder
  {
    Jobify::JobInfoPtr info;

    JobProgress(wxApp *app, Jobify::JobInfoPtr _info)
      : wxTiggit::ProgressHolder(app), info(_info) {}

    // Returns true on success, false on failure or abort.
    bool start(const std::string &msg);
  };
}
#endif
