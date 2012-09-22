#ifndef __WX_NEWSTAB_HPP_
#define __WX_NEWSTAB_HPP_

#include "tabbase.hpp"
#include "wxgamedata.hpp"

class wxListEvent;

namespace wxTiggit
{
  struct NewsList;

  // The tab responsible for showing news from the website.
  struct NewsTab : TabBase
  {
    NewsList *list;
    wxTextCtrl *textView;

    NewsTab(wxNotebook *parent, wxGameData &data);

    void reloadData();

  private:
    // Internal functions
    int getTitleNumber();
    void gotFocus();

    // Event handling
    void onWebsite(wxCommandEvent &event);
    void onReadAll(wxCommandEvent &event);
    void onListSelect(wxListEvent &event);
    void onListDeselect(wxListEvent &event);
    void onUrlEvent(wxTextUrlEvent &event);
  };
}
#endif
