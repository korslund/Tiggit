#ifndef __WX_GAMELISTVIEW_HPP_
#define __WX_GAMELISTVIEW_HPP_

#include "listbase.hpp"
#include <vector>

#include "wxgamedata.hpp"

namespace wxTiggit
{
  struct ColumnHandler
  {
    virtual wxString getText(const wxGameInfo &e) = 0;
    virtual bool doSort(wxGameList &list) = 0;

    void sort(wxGameList &list);
  };

  class GameListView : public ListBase, public wxGameListener
  {
    wxListItemAttr orange, gray, bold;
    wxGameList &lister;
    std::vector<ColumnHandler*> colHands;

  public:
    // Special display modifiers
    bool markNew, markInstalled;

    GameListView(wxWindow *parent, int id, wxGameList &lst);

    void addColumn(const std::string &name, int width, ColumnHandler *ch);

    // wxGameListener functions
    void gameInfoChanged();
    void gameSelectionChanged();
    void gameListChanged();
    void gameStatusChanged();

  private:
    void onHeaderClick(wxListEvent& event);
    void updateSize();

    // Inherited functions used for fetching the list data
    wxListItemAttr *OnGetItemAttr(long item) const;
    wxString OnGetItemText(long item, long column) const;
  };
}
#endif
