#ifndef __WX_FRAME_HPP_
#define __WX_FRAME_HPP_

#include "wxcommon.hpp"
#include "tabbase.hpp"
#include "wxgamedata.hpp"

namespace wxTiggit
{
  struct NewsTab;

  class TigFrame : public wxFrame, public wxAppListener
  {
    wxNotebook *book;
    TabBase *newGamesTab, *freewareTab, *demosTab, *installedTab;
    NewsTab *newsTab;
    wxGameData &data;
    wxSizer *mainSizer, *noticeSizer;

    wxStaticText *noticeText;
    wxButton *noticeButton;
    int noticeID;

  public:
    TigFrame(const wxString& title, const std::string &ver,
             wxGameData &data);

  private:
    void updateTabNames();
    void focusTab();

    // From wxAppListener
    void refreshNews();
    void displayNotification(const std::string &message, const std::string &button,
                             int id);

    // Event handling
    void onNoticeButton(wxCommandEvent &event);
    void onExit(wxCommandEvent &event) { Close(); }
    void onSpecialKey(wxCommandEvent &event);
    void onOption(wxCommandEvent &event);
    void onDataMenu(wxCommandEvent &event);
  };
}
#endif
