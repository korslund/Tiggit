#ifndef __WX_FRAME_HPP_
#define __WX_FRAME_HPP_

#include "wxcommon.hpp"
#include "tabbase.hpp"
#include "wxgamedata.hpp"

namespace wxTiggit
{
  class TigFrame : public wxFrame
  {
    wxNotebook *book;
    TabBase *newGamesTab, *freewareTab, *demosTab, *installedTab, *newsTab;
    wxGameData &data;

  public:
    TigFrame(const wxString& title, const std::string &ver,
             wxGameData &data);

  private:
    void updateTabNames();
    void focusTab();

    // Event handling
    void onExit(wxCommandEvent &event) { Close(); }
    void onSpecialKey(wxCommandEvent &event);
    void onOption(wxCommandEvent &event);
    void onDataMenu(wxCommandEvent &event);
  };
}
#endif
