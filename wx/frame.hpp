#ifndef __WX_FRAME_HPP_
#define __WX_FRAME_HPP_

#include "wxcommon.hpp"
#include "tabbase.hpp"

namespace wxTiggit
{
  class TigFrame : public wxFrame
  {
    wxNotebook *book;
    TabBase *allTab;

  public:
    TigFrame(const wxString& title, const std::string &ver);

  private:
    void updateTabNames();
    void onExit(wxCommandEvent &event) { Close(); }
    void focusTab();
  };
}
#endif
