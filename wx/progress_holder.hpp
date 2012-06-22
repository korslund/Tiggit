#ifndef __PROGRESS_HOLDER_HPP_
#define __PROGRESS_HOLDER_HPP_

#include "wxcommon.hpp"
#include <wx/progdlg.h>

namespace wxTiggit
{
  class ProgressHolder
  {
    wxApp *app;
    wxProgressDialog *dlg;

  public:

    ProgressHolder(wxApp *_app)
      : app(_app), dlg(NULL)
    {}
    ~ProgressHolder();

    bool update(int value);
    bool pulse();
    void yield();
    bool setMsg(const std::string &str, int value=0);
    bool setMsg(const wxString &str, int value=0);
  };
}
#endif
