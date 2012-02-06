#ifndef __PROGRESS_HOLDER_HPP_
#define __PROGRESS_HOLDER_HPP_

#include <wx/wx.h>
#include <wx/progdlg.h>

class ProgressHolder
{
  wxApp *app;
  wxProgressDialog *dlg;

public:

  ProgressHolder(wxApp *_app)
    : app(_app), dlg(NULL)
  {}

  ~ProgressHolder()
  {
    // Shut down the window
    if(dlg)
      {
        dlg->Update(100);
        dlg->Destroy();
        app->Yield();
      }
  }

  bool setMsg(const std::string &str, int value=0)
  { return setMsg(wxString(str.c_str(), wxConvUTF8), value); }

  bool update(int value)
  {
    assert(dlg);
    return dlg->Update(value);
  }

  bool pulse()
  {
    assert(dlg);
    return dlg->Pulse();
  }

  void yield() { app->Yield(); }

  bool setMsg(const wxString &str, int value=0)
  {
    if(!dlg)
      {
        // Crank up the ol' progress bar. Give the text some space to grow.
        dlg = new wxProgressDialog(wxT("Information"), wxT("Tiggit is checking for updates, please wait...        \n\n "),
                                   100, NULL, wxPD_APP_MODAL|wxPD_CAN_ABORT|wxPD_AUTO_HIDE);
        dlg->Show(true);
      }

    dlg->Show(true);
    bool res = dlg->Update(value, str);
    app->Yield();
    return res;
  }
};

#endif
