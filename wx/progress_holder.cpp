#include "progress_holder.hpp"

using namespace wxTiggit;

ProgressHolder::~ProgressHolder()
{
  // Shut down the window
  if(dlg)
    {
      dlg->Update(100);
      dlg->Destroy();
      ::wxSafeYield(dlg);
    }
}

bool ProgressHolder::setMsg(const std::string &str, int value)
{
  return setMsg(strToWx(str), value);
}

bool ProgressHolder::update(int value)
{
  assert(dlg);
  return dlg->Update(value);
}

bool ProgressHolder::pulse()
{
  assert(dlg);
  return dlg->Pulse();
}

void ProgressHolder::yield() { ::wxSafeYield(dlg); }

bool ProgressHolder::setMsg(const wxString &str, int value)
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
  ::wxSafeYield(dlg);
  return res;
}
