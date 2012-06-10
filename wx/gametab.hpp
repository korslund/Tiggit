#ifndef __GAMETAB_HPP_
#define __GAMETAB_HPP_

#include "tabbase.hpp"
#include "gamelist.hpp"

namespace wxTiggit
{
  struct ImageViewer;

  struct GameTab : TabBase, wxGameListener, wxScreenshotCallback
  {
    GameTab(wxNotebook *parent, const wxString &name, wxGameList &lst);

  protected:
    GameListView *list;
    wxGameList &lister;

  private:
    int select;

    // Controls
    wxButton *b1, *b2;
    wxTextCtrl *textView;
    ImageViewer *screenshot;
    wxListBox *tags;
    wxChoice *rateBox;
    wxStaticText *rateText;
    wxString rateString[7];

    // wxGameListener functions
    void gameStatusChanged();
    void gameInfoChanged();
    void gameListChanged();
    void gameListReloaded();

    // TabBase functions
    void gotFocus();
    int getTitleNumber();

    // Screenshot callback
    void shotIsReady(const std::string &idname, const wxImage &shot);

    // Event handling functions
    void onSpecialKey(wxCommandEvent &event);
    void onUrlEvent(wxTextUrlEvent &event);
    void onRating(wxCommandEvent &event);
    void onTagSelect(wxCommandEvent &event);
    void onSearch(wxCommandEvent &event);
    void onGamePage(wxCommandEvent &event);
    void onButton(wxCommandEvent &event);
    void onListActivate(wxListEvent &event);
    void onListDeselect(wxListEvent &event);
    void onListSelect(wxListEvent &event);
    void onListRightClick(wxListEvent &event);
    void onContextClick(wxCommandEvent &evt);

    // Internal functions
    void updateSelection();
    void fixButtons();
    void updateGameInfo();
    void doAction1(int index);
    void doAction2(int index);
  };
}
#endif
